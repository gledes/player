//
// Created by jin on 2018/9/16.
//

#include <cstring>
#include <pthread.h>


#include "FFmpeg.h"
#include "macro.h"

void* task_prepare(void *args) {

    FFmpeg *ffmpeg = static_cast<FFmpeg *>(args);
    ffmpeg->_prepare();
    return 0;
}

void* task_play(void *args) {

    FFmpeg *fFmpeg = static_cast<FFmpeg *>(args);
    fFmpeg->_start();
    return 0;
}

FFmpeg::FFmpeg(JavaCallHelper *callHelper, const char *dataSource) {

    this->callHelper = callHelper;
    this->dataSource = new char[strlen(dataSource) + 1];
    strcpy(this->dataSource, dataSource);

}

FFmpeg::~FFmpeg() {
    delete dataSource;
    dataSource = NULL;
    DELETE(callHelper);

}

void FFmpeg::prepare() {

    //创建线程负责解码流程
    pthread_create(&pid, NULL, task_prepare, this);

}

void FFmpeg::_prepare() {

    //初始化网络，让ffmpeg能够使用网络
    avformat_network_init();

    int ret = avformat_open_input(&formatContext, dataSource, NULL, NULL);
    if (ret) {
        LOGE("打开媒体失败：%s", av_err2str(ret));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_OPEN_URL);
        return ;
    }
    //查找媒体中的音视频流
    ret = avformat_find_stream_info(formatContext, NULL);
    if (ret < 0) {
        LOGE("查找流失败:%s", av_err2str(ret));
        callHelper->onError(THREAD_CHILD, FFMPEG_CAN_NOT_FIND_STREAMS);
        return ;
    }

    for (int i = 0; i < formatContext->nb_streams; ++i) {
        //可能代表一个视频，可能代表一个音频
        AVStream *stream = formatContext->streams[i];
        //包含了解码这段流的各种参数信息
        AVCodecParameters *codecpar = stream->codecpar;

        //视频和音频都需要干的一些事情（获取解码器）
        //1、通过当前流使用的编码方式 查找解码器
        AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
        if (codec == NULL) {
            LOGE("查找解码器失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_FIND_DECODER_FAIL);
            return ;
        }
        //2、获取解码器上下文  内存没释放
        AVCodecContext *context = avcodec_alloc_context3(codec);
        if (context == NULL) {
            LOGE("创建解码上下文失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_ALLOC_CODEC_CONTEXT_FAIL);
            return ;
        }

        //3、设置上下文的一些参数
        ret = avcodec_parameters_to_context(context, codecpar);
        if (ret < 0) {
            LOGE("设置解码上下文参数失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_CODEC_CONTEXT_PARAMETERS_FAIL);
            return ;
        }

        //4、打开解码器
        ret = avcodec_open2(context, codec, NULL);
        if (ret) {
            LOGE("打开解码器失败:%s", av_err2str(ret));
            callHelper->onError(THREAD_CHILD, FFMPEG_OPEN_DECODER_FAIL);
            return ;
        }

        //时间单位
        AVRational time_base = stream->time_base;
        if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            //音频
            //内存没释放
            audioChannel = new AudioChannel(i, context, time_base);

        } else if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {

            //帧率，单位时间内需要显示多少个图像
            AVRational frame_rate = stream->avg_frame_rate;
            int fps = av_q2d(frame_rate);
            //视频
            //内存没释放
            videoChannel = new VideoChannel(i, context, time_base, fps);
            videoChannel->setRenderFrameCallback(callback);

        }

    }

    //没有音视频
    if (!audioChannel && !videoChannel) {
        LOGE("没有音视频");
        callHelper->onError(THREAD_CHILD, FFMPEG_NOMEDIA);
        return ;
    }

    //准备好了，可以随时通知java播放
    callHelper->onPrepare(THREAD_CHILD);

}

void FFmpeg::start() {

    isPlaying = true;
    if (videoChannel) {
        videoChannel->setAudioChannel(audioChannel);
        videoChannel->play();
    }
    if (audioChannel) {
        audioChannel->play();
    }

    pthread_create(&pid_play, NULL, task_play, this);

}

void FFmpeg::_start() {

    int ret;
    while (isPlaying) {
        //申请内存，内存没有释放
        AVPacket *packet = av_packet_alloc();
        ret = av_read_frame(formatContext, packet);
        if (ret == 0) {
            //读取成功
            //stream_index 这一个流的序号
            if(audioChannel && audioChannel->id == packet->stream_index) {
                audioChannel->packets.push(packet);

            } else if (videoChannel && videoChannel->id == packet->stream_index){
                videoChannel->packets.push(packet);
//                LOGE("播放读视频包成功");
            }



        } else if(ret == AVERROR_EOF) {
            //读取完成，可能还没播放完成

        } else {
            //读取失败

        }


    }

}

void FFmpeg::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback;
}
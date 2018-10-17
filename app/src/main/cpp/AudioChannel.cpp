//
// Created by jin on 2018/9/16.
//


#include "AudioChannel.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "macro.h"



void* audio_decode(void* args) {

    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->decode();
    return 0;
}

void* audio_play(void* args) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(args);
    audioChannel->_play();
    return 0;
}


void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context) {
    AudioChannel *audioChannel = static_cast<AudioChannel *>(context);
    //获取pcm数据 多少个字节data
    int dataSize = audioChannel->getPcm();
    if (dataSize > 0) {
        //接收16位数据
        (*bq)->Enqueue(bq, audioChannel->data, dataSize);
    }

}

AudioChannel::AudioChannel(int id, AVCodecContext *avCodecContext, AVRational time_base) : BaseChannel(id, avCodecContext, time_base){
    //44100个16位 44100*2
    //双声道 44100*2*2
    data = static_cast<uint8_t *>(malloc(44100 * 2 * 2));
    memset(data, 0, 44100 * 2 * 2);

}

AudioChannel::~AudioChannel() {

    if (data) {
        free(data);
        data = NULL;
    }

}


//返回获取pcm的数据大小
    int AudioChannel::getPcm() {
    int data_size = 0;
    AVFrame *frame;
    int ret = frames.pop(frame);

    if (!isPlaying) {
        if (ret) {
            releaseAvFrame(frame);
        }
        return data_size;
    }

    //48000HZ 8位 ===》 44100 16位
    //重采样
    //将nb_samples个数据由sample_rate采样率转成44100后返回多少个数据
    int64_t delays = swr_get_delay(swrContext, frame->sample_rate);
    int64_t max_sample = av_rescale_rnd(delays + frame->nb_samples, 44100, frame->sample_rate, AV_ROUND_UP);

    //返回每一个声道的输出数据
    int sample = swr_convert(swrContext, &data, max_sample , const_cast<const uint8_t **>(frame->data), frame->nb_samples);
    //获取sample个 2字节(16位) * 2 声道
//    LOGE("delays:%ld", delays);
//    LOGE("nb_samples:%d, sample:%d", frame->nb_samples, sample);
    data_size = sample * 2 * 2;
//    LOGE("data_size:%d", data_size);
    //获取frame的一个相对播放时间, 获得相对这段播放的秒数
    clock = frame->pts * av_q2d(time_base);
    return data_size;
}


void AudioChannel::play() {

    packets.setWork(true);
    frames.setWork(true);
    isPlaying = true;
    swrContext = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, 44100,
    avCodecContext->channel_layout, avCodecContext->sample_fmt,
            avCodecContext->sample_rate, 0, 0);
    swr_init(swrContext);
    //解码线程
    pthread_create(&pid_audio_decode, NULL, audio_decode, this);

    //播放线程
    pthread_create(&pid_audio_play, NULL, audio_play, this);



}

void AudioChannel::decode() {

    AVPacket *packet = NULL;
    while (isPlaying) {
        int ret = packets.pop(packet);
        if (!isPlaying) {
            break;
        }
        if(!ret) {
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(packet);
        if (ret != 0){
            break;
        }
        //内存没释放
        AVFrame *frame = av_frame_alloc();
        ret = avcodec_receive_frame(avCodecContext, frame);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret != 0) {
            break;
        }
//        LOGE("解码成功");
        frames.push(frame);


    }

    releaseAvPacket(packet);
}

void AudioChannel::_play() {
    SLresult result;

    // 创建引擎 SLObjectItf engineObject
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 初始化引擎
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 获取引擎接口SLEngineItf engineInterface
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE,&engineInterface);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    /**
     * 2、设置混音器
     */
    // 创建混音器SLObjectItf outputMixObject
    result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, 0, 0);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }
    // 初始化混音器outputMixObject
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return;
    }

    /**
     * 3、创建播放器
     */
    /**
     * 配置输入声音信息
     */
    //创建buffer缓冲类型的队列 2个队列
    SLDataLocator_AndroidSimpleBufferQueue android_queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    //pcm数据格式
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
                            SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};

    //数据源 将上述配置信息放到这个数据源中
    SLDataSource slDataSource = {&android_queue, &pcm};

    //设置混音器
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&outputMix, NULL};

    //需要的接口
    const SLInterfaceID ids[1] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    //创建播放器
    (*engineInterface)->CreateAudioPlayer(engineInterface, &bqPlayerObject, &slDataSource, &audioSnk, 1, ids, req);

    //初始化播放器
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    //    得到接口后调用  获取Player接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerInterface);


    /**
     * 4、设置播放回调
     */
    //获取播放器队列接口
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bqPlayerBufferQueueInterface);
    //设置回调
    (*bqPlayerBufferQueueInterface)->RegisterCallback(bqPlayerBufferQueueInterface, bqPlayerCallback, this);

    /**
     * 5、设置播放状态
     */
    // 设置播放状态
    (*bqPlayerInterface)->SetPlayState(bqPlayerInterface, SL_PLAYSTATE_PLAYING);

    /**
     * 6、启动回调函数
     */
    bqPlayerCallback(bqPlayerBufferQueueInterface, this);


}


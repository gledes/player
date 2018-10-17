//
// Created by jin on 2018/9/16.
//

#include "VideoChannel.h"
#include "macro.h"

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
}


void *decode_task(void *args) {
    VideoChannel *videoChannel = static_cast<VideoChannel *>(args);
    videoChannel->decode();
    return 0;
}

void *render_task(void *args) {
    VideoChannel *videoChannel = (VideoChannel *) args;
    videoChannel->render();
    return 0;
}

VideoChannel::VideoChannel(int id, AVCodecContext *avCodecContext, AVRational time_base, int fps)
        : BaseChannel(id, avCodecContext, time_base), fps(fps) {

}

VideoChannel::~VideoChannel() {

}

void VideoChannel::setAudioChannel(AudioChannel *audioChannel) {
    this->audioChannel = audioChannel;
}

void VideoChannel::play() {
    isPlaying = true;
    packets.setWork(true);
    frames.setWork(true);
    //1、解码
    pthread_create(&pid_decode, NULL, decode_task, this);

    //2、播放
    pthread_create(&pid_render, NULL, render_task, this);

}

void VideoChannel::decode() {

    AVPacket *packet = NULL;
    while (isPlaying) {
        int ret = packets.pop(packet);
        if (!isPlaying) {
            break;
        }
        if (!ret) {
            continue;
        }
        ret = avcodec_send_packet(avCodecContext, packet);
        releaseAvPacket(packet);
        if (ret != 0) {
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

void VideoChannel::render() {

    swsContext = sws_getContext(avCodecContext->width, avCodecContext->height,
                                avCodecContext->pix_fmt,
                                avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA,
                                SWS_BILINEAR, NULL, NULL, NULL);
    //每个画面刷新的间隔
    double frame_delays = 1.0 / fps;
    AVFrame *frame = NULL;
    uint8_t *dst_data[4];
    int dst_linesize[4];
    av_image_alloc(dst_data, dst_linesize,
                   avCodecContext->width, avCodecContext->height, AV_PIX_FMT_RGBA, 1);

    LOGE("开始宣染");
    while (isPlaying) {
        int ret = frames.pop(frame);
        if (!isPlaying) {
            break;
        }
        sws_scale(swsContext, (const uint8_t *const *) frame->data,
                  frame->linesize, 0, avCodecContext->height, dst_data, dst_linesize);
        double clock = frame->best_effort_timestamp * av_q2d(time_base);

        if (audioChannel == NULL) {
            //休眠 单位微秒
            av_usleep(frame_delays * 1000000);
        } else {
            if (clock == 0) {
                av_usleep(frame_delays * 1000000);
            } else {
                //比较音频和视频
                double audioClock = audioChannel->clock;
                //间隔 音视频相关的间隔
                double diff = clock - audioClock;
//                LOGE("clock:%f, audioClock:%f", clock, audioClock);
                if (diff > 0) {
                    av_usleep((frame_delays + diff) * 1000000);
                } else if (diff < 0) {
                    //不睡了，快点赶上音频
                }

            }
        }



        callback(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
//        LOGE("宣染");
        releaseAvFrame(frame);
    }
    av_freep(&dst_data[0]);
    releaseAvFrame(frame);
}

void VideoChannel::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback;
}
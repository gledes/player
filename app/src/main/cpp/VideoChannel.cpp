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

/**
 * 丢包直到下一个关键帧
 * @param q
 */
void dropAvPacket(queue<AVPacket *>& q) {
    while (!q.empty()) {
        AVPacket *packet = q.front();
        if (packet->flags != AV_PKT_FLAG_KEY) {
            //如果不属于I帧 就丢掉
            BaseChannel::releaseAvPacket(packet);
            q.pop();
        } else {
            break;
        }
    }

}

void dropAvFrame(queue<AVFrame *>& q) {
    if (!q.empty()) {
        AVFrame *frame = q.front();
        BaseChannel::releaseAvFrame(frame);
        q.pop();
    }
}

VideoChannel::VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext, AVRational time_base, int fps)
        : BaseChannel(id, javaCallHelper, avCodecContext, time_base), fps(fps) {

    packets.setSyncHandle(dropAvPacket);
    packets.sync();

    frames.setSyncHandle(dropAvFrame);
    frames.sync();
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
            releaseAvPacket(packet);
            break;
        }
        if (!ret) {
            releaseAvPacket(packet);
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
//        if (ret == AVERROR(EAGAIN)) {
//            continue;
//        } else if (ret != 0) {
//            break;
//        }
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
            releaseAvFrame(frame);
            break;
        }
        sws_scale(swsContext, (const uint8_t *const *) frame->data,
                  frame->linesize, 0, avCodecContext->height, dst_data, dst_linesize);


        /**
         *  seek需要注意的点：编码器中存在缓存
         *  100s 的图像,用户seek到第 50s 的位置
         *  音频是50s的音频，但是视频 你获得的是100s的视频
         */
        if ((clock =frame->best_effort_timestamp) == AV_NOPTS_VALUE) {
            clock = 0;
        }
        //pts 单位就是time_base
        //av_q2d转为双精度浮点数 乘以 pts 得到pts --- 显示时间:秒
        clock = clock * av_q2d(time_base);
        //额外的间隔时间
        double extra_delay = frame->repeat_pict /(2*fps);
        //真实需要间隔的时间
        double delays = extra_delay + frame_delays;
        if (clock == 0) {
            av_usleep(delays * 1000000);
        } else {
            //比较音频和视频
            double audioClock = audioChannel ? audioChannel->clock : 0;
            //间隔 音视频相关的间隔
            double diff = fabs(clock - audioClock);
            LOGE("当前和音频比较:%f - %f = %f", clock, audioClock, diff);
//                LOGE("clock:%f, audioClock:%f", clock, audioClock);
            if (audioChannel) {
                //如果视频比音频快，延迟差值播放，否则直接播放
                if (clock > audioClock) {
                    if (diff > 1) {
                        //差的太久了， 那只能慢慢赶 不然就是卡好久
                        av_usleep(delays * 2 * 1000000);
                    } else {
                        av_usleep((delays + diff) * 1000000);
                    }
                } else {
                    //音频比视频快
                    //视频慢了 0.05s 已经比较明显了 (丢帧)
                    if (diff >= 0.05) {
                        releaseAvFrame(frame);
                        frames.sync();
                    }
//                    if (diff > 1) {
//                        //一种可能：快进了（因为解码器中有缓存数据，这样获得的avframe就和seek的匹配了）
//                    }

                }
            }

        }



        callback(dst_data[0], dst_linesize[0], avCodecContext->width, avCodecContext->height);
//        LOGE("宣染");
        releaseAvFrame(frame);
    }
    av_freep(&dst_data[0]);
    releaseAvFrame(frame);
    isPlaying = false;
    sws_freeContext(swsContext);
    swsContext = NULL;

}

void VideoChannel::setRenderFrameCallback(RenderFrameCallback callback) {
    this->callback = callback;
}

void VideoChannel::stop() {
    isPlaying = false;
    packets.setWork(false);
    frames.setWork(false);
    pthread_join(pid_decode, 0);
    pthread_join(pid_render, 0);
}

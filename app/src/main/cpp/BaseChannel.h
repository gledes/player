//
// Created by jin on 2018/9/17.
//

#ifndef PLAYER_BASECHANNEL_H
#define PLAYER_BASECHANNEL_H

extern "C" {
#include <libavcodec/avcodec.h>
};

#include "safe_queue.h"

class BaseChannel {
public:
    BaseChannel(int id, AVCodecContext *avCodecContext, AVRational time_base):id(id), avCodecContext(avCodecContext), time_base(time_base){
        frames.setReleaseCallback(releaseAvFrame);
        packets.setReleaseCallback(releaseAvPacket);
    }

    virtual ~BaseChannel(){

        packets.clear();
        frames.clear();
        if (avCodecContext) {
            avcodec_close(avCodecContext);
            avcodec_free_context(&avCodecContext);
            avCodecContext = NULL;
        }
    }

    static void releaseAvPacket(AVPacket*& packet) {
        if (packet) {
            av_packet_free(&packet);
            packet = NULL;
        }
    }

    static void releaseAvFrame(AVFrame*& frame) {
        if (frame) {
            av_frame_free(&frame);
            frame = NULL;
        }
    }

    virtual void play() = 0;

public:
    int id;
    SafeQueue<AVPacket *> packets;
    SafeQueue<AVFrame *> frames;
    bool isPlaying = false;
    AVCodecContext *avCodecContext;
    AVRational time_base;
    double clock;
};


#endif //PLAYER_BASECHANNEL_H

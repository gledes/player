//
// Created by jin on 2018/9/16.
//

#ifndef PLAYER_VIDEOCHANNEL_H
#define PLAYER_VIDEOCHANNEL_H


#include "BaseChannel.h"
#include "AudioChannel.h"
#include "JavaCallHelper.h"

extern "C" {
#include <libswscale/swscale.h>
}

typedef void (*RenderFrameCallback)(uint8_t *, int, int, int);

class VideoChannel : public BaseChannel{
public:
    VideoChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext, AVRational time_base, int fps);

    ~VideoChannel();

    void setAudioChannel(AudioChannel *audioChannel);

    void decode();

    void render();

    void play();

    void setRenderFrameCallback(RenderFrameCallback callback);

    void stop();

private:
    pthread_t pid_decode;
    pthread_t pid_render;
    SwsContext *swsContext = NULL;
    RenderFrameCallback callback = NULL;
    AudioChannel *audioChannel = NULL;
    int fps;

};


#endif //PLAYER_VIDEOCHANNEL_H
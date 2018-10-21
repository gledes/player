//
// Created by jin on 2018/9/16.
//

#ifndef PLAYER_AUDIOCHANNEL_H
#define PLAYER_AUDIOCHANNEL_H


#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "BaseChannel.h"
#include "JavaCallHelper.h"

extern "C" {
#include <libswresample/swresample.h>
}

class AudioChannel : public BaseChannel{

public:
    AudioChannel(int id, JavaCallHelper *javaCallHelper, AVCodecContext *avCodecContext, AVRational time_base);

    ~AudioChannel();

    void play();

    void decode();

    void _play();

    int getPcm();

    void stop();

public:
    uint8_t *data = NULL;

private:
    pthread_t pid_audio_decode;
    pthread_t pid_audio_play;
    //引擎
    SLObjectItf engineObject = NULL;
    //引擎接口
    SLEngineItf engineInterface = NULL;
    //混音器
    SLObjectItf outputMixObject = NULL;
    //播放器
    SLObjectItf bqPlayerObject = NULL;
    //播放器接口
    SLPlayItf bqPlayerInterface = NULL;

    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueueInterface = NULL;

    //重采样
    SwrContext *swrContext = NULL;


};


#endif //PLAYER_AUDIOCHANNEL_H
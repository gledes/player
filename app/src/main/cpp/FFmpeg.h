//
// Created by jin on 2018/9/16.
//

#ifndef PLAYER_FFMPEG_H
#define PLAYER_FFMPEG_H



#include "JavaCallHelper.h"
#include "AudioChannel.h"
#include "VideoChannel.h"


extern "C" {
#include <libavformat/avformat.h>
}

class FFmpeg {

    friend void* task_prepare(void *args);

    friend void* task_play(void *args);

public:
    FFmpeg(JavaCallHelper *callHelper, const char *dataSource);

    ~FFmpeg();

    void prepare();

    void start();

    void setRenderFrameCallback(RenderFrameCallback callback);



private:
    void _prepare();

    void _start();


private:
    char *dataSource;
    pthread_t pid;
    pthread_t pid_play;
    //包含视频宽、高等信息
    AVFormatContext *formatContext = NULL;

    JavaCallHelper *callHelper = NULL;
    AudioChannel *audioChannel = NULL;
    VideoChannel *videoChannel = NULL;

    bool isPlaying = false;
    RenderFrameCallback callback = NULL;

};

#endif //PLAYER_FFMPEG_H
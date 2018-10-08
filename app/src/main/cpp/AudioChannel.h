//
// Created by jin on 2018/9/16.
//

#ifndef PLAYER_AUDIOCHANNEL_H
#define PLAYER_AUDIOCHANNEL_H


#include "BaseChannel.h"

class AudioChannel : public BaseChannel{

public:
    AudioChannel(int id, AVCodecContext *avCodecContext);

    ~AudioChannel() {

    }

    void play();

};


#endif //PLAYER_AUDIOCHANNEL_H
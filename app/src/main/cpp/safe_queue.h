//
// Created by jin on 2018/9/15.
//

#ifndef PLAYER_SAFE_QUEUE_H
#define PLAYER_SAFE_QUEUE_H

#endif //PLAYER_SAFE_QUEUE_H

#include <queue>
#include <pthread.h>

using namespace std;


template <class T>
class SafeQueue {

    typedef void (*ReleaseCallback)(T&);

    typedef void (*SyncHandle)(queue<T> &);

public:
    SafeQueue() {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }
    ~SafeQueue() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void push(T value) {
        pthread_mutex_unlock(&mutex);
        if (work) {
            q.push(value);
            pthread_cond_signal(&cond);
//            pthread_mutex_unlock(&mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

    int pop(T& value) {
        int ret = 0;
        pthread_mutex_lock(&mutex);
        while (work && q.empty()) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mutex);
        return ret;
    }

    int empty() {
        return q.empty();
    }

    int size() {
        return q.size();
    }

    void clear() {
        pthread_mutex_lock(&mutex);
        int32_t size = q.size();
        for (int i = 0; i < size; ++i) {
            T value = q.front();
            if (NULL != releaseCallback) {
                releaseCallback(value);
            }
            q.pop();
        }

        pthread_mutex_unlock(&mutex);
    }

    void setReleaseCallback(ReleaseCallback releaseCallback) {
        this->releaseCallback = releaseCallback;
    }

    void setSyncHandle(SyncHandle syncHandle) {
        this->syncHandle = syncHandle;
    }

    void setWork(bool work) {
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    void sync() {
        pthread_mutex_lock(&mutex);
        syncHandle(q);
        pthread_mutex_unlock(&mutex);
    }

private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    queue<T> q;

    bool work;
    ReleaseCallback releaseCallback;
    SyncHandle syncHandle;

};
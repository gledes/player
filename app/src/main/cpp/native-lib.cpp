#include <jni.h>
#include <string>
#include <android/native_window_jni.h>

#include "FFmpeg.h"
#include "macro.h"

FFmpeg *ffmpeg = NULL;
JavaCallHelper *callHelper = NULL;
JavaVM *_vm;
ANativeWindow *window = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int JNI_OnLoad(JavaVM *vm, void *re) {
    _vm = vm;
    return JNI_VERSION_1_6;
}

//画画
void render(uint8_t *data, int linesize, int w, int h) {
    pthread_mutex_lock(&mutex);
    if (!window) {
        pthread_mutex_unlock(&mutex);
        return;
    }

    //设置窗口属性
    ANativeWindow_setBuffersGeometry(window, w, h, WINDOW_FORMAT_RGBA_8888);
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0)) {
        ANativeWindow_release(window);
        window = NULL;
        pthread_mutex_unlock(&mutex);
        return;
    }

    //填充RGB数据给dst_data
    uint8_t *dst_data = static_cast<uint8_t *> (window_buffer.bits);
    //stride:一行多少个数据(RGBA) * 4
    int dst_linesize = window_buffer.stride * 4;
//    LOGE("画画dst_linesize的大小：%d", dst_linesize);
    //一行一行的拷贝
    for (int i = 0; i < window_buffer.height; ++i) {
        memcpy(dst_data + i * dst_linesize, data + i * linesize, dst_linesize);
    }
    ANativeWindow_unlockAndPost(window);
    pthread_mutex_unlock(&mutex);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_jin_player_Player_native_1prepare(JNIEnv *env, jobject instance,
                                                   jstring dataSource_) {
    const char *dataSource = env->GetStringUTFChars(dataSource_, 0);
    //内存释放
    callHelper = new JavaCallHelper(_vm, env, instance);
    ffmpeg = new FFmpeg(callHelper, dataSource);
    ffmpeg->setRenderFrameCallback(render);
    ffmpeg->prepare();
    env->ReleaseStringUTFChars(dataSource_, dataSource);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_jin_player_Player_native_1destory(JNIEnv *env, jobject instance) {

    pthread_mutex_lock(&mutex);
    if (window) {
        ANativeWindow_release(window);
        window = NULL;
    }

    pthread_mutex_unlock(&mutex);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_jin_player_Player_native_1start(JNIEnv *env, jobject instance) {

    ffmpeg->start();

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_jin_player_Player_native_1setSurface(JNIEnv *env, jobject instance,
                                                      jobject surface) {

    pthread_mutex_lock(&mutex);
    if (window) {
        ANativeWindow_release(window);
        window = NULL;
    }
    window = ANativeWindow_fromSurface(env, surface);

    pthread_mutex_unlock(&mutex);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_jin_player_Player_native_1stop(JNIEnv *env, jobject instance) {
    if (ffmpeg) {
        ffmpeg->stop();
    }
    if (callHelper) {
        delete(callHelper);
        callHelper = NULL;
    }

//    if (ffmpeg) {
//        delete(ffmpeg);
//        ffmpeg = NULL;
//    }

}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_jin_player_Player_native_1getDuration(JNIEnv *env, jobject instance) {

    if (ffmpeg) {
        return ffmpeg->getDuration();
    }

    return 0;

}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_jin_player_Player_native_1seek(JNIEnv *env, jobject instance, jint progress) {


    if (ffmpeg) {
        ffmpeg->seek(progress);
    }
}
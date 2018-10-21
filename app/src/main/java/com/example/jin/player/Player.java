package com.example.jin.player;

import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.Toast;

/**
 * Created by jin on 2018/9/16.
 */

public class Player implements SurfaceHolder.Callback {

    static {
        System.loadLibrary("native-lib");
    }

    private String dataSource;

    private SurfaceHolder holder;

    private OnPrepareListener onPrepareListener;

    private OnErrorListener onErrorListener;

    private OnProgressListener onProgressListener;


    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        if (holder != null) {
            holder.removeCallback(this);
        }
        holder = surfaceView.getHolder();
        holder.addCallback(this);

    }

    public void prepare() {
        native_prepare(dataSource);
    }

    public void start() {
        native_start();
    }

    public void stop() {
        native_stop();
    }

    public void release() {
        holder.removeCallback(this);
        native_destory();
    }

    public void onError(int errorCode) {
//        Toast.makeText()
    }

    public void onPrepare() {
        if (null != onPrepareListener) {
            onPrepareListener.onPrepare();
        }

    }

    /**
     * native 回调给java 播放进去的
     * @param progress
     */
    public void onProgress(int progress) {
//        Log.e("play", "" + progress);
        if (null != onProgressListener) {
            onProgressListener.onProgress(progress);
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {

    }

    /**
     * 画布发生改变，横竖屏切换，按home都会触发改变
     * @param holder
     * @param format
     * @param width
     * @param height
     */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        Log.e("Player", "画布变化");
        native_setSurface(holder.getSurface());
    }

    /**
     * 销毁画布（按了home/退出应用）
     * @param holder
     */
    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }

    public int getDuration() {
        return native_getDuration();
    }

    public void seek(final int progress) {
        new Thread() {
            @Override
            public void run() {
                native_seek(progress);
            }
        }.start();
    }

    public native void native_prepare(String dataSource);

    public native void native_start();

    public native void native_stop();

    public native void native_setSurface(Surface surface);

    public native void native_destory();

    private native int native_getDuration();

    private native void native_seek(int progress);

    public void setOnErrorListener(OnErrorListener onErrorListener) {
        this.onErrorListener = onErrorListener;
    }

    public void setOnPrepareListener(OnPrepareListener onPrepareListener) {
        this.onPrepareListener = onPrepareListener;
    }

    public void setOnProgressListener(OnProgressListener onProgressListener) {
        this.onProgressListener = onProgressListener;
    }

    public interface OnPrepareListener {
        void onPrepare();
    }

    public interface OnErrorListener {
        void onError(int error);
    }

    public interface OnProgressListener {
        void onProgress(int progress);
    }

}

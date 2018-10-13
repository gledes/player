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

    private OnPrepareListener lintener;


    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    public void setSurfaceView(SurfaceView surfaceView) {
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

    }

    public void release() {
        holder.removeCallback(this);
    }

    public void onError(int errorCode) {
//        Toast.makeText()
    }

    public void onPrepare() {
        if (null != lintener) {
            lintener.onPrepare();
        }

    }

    public void setOnPrepareLintener(OnPrepareListener lintener) {
        this.lintener = lintener;
    }

    public interface OnPrepareListener {
        void onPrepare();
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

    public native void native_prepare(String dataSource);

    public native void native_start();

    public native void native_setSurface(Surface surface);

//    public native void native_destory();


}

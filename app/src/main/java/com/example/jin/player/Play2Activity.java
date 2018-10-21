package com.example.jin.player;

import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.SeekBar;

import com.trello.rxlifecycle2.components.support.RxAppCompatActivity;

public class Play2Activity extends RxAppCompatActivity implements SeekBar.OnSeekBarChangeListener{

    private Player player;

    private SeekBar seekBar;

    private int progress;

    private boolean isSeek;

    private boolean isTouch;



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_play2);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager
                .LayoutParams.FLAG_KEEP_SCREEN_ON);
        SurfaceView surfaceView = findViewById(R.id.surfaceView2);
        player = new Player();
        player.setSurfaceView(surfaceView);
        player.setOnPrepareListener(new Player.OnPrepareListener() {
            /**
             * 视频信息获取完成 随时可以播放的时候回调
             */
            @Override
            public void onPrepare() {
                //获得时间
                int duration = player.getDuration();
                //直播： 时间就是0
                if (duration != 0){
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            //显示进度条
                            seekBar.setVisibility(View.VISIBLE);
                        }
                    });
                }
                player.start();
            }
        });
        player.setOnErrorListener(new Player.OnErrorListener() {
            @Override
            public void onError(int error) {

            }
        });
        player.setOnProgressListener(new Player.OnProgressListener() {

            @Override
            public void onProgress(final int progress2) {
                Log.e("play2Activity", "progress2:" + progress2);
                if (!isTouch) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            int duration = player.getDuration();
//                            Log.e("play2Activity", "duration:" + duration);
                            //如果是直播
                            if (duration != 0) {
                                if (isSeek){
                                    isSeek = false;
                                    return;
                                }
                                //更新进度 计算比例
                                seekBar.setProgress(progress2 * 100 / duration);
                            }
                        }
                    });
                }
            }
        });
        seekBar = findViewById(R.id.seekBar);
        seekBar.setOnSeekBarChangeListener(this);
        player.setDataSource("/sdcard/b.mp4");
    }

    @Override
    protected void onResume() {
        super.onResume();
        player.prepare();
    }

    @Override
    protected void onStop() {
        super.onStop();
        player.stop();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        player.release();
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {

    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
        isTouch = true;
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {

        isSeek = true;
        isTouch = false;
        progress = player.getDuration() * seekBar.getProgress() / 100;
        //进度调整
        player.seek(progress);
    }

    public void start(View view) {
        player.prepare();
    }
}

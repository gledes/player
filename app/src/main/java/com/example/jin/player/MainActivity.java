package com.example.jin.player;

import android.content.Intent;
import android.os.Bundle;
import android.support.design.widget.TabLayout;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;

import com.example.live.LiveManager;
import com.example.live.list.LiveList;
import com.example.live.room.Room;
import com.example.live.room.Videoinfo;
import com.trello.rxlifecycle2.LifecycleTransformer;
import com.trello.rxlifecycle2.android.ActivityEvent;
import com.trello.rxlifecycle2.components.support.RxAppCompatActivity;

import org.reactivestreams.Subscriber;
import org.reactivestreams.Subscription;

import io.reactivex.Flowable;
import io.reactivex.Scheduler;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.schedulers.Schedulers;
import io.reactivex.subscribers.DisposableSubscriber;

public class MainActivity extends RxAppCompatActivity implements TabLayout.BaseOnTabSelectedListener, LiveAdapter.OnItemClickListener{

    private TabLayout tabLayout;

    private RecyclerView recyclerView;

    private LiveAdapter liveAdapter;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        //配置recyclerView
        recyclerView = findViewById(R.id.recycleView);
        LinearLayoutManager layoutManager = new LinearLayoutManager(this);
        layoutManager.setOrientation(LinearLayoutManager.VERTICAL);
        recyclerView.setLayoutManager(layoutManager);
        liveAdapter = new LiveAdapter(this);
        liveAdapter.setItemClickListener(this);
        recyclerView.setAdapter(liveAdapter);


        tabLayout = findViewById(R.id.tabLayout);
        tabLayout.addOnTabSelectedListener(this);

        addTabs();

//        player = new Player();
//        player.setSurfaceView(surfaceView);
////        player.setDataSource("rtmp://live.hkstv.hk.lxdns.com/live/hks");
//        player.setDataSource("http://pl3.live.panda.tv/live_panda/a565062faa92b2497fba298df9fc22ac_mid.flv?sign=e55498bcd19de9ed1312472695b84cc7&time=&ts=5ba310f9&rid=-70185294");
//        player.setOnPrepareLintener(new Player.OnPrepareListener() {
//            @Override
//            public void onPrepare() {
//                runOnUiThread(new Runnable() {
//                    @Override
//                    public void run() {
//                        Toast.makeText(MainActivity.this, "play", Toast.LENGTH_SHORT).show();
//                    }
//                });
//                player.start();
//            }
//        });

    }

    private void addTabs() {
        addTab("lol", "英雄联盟");
        addTab("acg", "二次元");
        addTab("food", "美食");
    }

    private void addTab(String tag, String title) {
        TabLayout.Tab tab = tabLayout.newTab();
        tab.setTag(tag);
        tab.setText(title);
        tabLayout.addTab(tab);
    }

    public void onTabSelected1(TabLayout.Tab tab) {
        //请求获取房间 todo 显示加载等待
        LiveManager.getInstance()
                .getLiveList(tab.getTag().toString())
                .compose(this.<LiveList>bindUntilEvent(ActivityEvent.DESTROY))
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new DisposableSubscriber<LiveList>() {

                    @Override
                    public void onNext(LiveList liveList) {
                        liveAdapter.setLiveList(liveList);
                        liveAdapter.notifyDataSetChanged();
                    }

                    @Override
                    public void onError(Throwable t) {
                        t.printStackTrace();
                    }

                    @Override
                    public void onComplete() {

                    }
                });
    }

    /**
     * 切换标签回调
     * @param tab
     */
    public void onTabSelected(TabLayout.Tab tab) {
        Flowable<LiveList>  flowable = LiveManager.getInstance().getLiveList(tab.getTag().toString());
        flowable.subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new Subscriber<LiveList>() {
                    @Override
                    public void onSubscribe(Subscription s) {
                        System.out.println("onSubscribe");
                        s.request(Long.MAX_VALUE);

                    }

                    @Override
                    public void onNext(LiveList liveList) {
                        liveAdapter.setLiveList(liveList);
                        liveAdapter.notifyDataSetChanged();
                    }

                    @Override
                    public void onError(Throwable t) {

                    }

                    @Override
                    public void onComplete() {
                        System.out.println("onComplete");
                    }
                });

    }

    @Override
    public void onTabUnselected(TabLayout.Tab tab) {

    }

    @Override
    public void onTabReselected(TabLayout.Tab tab) {

    }

    @Override
    public void onItemClick(String id) {

        LiveManager.getInstance().getLiveRoom(id)
                .subscribeOn(Schedulers.io())
                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new Subscriber<Room>() {
                    @Override
                    public void onSubscribe(Subscription s) {
                        s.request(Long.MAX_VALUE);
                    }

                    @Override
                    public void onNext(Room room) {
                        Videoinfo info = room.getData().getInfo().getVideoinfo();
                        String[] plflags = info.getPlflag().split("_");
                        String room_key =  info.getRoom_key();
                        String sign = info.getSign();
                        String ts = info.getTs();
                        Intent intent = new Intent(MainActivity.this, PlayActivity.class);
                        String v = "3";
                        if (null != plflags && plflags.length > 0) {
                            v = plflags[plflags.length - 1];
                        }
                        String url = "http://pl" + v + ".live" +
                                ".panda.tv/live_panda/" + room_key
                                + "_mid" +
                                ".flv?sign=" + sign +
                                "&time=" + ts;
                        intent.putExtra("url", url);
                        startActivity(intent);
                    }

                    @Override
                    public void onError(Throwable t) {

                    }

                    @Override
                    public void onComplete() {

                    }
                });

    }


}

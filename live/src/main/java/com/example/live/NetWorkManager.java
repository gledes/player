package com.example.live;

import org.reactivestreams.Subscriber;
import org.reactivestreams.Subscription;

import java.io.IOException;
import java.util.List;

import io.reactivex.BackpressureStrategy;
import io.reactivex.Flowable;
import io.reactivex.FlowableEmitter;
import io.reactivex.FlowableOnSubscribe;
import io.reactivex.Scheduler;
import io.reactivex.android.schedulers.AndroidSchedulers;
import io.reactivex.functions.Consumer;
import io.reactivex.functions.Function;
import io.reactivex.schedulers.Schedulers;
import io.reactivex.subscribers.DisposableSubscriber;
import retrofit2.Call;
import retrofit2.Callback;
import retrofit2.Response;
import retrofit2.Retrofit;
import retrofit2.adapter.rxjava2.RxJava2CallAdapterFactory;
import retrofit2.converter.gson.GsonConverterFactory;

/**
 * Created by jin on 2018/9/21.
 */

public class NetWorkManager {

    public static final String BASE_URL = "https://api.github.com/";


    public static void testRetrofit() throws IOException {

        Retrofit retrofit = new Retrofit.Builder()
                .baseUrl(BASE_URL)
                .addCallAdapterFactory(RxJava2CallAdapterFactory.create())
                .addConverterFactory(GsonConverterFactory.create())
                .build();
        NetWorkService service = retrofit.create(NetWorkService.class);
        Flowable<DataBean> call = service.getData();
        call.subscribeOn(Schedulers.io())
//                .observeOn(AndroidSchedulers.mainThread())
                .subscribe(new DisposableSubscriber<DataBean>() {
                    @Override
                    public void onNext(DataBean dataBean) {
                        System.out.println("onNext");
                    }

                    @Override
                    public void onError(Throwable t) {

                    }

                    @Override
                    public void onComplete() {
                        System.out.println("onComplete");
                    }
                });

//        DataBean bean = call.execute().body();
//        System.out.println(bean.getLogin());
//        call.enqueue(new Callback<DataBean>() {
//            @Override
//            public void onResponse(Call<DataBean> call1, Response<DataBean> response) {
//                DataBean bean = response.body();
//                System.out.println(bean.getLogin());
//            }
//
//            @Override
//            public void onFailure(Call<DataBean> call, Throwable t) {
//
//            }
//        });


//        Call<List<DataBean>> call_1 = service.getData("Square", "Retrofit");
//        List<DataBean> beans = call_1.execute().body();
//        for (int i = 0; i < beans.size(); i++) {
//            DataBean bean1 = beans.get(i);
//            System.out.println(bean1.getId());
//        }


    }

    public static void testFlowable() {
        Flowable<String> flowable = Flowable.create(new FlowableOnSubscribe<String>() {
            @Override
            public void subscribe(FlowableEmitter<String> emitter) throws Exception {

                emitter.onNext("hello RxJava2");
                System.out.println("hello");
                emitter.onComplete();

            }
        }, BackpressureStrategy.BUFFER);


        Subscriber subscriber = new Subscriber() {
            @Override
            public void onSubscribe(Subscription s) {
                System.out.println("onSubscribe");
                s.request(Long.MAX_VALUE);
            }

            @Override
            public void onNext(Object o) {
                System.out.println(o);
            }

            @Override
            public void onError(Throwable t) {

            }

            @Override
            public void onComplete() {
                System.out.println("onComplete");
            }
        };

        flowable.subscribe(subscriber);
    }

    public static void flowableJust() {
        Flowable<String> flowable = Flowable.just("hello Rxjava");
        Consumer<String> consumer = new Consumer<String>() {
            @Override
            public void accept(String s) throws Exception {
                System.out.println(s);
            }
        };
        flowable.subscribe(consumer);
    }

    public static void flowableJustMap() {
        Flowable.just("mapaaa")
                .map(new Function<String, Integer>() {

                    @Override
                    public Integer apply(String s) throws Exception {
                        s = s + " -ittianyu";
                        return s.hashCode();
                    }
                })
                .map(new Function<Integer, Object>() {

                    public String apply(Integer s) throws Exception {
                        System.out.println(s);
                        return s.toString();
                    }
                })
                .subscribe(new Consumer<Object>() {
                    @Override
                    public void accept(Object o) throws Exception {
                        System.out.println(o);
                    }
                });
    }

    public static void main(String args[]) throws IOException {
        testRetrofit();
//        testFlowable();

//        Flowable.just("hello rx")
//                .subscribe(s -> System.out.println("xxx"));







    }



}

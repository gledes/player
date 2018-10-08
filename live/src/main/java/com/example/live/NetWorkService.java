package com.example.live;

import java.util.List;

import io.reactivex.Flowable;
import retrofit2.Call;
import retrofit2.http.GET;
import retrofit2.http.Path;
import retrofit2.http.Query;

/**
 * Created by jin on 2018/9/21.
 */

public interface NetWorkService {

    //https://api.github.com/users/basil2style
    @GET("users/basil2style")
    Flowable<DataBean> getData();

    //https://api.github.com/repos/Square/Retrofit/contributors
    @GET("repos/{params1}/{params2}/contributors")
    Call<List<DataBean>> getData(
            @Path("params1") String params1,
            @Path("params2") String params2
    );

    @GET("repos/square/{retrofit}/contributors")
    Call<List<DataBean>> groupData(@Path("retrofit") String retrofit, @Query("sort") String sort);


}

package com.example.jin.player;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.TextView;

import com.bumptech.glide.Glide;
import com.example.live.list.Items;
import com.example.live.list.LiveList;

import java.util.ArrayList;
import java.util.List;

public class LiveAdapter extends RecyclerView.Adapter<LiveAdapter.MyHolder> implements View.OnClickListener {

    private LayoutInflater layoutInflater;

    private List<Items> items = new ArrayList<Items>();

    private OnItemClickListener itemClickListener = null;

    public LiveAdapter(Context context) {
        layoutInflater = LayoutInflater.from(context);
    }


    public void setItemClickListener(OnItemClickListener itemClickListener) {
        this.itemClickListener = itemClickListener;
    }

    @Override
    public void onClick(View v) {
        if (itemClickListener != null) {
            itemClickListener.onItemClick(v.getTag().toString());
        }
    }

    public void setLiveList(LiveList liveList) {
        items = liveList.getData().getItems();
    }

    @NonNull
    @Override
    public LiveAdapter.MyHolder onCreateViewHolder(@NonNull ViewGroup viewGroup, int i) {
        View view = layoutInflater.inflate(R.layout.item_room, viewGroup, false);
        view.setOnClickListener(this);

        return new MyHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull LiveAdapter.MyHolder myHolder, int i) {
        Items data = items.get(i);

        myHolder.title.setText(data.getName());
        Glide.with(myHolder.picture).load(data.getPictures().getImg()).into(myHolder.picture);
        myHolder.itemView.setTag(data.getId());

    }

    @Override
    public int getItemCount() {
        return items.size();
    }

    public interface OnItemClickListener {
        void onItemClick(String id);
    }

    class MyHolder extends RecyclerView.ViewHolder {

        ImageView picture;
        TextView title;
        public MyHolder(@NonNull View itemView) {
            super(itemView);
            picture = itemView.findViewById(R.id.picture);
            title = itemView.findViewById(R.id.title);

        }
    }
}

package com.example.live.observer;

import java.util.Observable;
import java.util.Observer;

public class Subscribe implements Observer {

    private String name;

    public Subscribe(String name) {
        this.name = name;
    }

    @Override
    public void update(Observable o, Object arg) {
        System.out.println(name + "看到股市点数：" + arg);
    }
}

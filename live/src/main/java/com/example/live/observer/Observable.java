package com.example.live.observer;

import java.util.ArrayList;
import java.util.List;

public class Observable {

    List<Observer> list = new ArrayList<Observer>();

    public void register(Observer observer) {
        list.add(observer);
    }


    public void notifyObserver(Object arg) {
        for (int i = 0; i < list.size(); i++) {
            Observer observer = list.get(i);
            observer.update(arg);
        }
    }

    public void unRegister(Observer observer) {
        list.remove(observer);
    }

}

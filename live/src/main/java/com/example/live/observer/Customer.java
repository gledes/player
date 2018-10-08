package com.example.live.observer;

public class Customer implements Observer {

    private String name;

    public Customer(String name) {
        this.name = name;
    }

    @Override
    public void update(Object arg) {
        System.out.println(name + "拿到" + arg);
    }
}

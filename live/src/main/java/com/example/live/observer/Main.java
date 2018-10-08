package com.example.live.observer;

import java.util.Random;

public class Main {

    public static void market() {
        EquityMarket equityMarket = new EquityMarket();
        equityMarket.addObserver(new Subscribe("张三"));
        equityMarket.addObserver(new Subscribe("李四"));
        equityMarket.addObserver(new Subscribe("王五"));


        Random r = new Random();
        for (int i = 0; i < 10; i++) {
            int a = (int)(100 * r.nextFloat());
            equityMarket.changed();
            equityMarket.notifyObservers(a);
        }

    }
    public static void main(String args[]) {
        testObserver();
        System.out.println("hello");
    }

    public static void testObserver() {
        Observable observable = new Observable();
        Customer 张三 = new Customer("张三");
        observable.register(张三);
        Customer 李四 = new Customer("李四");
        observable.register(李四);
        Customer 王五 = new Customer("王五");
        observable.register(new Customer("王五"));

        observable.notifyObserver("报纸");
        observable.unRegister(李四);
        observable.notifyObserver("牛奶");

    }
}

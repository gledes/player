package com.example.live.observer;

import java.util.Observable;

public class EquityMarket extends Observable {


    public void changed() {
        setChanged();
    }
}

package com.example.live;

import java.util.ArrayList;
import java.util.List;

public class Test {

    public static void main(String args[]) {


        System.out.println("hello");

        List<? super A1> list = new ArrayList<A1>();
        list.add(new A1());
        list.add(new A2());
//        list.add(new A());
    }



}

class A {
    public int i;
}

class A1 extends A {

}

class A2 extends  A1 {

}



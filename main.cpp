//
//  main.cpp
//  FunnyCoding
//
//  Created by Xplorld on 2016/3/27.
//  Copyright © 2016年 xplorld. All rights reserved.
//
#include "XorLinkedList.hpp"
#include <cstdio>
#include <vector>
#include <list>
#include <deque>
#include <typeinfo>
#include <algorithm>

class Shouter {
public:
    int i;
    void hw() const {
        printf("hello,world,I am %d!\n",i);
    }
    void setI(int i) {this->i = i;};
    
    ~Shouter() {
        printf("I died,     I am %d!\n",i);
    }
    Shouter(Shouter &&t) {
        i = t.i;
        t.i = 0;
        printf("I moved,    I am %d!\n",i);
    }
    
    Shouter(Shouter const &t) {
        i = t.i;
        printf("I copied,   I am %d!\n",i);
    }
    Shouter(int i) : i(i) {
        printf("I am born,  I am %d!\n",i);
    }
};


template <typename container>
void initWithContainer() {
    printf("initWithContainer of %s begin\n",typeid(container).name());
    
    auto cont = container({Shouter(0),Shouter(1),Shouter(2)});
    auto list_l = XorLinkedList<Shouter>(cont);
    printf("I am from lvalue %s\n",typeid(container).name());
    for (auto &i: list_l) {
        i.hw();
    }
    
    auto list_r = XorLinkedList<Shouter>(container({Shouter(0),Shouter(1),Shouter(2)}));
    printf("I am from rvalue %s\n",typeid(container).name());
    for (auto &i: list_l) {
        i.hw();
    }
    printf("initWithContainer of %s end\n",typeid(container).name());
}
void initWithContainers() {
    printf("initWithContainers test begin\n");
    
    initWithContainer<std::vector<Shouter>>();
    initWithContainer<std::list<Shouter>>();
    initWithContainer<std::deque<Shouter>>();
    initWithContainer<std::initializer_list<Shouter>>();
    
    printf("initWithContainers test end\n");
}

template <typename container>
void initWithIterator() {
    
    printf("initWithIterator of %s begin\n",typeid(container).name());
    auto cont = container({Shouter(0),Shouter(1),Shouter(2)});
    auto list_iter = XorLinkedList<Shouter>(cont.begin(),cont.end());
    printf("I am from begin/end %s\n",typeid(container).name());
    for (auto &i: list_iter) {
        i.hw();
    }
    printf("initWithIterator of %s end\n",typeid(container).name());
}

void initWithIterators() {
    printf("initWithIterators test begin\n");
    
    initWithIterator<std::vector<Shouter>>();
    initWithIterator<std::list<Shouter>>();
    initWithIterator<std::deque<Shouter>>();
    initWithIterator<std::initializer_list<Shouter>>();
    
    Shouter arr[3] = {Shouter(0),Shouter(1),Shouter(2)};
    auto list_arr = XorLinkedList<Shouter>(arr,3);
    auto list_arr_p = XorLinkedList<Shouter>(arr,arr+3);
    
    printf("initWithIterators test end\n");
}

void initWithSingleElement() {
    
    printf("initWithSingleElement test begin\n");
    
    auto from_rvalue = XorLinkedList<Shouter>(Shouter(1));
    auto s = Shouter(2);
    auto from_lvalue_ref = XorLinkedList<Shouter>(s);
    auto from_lvalue_p = XorLinkedList<Shouter>(&s);
    auto from_void = XorLinkedList<Shouter>();
    
    printf("initWithSingleElement test end\n");
}

void copyAndMove() {
    printf("copyAndMove test begin\n");
    
    auto list_original = XorLinkedList<Shouter>({Shouter(0),Shouter(1),Shouter(2)});
    auto list_copied = XorLinkedList<Shouter>(list_original);
    auto list_moved = XorLinkedList<Shouter>(std::move(list_original));
    printf("original list:(should be empty)\n");
    assert(list_original.size() == 0 && "moved list not empty\n");
    for (auto &i : list_original) {
        i.hw();
    }
    printf("copied list:\n");
    for (auto &i : list_copied) {
        i.hw();
    }
    printf("moved list:\n");
    for (auto &i : list_moved) {
        i.hw();
    }
    
    printf("copyAndMove test end\n");
}

void appendAndJoin() {
    printf("appendAndJoin test begin\n");
    auto list_1 = XorLinkedList<Shouter>({Shouter(0),Shouter(1),Shouter(2)});
    auto size_1 = list_1.size();
    auto list_2 = XorLinkedList<Shouter>({Shouter(3),Shouter(4),Shouter(5)});
    for (auto &i: list_2) {
        list_1.push_back(i);
    }
    assert(list_1.size() == size_1 + list_2.size() && "append size mismatch");
    for (auto &i : list_2) {
        i.hw();
    }
    
    //TODO: join has not been implemented
    //list_1.join(list_2);
    //list_1.join(std::move(list_2));
    printf("appendAndJoin test end\n");
}

void iterators() {
    printf("iterators test begin\n");
    auto list = XorLinkedList<Shouter>({Shouter(0),Shouter(1),Shouter(2)});
    for (auto i = list.begin(); i != list.end(); ++i) {
        //iterator is mutable
        i->setI(i->i + 2);
    }
    for (auto i = list.cbegin(); i != list.cend(); ++i) {
        //i->setI(i->i + 2); //error: iterator is const
        i->hw(); //good, const methods
    }
    for (auto i = list.rbegin(); i != list.rend(); ++i) {
        //reverse iterator is mutable
        i->setI(i->i + 2);
    }
    for (auto i = list.crbegin(); i != list.crend(); ++i) {
        //i->setI(i->i + 2); //error: iterator is const
        i->hw(); //good, const methods
    }
    printf("iterators test end\n");
}

int main() {
    initWithSingleElement();
    initWithContainers();
    initWithIterators();
    copyAndMove();
    appendAndJoin();
    iterators();
}
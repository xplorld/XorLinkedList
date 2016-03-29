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
#include <algorithm>

class Shouter {
public:
    int i;
    void hw() {
        printf("hello,world,I am %d!\n",i);
    }
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

void copyFromVector() {
    printf("vector init\n");
    auto vector = std::vector<Shouter>({Shouter(0),Shouter(1),Shouter(2)});
    printf("XorLinkedList init \n");
    auto list = XorLinkedList<Shouter>(std::begin(vector),std::end(vector));
    printf("iteration init \n");
    for (auto i = list.begin(); i != list.end(); ++i) {
        i->hw();
    }
    printf("RAII is your friend!\n");
}
void copyFromInitializerList() {
    printf("XorLinkedList init \n");
    auto list = XorLinkedList<Shouter>({Shouter(0),Shouter(1),Shouter(2)});
    printf("iteration init \n");
    for (auto i = list.begin(); i != list.end(); ++i) {
        i->hw();
    }
    printf("RAII is your friend!\n");
}

void copyFromArray() {
    Shouter arr[3] = {Shouter(0),Shouter(1),Shouter(2)};
    auto list = XorLinkedList<Shouter>(arr,3);
    printf("iteration init \n");
    for (auto i = list.begin(); i != list.end(); ++i) {
        i->hw();
    }
    printf("RAII is your friend!\n");
}

void moveFromArray() {
    Shouter arr[3] = {Shouter(0),Shouter(1),Shouter(2)};
    auto list = XorLinkedList<Shouter>(
                                       std::make_move_iterator(arr),
                                       std::make_move_iterator(arr+3)
                                       );
    printf("iteration init \n");
    auto i = list.begin();
    for (; i != list.end(); ++i) {
        i->hw();
    }
    for (--i; i != --list.begin(); --i) {
        i->hw();
    }
    printf("RAII is your friend!\n");
}

void moveSelf() {
    Shouter arr[3] = {Shouter(0),Shouter(1),Shouter(2)};
    auto list = XorLinkedList<Shouter>(
                                       std::make_move_iterator(arr),
                                       std::make_move_iterator(arr+3)
                                       );
    auto another = std::move(list);
    printf("iteration init \n");
    for (auto i = another.begin(); i != another.end(); ++i) {
        i->hw();
    }
    printf("what about list?\n");
    for (auto i = list.begin(); i != list.end(); ++i) {
        i->hw();
    }
    printf("RAII is your friend!\n");
}

void forwardAndBackward() {
    Shouter arr[3] = {Shouter(0),Shouter(1),Shouter(2)};
    auto list = XorLinkedList<Shouter>(
                                       std::make_move_iterator(arr),
                                       std::make_move_iterator(arr+3)
                                       );
    printf("iteration forward \n");
    for (auto i = list.begin(); i != list.end(); ++i) {
        i->hw();
    }
    printf("iteration backward\n");
    //TODO: need implement rbegin and rend
    for (auto i = --list.end(); i != --list.begin(); --i) {
        i->hw();
    }
    printf("RAII is your friend!\n");
}

void nullList() {
    auto list = XorLinkedList<Shouter>();
    
    //will it crash?
    for (auto i = list.begin(); i != list.end(); ++i) {
        i->hw();
    }
    
    for (int i = 0 ; i < 3; ++i) {
        list.append(Shouter(i));
    }
    
    //did append good?
    for (auto i = list.begin(); i != list.end(); ++i) {
        i->hw();
    }
}
void STLCompliant() {
    auto list = XorLinkedList<int>({1,4,3,5});
    for (auto i = list.begin(); i != list.end(); ++i) {
        printf("%d\n",*i);
    }
    
    std::reverse(list.begin(), list.end());
    printf("-----reversed\n");
    for (auto i = list.begin(); i != list.end(); ++i) {
        printf("%d\n",*i);
    }
    
    std::rotate(list.begin(),++++list.begin(), list.end());
    printf("-----rotated\n");
    for (auto i = list.begin(); i != list.end(); ++i) {
        printf("%d\n",*i);
    }
    
    auto findRes = std::find(list.begin(), list.end(), 5);
    if (findRes) {
        printf("finding 5, found %d\n",*findRes);
    }
    
    printf("range_based for\n");
    for (auto& i: list) {
        printf("%d\n",i);
    }

}
int main() {
    printf("-----\n");
    copyFromVector();
    printf("-----\n");
    copyFromArray();
    printf("-----\n");
    copyFromInitializerList();
    printf("-----\n");
    moveFromArray();
    printf("-----\n");
    moveSelf();
    printf("-----\n");
    forwardAndBackward();
    printf("-----\n");
    nullList();
    printf("-----\n");
    STLCompliant();
    
}
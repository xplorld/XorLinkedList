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
#include <string>
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
void modify() {
    auto list = XorLinkedList<int>({1,2,3,4,5,6,7,8,666,9,666,666,666,666,0});
    printf("before erase:\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    auto beg = list.begin();
    std::advance(beg, 3);
    auto end = list.begin();
    std::advance(end, 7);
    list.erase(beg,end);
    printf("after erasing index 3 to 6:\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    list.erase(++list.begin());
    printf("after erasing index 1:\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    auto ins = list.begin();
    std::advance(ins, 2);
    list.insert(ins, 42);
    printf("after inserting 42 at index 2:\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    list.remove(666);
    printf("after removing 666:\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    list.remove_if([](const int& n){ return (n & 0x01) == 0; });
    printf("after removing even integers:\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    list.pop_back();
    printf("after pop_back\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    list.pop_front();
    printf("after pop_front\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    list.clear();
    printf("after clear:\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
}

void operations() {
    auto list = XorLinkedList<int>({1,2,3,4,5,6,7,8,9,0});
    printf("list before reverse:\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    list.reverse();
    printf("after reverse:\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    auto list2 = XorLinkedList<int>({42,43,44});
    printf("list2: \n");
    for (auto &i:list2) printf("%d ",i);
    printf("\n");
    
    auto iter = list2.cbegin();
    std::advance(iter, 2); // point to 44
    list2.splice(iter, list);
    printf("list2.splice(<iter to 44>, list): \n");
    for (auto &i:list2) printf("%d ",i);
    printf("\n");
    printf("now list:(should be empty)\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    printf("size: list %lu, list2 %lu\n",list.size(),list2.size());
    
    list.splice(list.begin(), list2, ++list2.begin());
    printf("list.splice(list.begin(), list2, ++list2.begin());\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    printf("now list2:(should be one less)\n");
    for (auto &i:list2) printf("%d ",i);
    printf("\n");
    printf("size: list %lu, list2 %lu\n",list.size(),list2.size());
    
    list.splice(list.begin(), list2, ++list2.begin(),--list2.end());
    printf("list.splice(list.begin(), list2, ++list2.begin(),--list2.end());\n");
    for (auto &i:list) printf("%d ",i);
    printf("\n");
    
    printf("now list2:(should remain 2 elements)\n");
    for (auto &i:list2) printf("%d ",i);
    printf("\n");
    printf("size: list %lu, list2 %lu\n",list.size(),list2.size());
    
    auto list4 = XorLinkedList<int>({1, 2, 4, 4, 8, 9, 11, 22, 1, 2});
    printf("list4:\n");
    for (auto &i:list4) printf("%d ",i);
    printf("\n");
    list4.unique([](const int&a,const int&b){return b == 2*a;});
    printf("list4.unique([](const int&a,const int&b){return b == 2*a;});\n");
    for (auto &i:list4) printf("%d ",i);
    printf("\n");
    list4.unique();
    printf("list4.unique();\n");
    for (auto &i:list4) printf("%d ",i);
    printf("\n");
    
}

void compare() {
    auto list1 = XorLinkedList<int>({1,2,3});
    auto list2 = XorLinkedList<int>({1,2,3});
    auto list3 = XorLinkedList<int>({2,3,4});
    
    printf("list1:\n");
    for (auto &i:list1) printf("%d ",i);
    printf("\n");
    
    printf("list2:\n");
    for (auto &i:list2) printf("%d ",i);
    printf("\n");
    
    printf("list3:\n");
    for (auto &i:list3) printf("%d ",i);
    printf("\n");
    
    printf("list1 == list1: %s\n",(list1 == list1) ? "true" : "false");
    printf("list1 == list2: %s\n",(list1 == list2) ? "true" : "false");
    printf("list1 <= list2: %s\n",(list1 <= list2) ? "true" : "false");
    printf("list1 == list3: %s\n",(list1 == list3) ? "true" : "false");
    printf("list1 != list3: %s\n",(list1 != list3) ? "true" : "false");
    printf("list1 >  list3: %s\n",(list1 >  list3) ? "true" : "false");
    printf("list1 <  list3: %s\n",(list1 <  list3) ? "true" : "false");
    printf("list1 >= list3: %s\n",(list1 >= list3) ? "true" : "false");
    printf("list1 <= list3: %s\n",(list1 <= list3) ? "true" : "false");
}
void assignment() {
    XorLinkedList<Shouter> list_lvalue({Shouter(1),Shouter(2),Shouter(3)});
    printf("list inited\n");
    XorLinkedList<Shouter> list;
    
    //assignment by lvalue ref
    list = list_lvalue;
    printf("list assigned\n");
    
    //assignment by init list
    list = {Shouter(4),Shouter(5),Shouter(6)};
    printf("list assigned\n");
    
    //assignment by rvalue ref
    list = XorLinkedList<Shouter>({Shouter(7),Shouter(8),Shouter(9)});
    printf("list assigned\n");
    
    //assignment by vector
    auto vec = std::vector<Shouter>({Shouter(7),Shouter(8),Shouter(9)});
    printf("vec inited\n");
    list = vec;
    printf("list assigned\n");
    list = std::move(vec);
    printf("list moved\n");
    //    list.assign(5, Shouter(5));
    for (auto &i:list) i.hw();
    printf("\n");
    
}

void emplace() {
    XorLinkedList<Shouter> list;
    list.emplace_back(1);
    list.emplace_back(3);
    list.emplace_front(0);
    
    auto iter = list.begin();
    std::advance(iter, 2);
    list.emplace(iter, 2);
    for (auto &i:list) i.hw(); //0,1,2,3
}

void merge() {
    XorLinkedList<int> list1 {0,2,6,8};
    XorLinkedList<int> list2 {1,3,4,5,7};
    list1.merge(list2);
    printf("\n");
    for (auto &i:list1) printf("%d ",i);
    printf("\n");
    for (auto &i:list2) printf("%d ",i);
}

void sort() {
    XorLinkedList<int> list {1,5,3,5,6,2,1,4};
    list.sort();
    printf("\n");
    for (auto &i:list) printf("%d ",i);
}

void resize() {
    XorLinkedList<Shouter> list {Shouter(7),Shouter(8),Shouter(9),Shouter(10),Shouter(11)};
    printf("\n");
    for (auto &i:list) i.hw();
    list.resize(3, Shouter(0));
    printf("\n");
    for (auto &i:list) i.hw();
    list.resize(10, Shouter(12));
    printf("\n");
    for (auto &i:list) i.hw();
    
}

int main() {
    initWithSingleElement();
    initWithContainers();
    initWithIterators();
    copyAndMove();
    appendAndJoin();
    iterators();
    modify();
    operations();
    compare();
    assignment();
    emplace();
    merge();
    sort();
    resize();
    
}
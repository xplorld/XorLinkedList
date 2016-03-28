//
//  XorLinkedList.h
//  FunnyCoding
//
//  Created by Xplorld on 2016/3/27.
//  Copyright © 2016年 xplorld. All rights reserved.
//

#ifndef XorLinkedList_hpp
#define XorLinkedList_hpp

#include <utility>
#include <iterator>
#include <cstdint>
#include <cassert>
#include <initializer_list>

template <typename T>
class XorLinkedList {
public:
    class Iterator;
private:
    class Node {
        friend XorLinkedList;
        friend Iterator;
        uintptr_t xorptr;
        T value;
        
        Node(T &&v);
        Node(const T &v);
        
        template < typename inputIterator>
        Node(const inputIterator v);
        
        Node* join(T &&v);
        Node* join(const T& v);

        template < typename inputIterator>
        Node* join(const inputIterator v);
        
        template < typename inputIterator>
        static std::pair<Node *, Node *> newList(inputIterator begin,inputIterator end);

        template < typename inputIterator>
        static std::pair<Node *, Node *> newList(std::move_iterator<inputIterator> begin,std::move_iterator<inputIterator> end);

        //do not declare as method, since `curr` may be nullptr
        static Node *another(Node* curr, Node* one); //->another
        
        ~Node() ;
    };
    
    //Wrapper so that List can put on the stack
    //and befriend RAII
    Node *head;
    //last `Node` that is not nullptr
    Node *tail;
public:
    class Iterator {
        Node *prev;
        Node *curr;
    public:
        Iterator(Node *prev,Node *curr);
        T &value()      ;
        T &operator*()  ;
        T *operator->() ;
        Iterator &operator++() ;      //++i
        Iterator operator++ ( int );  //i++
        Iterator &operator--() ;      //--i
        Iterator operator-- ( int );  //i--
        
        friend bool operator!=(Iterator lhs, Iterator rhs) {return lhs.curr != rhs.curr;}
        explicit operator bool() const ;
    };
    XorLinkedList();
    XorLinkedList(const T* v) ;
    XorLinkedList(const T& v) ;
    XorLinkedList(T&& v) ;
    XorLinkedList(std::initializer_list<T> array);
    XorLinkedList(T *array,size_t count);
    template < typename inputIterator>
    XorLinkedList(inputIterator begin,inputIterator end);
    //move constructor
    XorLinkedList(XorLinkedList&& another);
    
    void append(const T &v);
    
    ~XorLinkedList() ;
    XorLinkedList::Iterator begin();
    XorLinkedList::Iterator end();
};

//template <typename T>
//bool operator!=(typename XorLinkedList<T>::Iterator &one, typename XorLinkedList<T>::Iterator &another);

#include <algorithm>
#define PtrToInt(p) reinterpret_cast<uintptr_t>((void *)(p))
#define IntToPtr(i) reinterpret_cast<Node *>((i))

//template <typename T>
//XorLinkedList<T>::Node::Node(const T *v) : xorptr(0),value(*v){}
template <typename T>
template <typename inputIterator>
XorLinkedList<T>::Node::Node(const inputIterator v) : xorptr(0),value(*v){} //copy, not move


template <typename T>
XorLinkedList<T>::Node::Node(const T &v) : xorptr(0),value(v){} //copy

template <typename T>
XorLinkedList<T>::Node::Node(T &&v) : xorptr(0),value(std::move(v)){}

template <typename T>
template <typename inputIterator>
typename std::pair< typename XorLinkedList<T>::Node *, typename XorLinkedList<T>::Node *>
XorLinkedList<T>::Node::newList(inputIterator begin,inputIterator end) {
    Node *list = nullptr,*tail = nullptr;
    try {
        assert(begin != end && "input feed is empty");
        list = new Node(&*begin); //cast iter to pointer
        ++begin;
        tail = list;
        while (begin != end){
            
            tail = tail->join(&*begin);
            ++begin;
        }
    } catch(...) {
        delete list;
        throw;
    }

    return std::make_pair(list, tail);
}

template <typename T>
template < typename inputIterator>
typename std::pair< typename XorLinkedList<T>::Node *, typename XorLinkedList<T>::Node *>
XorLinkedList<T>::Node::newList(std::move_iterator<inputIterator> begin,std::move_iterator<inputIterator> end) {
    Node *list = nullptr,*tail = nullptr;
    try {
        assert(begin != end && "input feed is empty");
         list = new Node(std::move(begin));
        ++begin;
         tail = list;
        while (begin != end){
            
            tail = tail->join(std::move(begin));
            ++begin;
        }
    } catch(...) {
        delete list;
        throw;
    }

    return std::make_pair(list, tail);
}

template <typename T>
typename XorLinkedList<T>::Node * XorLinkedList<T>::Node::another(Node*curr, Node* one) {
    if (curr == nullptr) {
        return nullptr;
    }
    return IntToPtr(PtrToInt(one) ^ curr->xorptr);
}

template <typename T>
XorLinkedList<T>::Node::~Node() {
    printf("deallocating, I am %p\n",&value);
    
    
    auto B = another(this,nullptr);
    if (B) {
        //A -> B -> C
        //I am head A!
        auto C = another(B,this);
        B->xorptr = PtrToInt(C);
        delete B;
    }
    //        else {
    //I am tail
    //do nothing
    //        }
}

template <typename T>
typename XorLinkedList<T>::Node * XorLinkedList<T>::Node::join(const T &v) {
    //A->B, C.
    //B.p = A, C.p = 0
    
    Node *endNode = this;
    Node *newNode = new Node(std::move(v));
    endNode->xorptr = endNode->xorptr ^ PtrToInt((void *)newNode);
    newNode->xorptr = PtrToInt((void *)endNode);
    //A->B->C
    //B.p = A^C, C.p = B^0 = B
    return newNode;
}

template <typename T>
template < typename inputIterator>
typename XorLinkedList<T>::Node * XorLinkedList<T>::Node::join(const inputIterator v) {
    //A->B, C.
    //B.p = A, C.p = 0
    
    Node *endNode = this;
    Node *newNode = new Node(v);
    endNode->xorptr = endNode->xorptr ^ PtrToInt((void *)newNode);
    newNode->xorptr = PtrToInt((void *)endNode);
    //A->B->C
    //B.p = A^C, C.p = B^0 = B
    return newNode;
}


template <typename T>
typename XorLinkedList<T>::Node * XorLinkedList<T>::Node::join(T&& v) {
    //A->B, C.
    //B.p = A, C.p = 0
    
    Node *endNode = this;
    Node *newNode = new Node(std::move(v));
    endNode->xorptr = endNode->xorptr ^ PtrToInt((void *)newNode);
    newNode->xorptr = PtrToInt((void *)endNode);
    //A->B->C
    //B.p = A^C, C.p = B^0 = B
    return newNode;
}

#undef PtrToInt
#undef IntToPtr

template <typename T>
XorLinkedList<T>::Iterator::Iterator(Node *prev,Node *curr) : prev(prev),curr(curr) {}

template <typename T>
T & XorLinkedList<T>::Iterator::value()      { return curr->value; }


template <typename T>
T & XorLinkedList<T>::Iterator::operator*()  { return curr->value; }

template <typename T>
T * XorLinkedList<T>::Iterator::operator->() { return &curr->value; }

template <typename T>
typename XorLinkedList<T>::Iterator & XorLinkedList<T>::Iterator::operator++() {
    Node *nextNode = Node::another(curr,prev);
    prev = curr;
    curr = nextNode;
    return *this;
}

template <typename T>
typename XorLinkedList<T>::Iterator  XorLinkedList<T>::Iterator::operator++ ( int )
{
    Iterator nowIter = Iterator(curr);
    nowIter.prev = prev;
    
    ++(*this);
    return nowIter;
}

template <typename T>
typename XorLinkedList<T>::Iterator & XorLinkedList<T>::Iterator::operator--() {
    Node *prevNode = Node::another(prev,curr);
    curr = prev;
    prev = prevNode;
    return *this;
}

template <typename T>
typename XorLinkedList<T>::Iterator  XorLinkedList<T>::Iterator::operator-- ( int )
{
    Iterator nowIter = Iterator(curr);
    nowIter.prev = prev;
    
    --(*this);
    return nowIter;
}

template <typename T>
XorLinkedList<T>::Iterator::operator bool() const {   return curr;    }

template <typename T>
XorLinkedList<T>::XorLinkedList() : head(nullptr), tail(nullptr) {}


template <typename T>
XorLinkedList<T>::XorLinkedList(const T& v) : head(new Node(v)), tail(head) {}

template <typename T>
XorLinkedList<T>::XorLinkedList(T &&v) : head(new Node(std::move(v))), tail(head) {}

template <typename T>
XorLinkedList<T>::XorLinkedList(const T *v) : head(new Node(v)), tail(head) {}


template <typename T>
XorLinkedList<T>::XorLinkedList(std::initializer_list<T> array) {
    auto pair = Node::newList(array.begin(),array.end());
    head = pair.first;
    tail = pair.second;
}

template <typename T>
XorLinkedList<T>::XorLinkedList(T *array,size_t count) {
    auto pair = Node::newList(array,array+count);
    head = pair.first;
    tail = pair.second;
}

template <typename T>
template <typename inputIterator>
XorLinkedList<T>::XorLinkedList(inputIterator begin,inputIterator end) {
    auto pair = Node::newList(begin,end);
    head = pair.first;
    tail = pair.second;
}

template <typename T>
XorLinkedList<T>::~XorLinkedList() {
    printf("deleting linked list\n");
    if (head) delete head;
    //no need to delete tail, it will be deleted by Node deconstructer recursively
}

template <typename T>
XorLinkedList<T>::XorLinkedList(XorLinkedList&& another) {
    head = another.head;
    tail = another.tail;
    another.head = nullptr;
    another.tail = nullptr;
}

template <typename T>
void XorLinkedList<T>::append(const T &v){
        if (tail) {
            tail = tail->join(&v);
        } else {
            //empty linked list
            head = new Node(&v);
            tail = head;
        }
}

template <typename T>
typename XorLinkedList<T>::Iterator XorLinkedList<T>::begin() {
    if (head) return Iterator(nullptr,head);
    return Iterator(nullptr,nullptr);
}

template <typename T>
typename XorLinkedList<T>::Iterator XorLinkedList<T>::end() {
    if (tail) return Iterator(tail,nullptr);
    return Iterator(nullptr,nullptr);
}

#endif /* XorLinkedList_hpp */

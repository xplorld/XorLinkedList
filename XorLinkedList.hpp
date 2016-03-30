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
#include "is_container_SFINAE.h"

template <typename T>
class XorLinkedList {
private:
    template <bool is_const>
    class _iterator;
   
    template <bool is_const>
    class _reverse_iterator;
public:
    
    typedef _iterator<false> iterator;
    typedef _iterator<true> const_iterator;
    
    
    typedef _reverse_iterator<false> reverse_iterator;
    typedef _reverse_iterator<true> const_reverse_iterator;
private:
    
    class node {
        friend XorLinkedList;
        friend iterator;
        
        uintptr_t xorptr;
        T value;
        
        node(T &&v)
        noexcept(std::is_nothrow_move_constructible<T>::value)
        : xorptr(0),value(std::move(v)){};
        
        node(const T &v)
        noexcept(std::is_nothrow_constructible<T, const T &>::value)
        : xorptr(0),value(v){}; //copy
        
        template < typename input_iterator>
        node(const input_iterator v)
        noexcept(std::is_nothrow_constructible<T, const input_iterator>::value)
        : xorptr(0),value(*v){}; //copy, not move
        
        node* join(T &&v);
        node* join(const T& v);
        
        template < typename input_iterator>
        node* join(const input_iterator v);
        
        template < typename input_iterator>
        static std::pair<node *, node *> newList(input_iterator begin,input_iterator end);
        
        template < typename input_iterator>
        static std::pair<node *, node *> newList(std::move_iterator<input_iterator> begin,std::move_iterator<input_iterator> end);
        
        //do not declare as method, since `curr` may be nullptr
        static node *another(node* curr, node* one) noexcept; //->another
        
        ~node() ;
    };
    
    //Wrapper so that List can put on the stack
    //and befriend RAII
    node *head;
    //last `Node` that is not nullptr
    node *tail;
    template <bool is_const = false>
    class _iterator
    : public std::iterator <std::bidirectional_iterator_tag,T>
    {
        typedef typename std::conditional<is_const, const T, T>::type ReturnType;
        friend _iterator<true>;
        friend _iterator<false>;
        
        //node is still non-const
        node *prev = nullptr;
        node *curr = nullptr;
    public:
        _iterator(node *prev, node *curr) : prev(prev),curr(curr) {};
        //only allow non-const to const
        _iterator(const _iterator<false> &copyFrom)  : prev(copyFrom.prev),curr(copyFrom.curr) {};
        
        
        ReturnType &operator*()
        {assert(curr != nullptr && "dereference iterator!"); return  curr->value; };
        ReturnType *operator->() {return &*(*this);};
        
        
        _iterator &operator++() noexcept ;      //++i
        _iterator operator++ ( int ) noexcept;  //i++
        _iterator &operator--() noexcept ;      //--i
        _iterator operator-- ( int ) noexcept;  //i--
        
        void swap(_iterator & other) noexcept;
        
        explicit operator bool() const noexcept {return prev == nullptr && curr == nullptr;}
        bool operator == (const _iterator& rhs) const {return curr == rhs.curr && prev == rhs.prev;};
        bool operator != (const _iterator& rhs) const {return curr != rhs.curr || prev != rhs.prev;};
    };
    
    
    
    template <bool is_const = false>
    class _reverse_iterator
    : public std::iterator <std::bidirectional_iterator_tag,T>
    {
        
        typedef typename std::conditional<is_const, const T, T>::type ReturnType;
        friend _reverse_iterator<true>;
        friend _reverse_iterator<false>;
        
        node *curr = nullptr;
        node *next = nullptr;
    public:
        _reverse_iterator(node *curr,node *next) : curr(curr), next(next) {};
        
        
        _reverse_iterator(const _reverse_iterator<false> &copyFrom)  : curr(copyFrom.curr), next(copyFrom.next) {};
        
        
        ReturnType &operator*() {assert(curr != nullptr && "dereference iterator!"); return  curr->value; };
        ReturnType *operator->() {return &*(*this);};
        
        _reverse_iterator &operator++() noexcept ;      //++i
        _reverse_iterator operator++ ( int ) noexcept;  //i++
        _reverse_iterator &operator--() noexcept ;      //--i
        _reverse_iterator operator-- ( int ) noexcept;  //i--
        
        void swap(_reverse_iterator & other) noexcept;
        
        explicit operator bool() const noexcept {return next == nullptr && curr == nullptr;}
        bool operator == (const _reverse_iterator& rhs) const {return curr == rhs.curr && next == rhs.next;};
        bool operator != (const _reverse_iterator& rhs) const {return curr != rhs.curr || next != rhs.next;};
    };
public:

    XorLinkedList() : head(nullptr), tail(nullptr) {};
    XorLinkedList(const T* v) : head(new node(v)), tail(head) {};
    XorLinkedList(const T& v) : head(new node(v)), tail(head) {};
    XorLinkedList(T&& v) : head(new node(std::move(v))), tail(head) {};
    XorLinkedList(std::initializer_list<T> array);
    
    template <typename Container, class = typename std::enable_if<is_container<Container>::value>::type>
    XorLinkedList(Container& container);
    
    XorLinkedList(T *array,size_t count);
    template < typename input_iterator>
    XorLinkedList(input_iterator begin,input_iterator end);
    //move constructor
    XorLinkedList(XorLinkedList&& another);
    
    void append(const T &v);
    
    //no need to delete tail, it will be deleted by Node deconstructer recursively
    ~XorLinkedList() {if (head) delete head;} ;
    
    iterator begin();
    iterator end();
    const_iterator cbegin() {return const_iterator(begin());};
    const_iterator cend() {return const_iterator(end());};
    reverse_iterator rbegin();
    reverse_iterator rend();
    const_reverse_iterator crbegin() {return const_reverse_iterator(rbegin());};
    const_reverse_iterator crend() {return const_reverse_iterator(rend());};
};

//template <typename T>
//bool operator!=(typename XorLinkedList<T>::iterator &one, typename XorLinkedList<T>::iterator &another);

#include <algorithm>
#define PtrToInt(p) reinterpret_cast<uintptr_t>((void *)(p))
#define IntToPtr(i) reinterpret_cast<node *>((i))

template <typename T>
template <typename input_iterator>
typename std::pair< typename XorLinkedList<T>::node *, typename XorLinkedList<T>::node *>
XorLinkedList<T>::node::newList(input_iterator begin,input_iterator end) {
    node *list = nullptr,*tail = nullptr;
    try {
        assert(begin != end && "input feed is empty");
        list = new node(&*begin); //cast iter to pointer
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
template < typename input_iterator>
typename std::pair< typename XorLinkedList<T>::node *, typename XorLinkedList<T>::node *>
XorLinkedList<T>::node::newList(std::move_iterator<input_iterator> begin,std::move_iterator<input_iterator> end) {
    node *list = nullptr,*tail = nullptr;
    try {
        assert(begin != end && "input feed is empty");
        list = new node(std::move(begin));
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
typename XorLinkedList<T>::node * XorLinkedList<T>::node::another(node*curr, node* one) noexcept {
    if (curr == nullptr) {
        return nullptr;
    }
    return IntToPtr(PtrToInt(one) ^ curr->xorptr);
}

template <typename T>
XorLinkedList<T>::node::~node() {
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
typename XorLinkedList<T>::node * XorLinkedList<T>::node::join(const T &v) {
    //A->B, C.
    //B.p = A, C.p = 0
    
    node *endNode = this;
    node *newNode = new node(std::move(v));
    endNode->xorptr = endNode->xorptr ^ PtrToInt((void *)newNode);
    newNode->xorptr = PtrToInt((void *)endNode);
    //A->B->C
    //B.p = A^C, C.p = B^0 = B
    return newNode;
}

template <typename T>
template < typename input_iterator>
typename XorLinkedList<T>::node * XorLinkedList<T>::node::join(const input_iterator v) {
    //A->B, C.
    //B.p = A, C.p = 0
    
    node *endNode = this;
    node *newNode = new node(v);
    endNode->xorptr = endNode->xorptr ^ PtrToInt((void *)newNode);
    newNode->xorptr = PtrToInt((void *)endNode);
    //A->B->C
    //B.p = A^C, C.p = B^0 = B
    return newNode;
}


template <typename T>
typename XorLinkedList<T>::node * XorLinkedList<T>::node::join(T&& v) {
    //A->B, C.
    //B.p = A, C.p = 0
    
    node *endNode = this;
    node *newNode = new node(std::move(v));
    endNode->xorptr = endNode->xorptr ^ PtrToInt((void *)newNode);
    newNode->xorptr = PtrToInt((void *)endNode);
    //A->B->C
    //B.p = A^C, C.p = B^0 = B
    return newNode;
}

#undef PtrToInt
#undef IntToPtr


template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template _iterator<is_const> & XorLinkedList<T>::_iterator<is_const>::operator++() noexcept{
    node *nextNode = node::another(curr,prev);
    prev = curr;
    curr = nextNode;
    return *this;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template _iterator<is_const> XorLinkedList<T>::_iterator<is_const>::operator++( int ) noexcept{
    _iterator nowIter = _iterator(prev,curr);
    ++(*this);
    return nowIter;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template _iterator<is_const> & XorLinkedList<T>::_iterator<is_const>::operator--() noexcept {
    node *prevNode = node::another(prev,curr);
    curr = prev;
    prev = prevNode;
    return *this;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template _iterator<is_const> XorLinkedList<T>::_iterator<is_const>::operator-- ( int ) noexcept
{
    _iterator nowIter = _iterator(prev,curr);
    --(*this);
    return nowIter;
}


template <typename T>
template <bool is_const>
void  XorLinkedList<T>::_iterator<is_const>::swap(XorLinkedList<T>::_iterator<is_const> &other) noexcept {
    using std::swap;
    swap(prev, other.prev);
    swap(curr, other.curr);
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template _reverse_iterator<is_const> & XorLinkedList<T>::_reverse_iterator<is_const>::operator++() noexcept{
    node *prevNode = node::another(curr,next);
    next = curr;
    curr = prevNode;
    return *this;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template _reverse_iterator<is_const>  XorLinkedList<T>::_reverse_iterator<is_const>::operator++ ( int ) noexcept
{
    _reverse_iterator<is_const> nowIter = _reverse_iterator<is_const>(curr,next);
    ++(*this);
    return nowIter;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template _reverse_iterator<is_const> & XorLinkedList<T>::_reverse_iterator<is_const>::operator--() noexcept {
    node *nextNode = node::another(next,curr);
    curr = next;
    next = nextNode;
    return *this;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template _reverse_iterator<is_const>  XorLinkedList<T>::_reverse_iterator<is_const>::operator-- ( int ) noexcept
{
    _reverse_iterator<is_const> nowIter = _reverse_iterator<is_const>(curr,next);
    --(*this);
    return nowIter;
}

template <typename T>
template <bool is_const>
void  XorLinkedList<T>::template _reverse_iterator<is_const>::swap(XorLinkedList::_reverse_iterator<is_const> &other) noexcept {
    using std::swap;
    swap(next, other.next);
    swap(curr, other.curr);
}


template <typename T>
XorLinkedList<T>::XorLinkedList(std::initializer_list<T> array) {
    auto pair = node::newList(array.begin(),array.end());
    head = pair.first;
    tail = pair.second;
}

template <typename T>
template <typename Container,class>
XorLinkedList<T>::XorLinkedList(Container& container) {
    auto pair = node::newList(container.begin(),container.end());
    head = pair.first;
    tail = pair.second;
}


template <typename T>
XorLinkedList<T>::XorLinkedList(T *array,size_t count) {
    auto pair = node::newList(array,array+count);
    head = pair.first;
    tail = pair.second;
}

template <typename T>
template <typename input_iterator>
XorLinkedList<T>::XorLinkedList(input_iterator begin,input_iterator end) {
    auto pair = node::newList(begin,end);
    head = pair.first;
    tail = pair.second;
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
        head = new node(&v);
        tail = head;
    }
}

template <typename T>
typename XorLinkedList<T>::iterator XorLinkedList<T>::begin() {
    if (head) return iterator(nullptr, head);
    return iterator(nullptr,nullptr);
}

template <typename T>
typename XorLinkedList<T>::iterator XorLinkedList<T>::end() {
    if (tail) return iterator(tail,nullptr);
    return iterator(nullptr,nullptr);
}

template <typename T>
typename XorLinkedList<T>::reverse_iterator XorLinkedList<T>::rbegin() {
    if (tail) return reverse_iterator(tail, nullptr);
    return reverse_iterator(nullptr,nullptr);
}


template <typename T>
typename XorLinkedList<T>::reverse_iterator XorLinkedList<T>::rend() {
    if (tail) return reverse_iterator(nullptr,head);
    return reverse_iterator(nullptr,nullptr);
}

#endif /* XorLinkedList_hpp */

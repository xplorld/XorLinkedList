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
#include <tuple>
#include <type_traits>
#include "is_container_SFINAE.h"

template <typename T>
class XorLinkedList {
private:
    template <bool is_const>
    class iterator_;
   
    template <bool is_const>
    class reverse_iterator_;
public:
    
    typedef iterator_<false> iterator;
    typedef iterator_<true> const_iterator;
    
    
    typedef reverse_iterator_<false> reverse_iterator;
    typedef reverse_iterator_<true> const_reverse_iterator;
private:
    
    class node {
        friend XorLinkedList;
        friend iterator;
        
        uintptr_t xorptr;
        T value;
        
        node(T &&v)
        noexcept(std::is_nothrow_move_constructible<T>::value)
        : xorptr(0),value(std::move(v)){}
        
        node(const T &v)
        noexcept(std::is_nothrow_constructible<T, const T &>::value)
        : xorptr(0),value(v){} //copy
        
        template < typename input_iterator>
        node(const input_iterator v)
        noexcept(std::is_nothrow_constructible<T, const input_iterator>::value)
        : xorptr(0),value(*v){} //copy, not move
        
        node* join(T &&v);
        node* join(const T& v);
        
        template < typename input_iterator>
        node* join(const input_iterator v);
        
        template < typename input_iterator>
        static std::tuple<node *, node *,size_t> newList(input_iterator begin,input_iterator end);
        
        template < typename input_iterator>
        static std::tuple<node *, node *,size_t> newList(std::move_iterator<input_iterator> begin,std::move_iterator<input_iterator> end);
        
        //do not declare as method, since `curr` may be nullptr
        static node *another(node* curr, node* one) noexcept; //->another
        
        ~node() ;
    };
    
    //Wrapper so that List can put on the stack
    //and befriend RAII
    node *head = nullptr;
    //last `Node` that is not nullptr
    node *tail = nullptr;
    size_t size_ = 0;
    
    template <bool is_const = false>
    class iterator_
    : public std::iterator <std::bidirectional_iterator_tag,typename std::conditional<is_const, const T, T>::type>
    {
        typedef typename std::conditional<is_const, const T, T>::type ReturnType;
        friend iterator_<true>;
        friend iterator_<false>;
        //node is still non-const
        node *prev = nullptr;
        node *curr = nullptr;
    public:
        iterator_(node *prev, node *curr) : prev(prev),curr(curr) {}
        //only allow non-const to const
        iterator_(const iterator_<false> &copyFrom)  : prev(copyFrom.prev),curr(copyFrom.curr) {}
        
        
        ReturnType &operator*()
        {assert(curr != nullptr && "dereference iterator!"); return  curr->value; }
        ReturnType *operator->() {return &*(*this);}
        
        
        iterator_ &operator++() noexcept ;      //++i
        iterator_ operator++ ( int ) noexcept;  //i++
        iterator_ &operator--() noexcept ;      //--i
        iterator_ operator-- ( int ) noexcept;  //i--
        
        void swap(iterator_ & other) noexcept;
        
        explicit operator bool() const noexcept {return prev == nullptr && curr == nullptr;}
        bool operator == (const iterator_& rhs) const {return curr == rhs.curr && prev == rhs.prev;}
        bool operator != (const iterator_& rhs) const {return curr != rhs.curr || prev != rhs.prev;}
    };
    
    
    template <bool is_const = false>
    class reverse_iterator_
    : public std::iterator <std::bidirectional_iterator_tag,typename std::conditional<is_const, const T, T>::type>
    {
        
        typedef typename std::conditional<is_const, const T, T>::type ReturnType;
        friend reverse_iterator_<true>;
        friend reverse_iterator_<false>;
        
        node *curr = nullptr;
        node *next = nullptr;
    public:
        reverse_iterator_(node *curr,node *next) : curr(curr), next(next) {}
        
        
        reverse_iterator_(const reverse_iterator_<false> &copyFrom)  : curr(copyFrom.curr), next(copyFrom.next) {}
        
        
        ReturnType &operator*() {assert(curr != nullptr && "dereference iterator!"); return  curr->value; }
        ReturnType *operator->() {return &*(*this);}
        
        reverse_iterator_ &operator++() noexcept ;      //++i
        reverse_iterator_ operator++ ( int ) noexcept;  //i++
        reverse_iterator_ &operator--() noexcept ;      //--i
        reverse_iterator_ operator-- ( int ) noexcept;  //i--
        
        void swap(reverse_iterator_ & other) noexcept;
        
        explicit operator bool() const noexcept {return next == nullptr && curr == nullptr;}
        bool operator == (const reverse_iterator_& rhs) const {return curr == rhs.curr && next == rhs.next;}
        bool operator != (const reverse_iterator_& rhs) const {return curr != rhs.curr || next != rhs.next;}
    };
public:

    XorLinkedList() : head(nullptr), tail(nullptr) {}
    XorLinkedList(const T* v) : head(new node(v)), tail(head), size_(1) {}
    XorLinkedList(const T& v) : head(new node(v)), tail(head), size_(1) {}
    XorLinkedList(T&& v) : head(new node(std::move(v))), tail(head), size_(1) {}
    
    XorLinkedList(std::initializer_list<T> array);
    template <typename Container, class = typename std::enable_if<is_container<Container>::value>::type>
    explicit XorLinkedList(Container container);
    
    XorLinkedList(T *array,size_t count);
    template < typename input_iterator>
    XorLinkedList(input_iterator begin,input_iterator end);
    
    //move constructor
    XorLinkedList(XorLinkedList&& another);
    //copy
    XorLinkedList(XorLinkedList const &another);
    
    void append(const T &v);
    
    //no need to delete tail, it will be deleted by Node deconstructer recursively
    ~XorLinkedList() {if (head) delete head;} ;
    
    size_t size() {return size_;}
    iterator begin();
    iterator end();
    const_iterator cbegin() const;
    const_iterator cend() const;
    reverse_iterator rbegin();
    reverse_iterator rend();
    const_reverse_iterator crbegin() const;
    const_reverse_iterator crend() const;
};

#include <algorithm>
#define PtrToInt(p) reinterpret_cast<uintptr_t>((void *)(p))
#define IntToPtr(i) reinterpret_cast<node *>((i))

template <typename T>
template <typename input_iterator>
typename std::tuple< typename XorLinkedList<T>::node *, typename XorLinkedList<T>::node *, size_t>
XorLinkedList<T>::node::newList(input_iterator begin,input_iterator end) {
    node *list = nullptr,*tail = nullptr;
    size_t size = 0;
    try {
        assert(begin != end && "input feed is empty");
        list = new node(&*begin); //cast iter to pointer
        ++begin;
        ++size;
        tail = list;
        while (begin != end){
            
            tail = tail->join(&*begin);
            ++begin;
            ++size;
        }
    } catch(...) {
        delete list;
        throw;
    }
    
    return std::make_tuple(list, tail,size);
}

template <typename T>
template < typename input_iterator>
typename std::tuple< typename XorLinkedList<T>::node *, typename XorLinkedList<T>::node *, size_t>
XorLinkedList<T>::node::newList(std::move_iterator<input_iterator> begin,std::move_iterator<input_iterator> end) {
    node *list = nullptr,*tail = nullptr;
    size_t size = 0;
    try {
        assert(begin != end && "input feed is empty");
        list = new node(std::move(begin));
        ++begin;
        ++size;
        tail = list;
        while (begin != end){
            tail = tail->join(std::move(begin));
            ++begin;
            ++size;
        }
    } catch(...) {
        delete list;
        throw;
    }
    
    return std::make_tuple(list, tail,size);
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
typename XorLinkedList<T>::template iterator_<is_const> & XorLinkedList<T>::iterator_<is_const>::operator++() noexcept{
    node *nextNode = node::another(curr,prev);
    prev = curr;
    curr = nextNode;
    return *this;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template iterator_<is_const> XorLinkedList<T>::iterator_<is_const>::operator++( int ) noexcept{
    iterator_ nowIter = iterator_(prev,curr);
    ++(*this);
    return nowIter;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template iterator_<is_const> & XorLinkedList<T>::iterator_<is_const>::operator--() noexcept {
    node *prevNode = node::another(prev,curr);
    curr = prev;
    prev = prevNode;
    return *this;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template iterator_<is_const> XorLinkedList<T>::iterator_<is_const>::operator-- ( int ) noexcept
{
    iterator_ nowIter = iterator_(prev,curr);
    --(*this);
    return nowIter;
}


template <typename T>
template <bool is_const>
void  XorLinkedList<T>::iterator_<is_const>::swap(XorLinkedList<T>::iterator_<is_const> &other) noexcept {
    using std::swap;
    swap(prev, other.prev);
    swap(curr, other.curr);
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template reverse_iterator_<is_const> & XorLinkedList<T>::reverse_iterator_<is_const>::operator++() noexcept{
    node *prevNode = node::another(curr,next);
    next = curr;
    curr = prevNode;
    return *this;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template reverse_iterator_<is_const>  XorLinkedList<T>::reverse_iterator_<is_const>::operator++ ( int ) noexcept
{
    reverse_iterator_<is_const> nowIter = reverse_iterator_<is_const>(curr,next);
    ++(*this);
    return nowIter;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template reverse_iterator_<is_const> & XorLinkedList<T>::reverse_iterator_<is_const>::operator--() noexcept {
    node *nextNode = node::another(next,curr);
    curr = next;
    next = nextNode;
    return *this;
}

template <typename T>
template <bool is_const>
typename XorLinkedList<T>::template reverse_iterator_<is_const>  XorLinkedList<T>::reverse_iterator_<is_const>::operator-- ( int ) noexcept
{
    reverse_iterator_<is_const> nowIter = reverse_iterator_<is_const>(curr,next);
    --(*this);
    return nowIter;
}

template <typename T>
template <bool is_const>
void  XorLinkedList<T>::reverse_iterator_<is_const>::swap(XorLinkedList<T>::reverse_iterator_<is_const> &other) noexcept {
    using std::swap;
    swap(next, other.next);
    swap(curr, other.curr);
}


template <typename T>
XorLinkedList<T>::XorLinkedList(std::initializer_list<T> array) {
    auto tuple = node::newList(array.begin(),array.end());
    head = std::get<0>(tuple);
    tail = std::get<1>(tuple);
    size_ = std::get<2>(tuple);
}

template <typename T>
template <typename Container,class>
XorLinkedList<T>::XorLinkedList(Container container) {
    auto tuple = node::newList(std::make_move_iterator(container.begin()), std::make_move_iterator(container.end()));
    head = std::get<0>(tuple);
    tail = std::get<1>(tuple);
    size_ = std::get<2>(tuple);
}

template <typename T>
XorLinkedList<T>::XorLinkedList(T *array,size_t count) {
    auto tuple = node::newList(array,array+count);
    head = std::get<0>(tuple);
    tail = std::get<1>(tuple);
    size_ = std::get<2>(tuple);
}

template <typename T>
template <typename input_iterator>
XorLinkedList<T>::XorLinkedList(input_iterator begin,input_iterator end) {
    auto tuple = node::newList(begin,end);
    head = std::get<0>(tuple);
    tail = std::get<1>(tuple);
    size_ = std::get<2>(tuple);
}

template <typename T>
XorLinkedList<T>::XorLinkedList(XorLinkedList&& another) {
    head = another.head;
    tail = another.tail;
    size_ = another.size_;
    another.head = nullptr;
    another.tail = nullptr;
    another.size_ = 0;
}

template <typename T>
XorLinkedList<T>::XorLinkedList(XorLinkedList const& another) {
    auto tuple = node::newList(another.cbegin(),another.cend());
    head = std::get<0>(tuple);
    tail = std::get<1>(tuple);
    size_ = std::get<2>(tuple);
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
    size_++;
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

template <typename T>
typename XorLinkedList<T>::const_iterator XorLinkedList<T>::cbegin() const {
    if (head) return const_iterator(nullptr, head);
    return const_iterator(nullptr,nullptr);
}

template <typename T>
typename XorLinkedList<T>::const_iterator XorLinkedList<T>::cend() const {
    if (tail) return const_iterator(tail,nullptr);
    return const_iterator(nullptr,nullptr);
}

template <typename T>
typename XorLinkedList<T>::const_reverse_iterator XorLinkedList<T>::crbegin() const {
    if (tail) return const_reverse_iterator(tail, nullptr);
    return const_reverse_iterator(nullptr,nullptr);
}


template <typename T>
typename XorLinkedList<T>::const_reverse_iterator XorLinkedList<T>::crend() const {
    if (tail) return const_reverse_iterator(nullptr,head);
    return const_reverse_iterator(nullptr,nullptr);
}

#endif /* XorLinkedList_hpp */

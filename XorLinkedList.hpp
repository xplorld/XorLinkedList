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
        friend iterator_<true>;
        friend iterator_<false>;
        //node is still non-const
        node *prev = nullptr;
        node *curr = nullptr;
    public:
        iterator_(node *prev, node *curr) : prev(prev),curr(curr) {}
        //only allow non-const to const
        iterator_(const iterator_<false> &copyFrom)  : prev(copyFrom.prev),curr(copyFrom.curr) {}
        
        typename iterator_::reference operator*()
        {assert(curr != nullptr && "dereference null iterator!"); return  curr->value; }
        typename iterator_::pointer operator->() {return std::addressof(operator*());}
        
        iterator_ &operator++() noexcept ;      //++i
        iterator_ operator++ ( int ) noexcept;  //i++
        iterator_ &operator--() noexcept ;      //--i
        iterator_ operator-- ( int ) noexcept;  //i--
        
        void swap(iterator_ & other) noexcept;
        
        explicit operator bool() const noexcept {return prev == nullptr && curr == nullptr;}
        bool operator == (const iterator_& rhs) const {return curr == rhs.curr && prev == rhs.prev;}
        bool operator != (const iterator_& rhs) const {return curr != rhs.curr || prev != rhs.prev;}
    };
    
    template<bool is_const = false>
        class reverse_iterator_ :
            public std::reverse_iterator<iterator_<is_const>>
    {
        public:
            friend reverse_iterator_<true>;
            reverse_iterator_(node *curr, node *next):
                std::reverse_iterator<iterator_<is_const>>
                (
                 iterator_<is_const>(curr, next)
                )
            {}  

            reverse_iterator_(const reverse_iterator_<false> &copyFrom):
                std::reverse_iterator<iterator_<is_const>>
                (
                 iterator_<is_const>(copyFrom.current)
                )
            {}
            explicit operator bool() const noexcept { return (bool)this->current; }
    };

public:
    XorLinkedList() : head(nullptr), tail(nullptr), size_(0) {}
    XorLinkedList(const T* v) : head(new node(v)), tail(head), size_(1) {}
    XorLinkedList(const T& v) : head(new node(v)), tail(head), size_(1) {}
    XorLinkedList(T&& v) : head(new node(std::move(v))), tail(head), size_(1) {}
    
    XorLinkedList(std::initializer_list<T> array) : XorLinkedList(array.begin(),array.end()) {}

    template <typename Container, class = typename std::enable_if<is_container<Container>::value>::type>
    explicit XorLinkedList(Container container)
    : XorLinkedList(std::make_move_iterator(container.begin()), std::make_move_iterator(container.end())) {}
    
    XorLinkedList(T *array,size_t count) : XorLinkedList(array,array+count) {}

    template < typename input_iterator>
    XorLinkedList(input_iterator begin,input_iterator end);
    
    //move constructor
    XorLinkedList(XorLinkedList&& another);
    //copy
    XorLinkedList(XorLinkedList const &another) : XorLinkedList(another.cbegin(),another.cend()) {}
    
    void append(const T &v);
    
    //no need to delete tail, it will be deleted by Node deconstructer recursively
    //head is nullptr if empty; OK to delete
    ~XorLinkedList()                        {delete head;}
    
    size_t size()                           {return size_;}
    iterator begin()                        { return iterator(nullptr, head);}
    iterator end()                          { return iterator(tail, nullptr);}
    const_iterator cbegin() const           { return const_iterator(nullptr, head);}
    const_iterator cend() const             { return const_iterator(tail, nullptr);}
    reverse_iterator rbegin()               { return reverse_iterator(tail, nullptr);}
    reverse_iterator rend()                 { return reverse_iterator(nullptr, head);}
    const_reverse_iterator crbegin() const  { return const_reverse_iterator(tail, nullptr);}
    const_reverse_iterator crend() const    { return const_reverse_iterator(nullptr, head);}
};

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
        list = new node(std::addressof(*begin)); //cast iter to pointer
        ++begin;
        ++size;
        tail = list;
        while (begin != end){
            
            tail = tail->join(std::addressof(*begin));
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
    operator++();
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
    operator--();
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
void XorLinkedList<T>::append(const T &v){
    if (tail) {
        tail = tail->join(std::addressof(v));
    } else {
        //empty linked list
        head = new node(std::addressof(v));
        tail = head;
    }
    size_++;
}

#endif /* XorLinkedList_hpp */

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
#include <limits>
#include <cstdint>
#include <cassert>
#include <tuple>
#include <type_traits>
#include "is_container_SFINAE.h"

#include <iostream>
//template <typename T, typename A = std::allocator<T> >
//class XorLinkedList {
//    typedef A allocator_type;
//    typedef typename A::value_type value_type;
//    typedef typename A::reference reference;
//    typedef typename A::const_reference const_reference;
//    typedef typename A::difference_type difference_type;
//    typedef typename A::size_type size_type;
//    
template <typename T>
class XorLinkedList {
//        typedef A allocator_type;
        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;
//        typedef typename A::difference_type difference_type;
        typedef size_t size_type;
    
    class node {
        friend XorLinkedList;
        
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
        
        node* join(node *newNode) noexcept;
        
        static std::tuple<node *, node *,size_type> newList(const T& value,size_type count);
        
        template < typename input_iterator>
        static std::tuple<node *, node *,size_type> newList(input_iterator begin,input_iterator end);
        
        template < typename input_iterator>
        static std::tuple<node *, node *,size_type> newList(std::move_iterator<input_iterator> begin,std::move_iterator<input_iterator> end);
        
        //do not declare as method, since `curr` may be nullptr
        static node *another(node* curr, node* one) noexcept; //->another
        void link(node *former,node *latter);
        
        //FIXME: current `~node` assumes `this` is the head
        ~node() noexcept(std::is_nothrow_destructible<T>::value) ;
    };
    
    //Wrapper so that List can put on the stack
    //and befriend RAII
    node *head = nullptr;
    //last `Node` that is not nullptr
    node *tail = nullptr;
    size_type size_ = 0;
    
    template <bool is_const = false>
    class iterator_
    : public std::iterator <std::bidirectional_iterator_tag,typename std::conditional<is_const, const T, T>::type>
    {
        friend iterator_<true>;
        friend XorLinkedList;
        
        //node is still non-const
        node *prev = nullptr;
        node *curr = nullptr;
    public:
        iterator_(node *prev, node *curr) noexcept : prev(prev),curr(curr) {}
        //only allow non-const to const
        iterator_(const iterator_<false> &copyFrom) noexcept  : prev(copyFrom.prev),curr(copyFrom.curr) {}
        
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
        reverse_iterator_(node *curr, node *next) noexcept :
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
    
    typedef iterator_<false> iterator;
    typedef iterator_<true> const_iterator;
    typedef reverse_iterator_<false> reverse_iterator;
    typedef reverse_iterator_<true> const_reverse_iterator;
    
    iterator insertNodes(const_iterator pos, node* newHead,node *newTail, size_type new_size);
    iterator insertNode(const_iterator pos, node* n) {return insertNodes(pos, n, n, 1);}
public:
    
#pragma mark - ctors
    XorLinkedList() : head(nullptr), tail(nullptr), size_(0) {}
    XorLinkedList(const T* v) : head(new node(v)), tail(head), size_(1) {}
    XorLinkedList(const T& v) : head(new node(v)), tail(head), size_(1) {}
    XorLinkedList(T&& v) : head(new node(std::move(v))), tail(head), size_(1) {}
    
    XorLinkedList(std::initializer_list<T> array) : XorLinkedList(array.begin(),array.end()) {}
    
    template <typename Container, class = typename std::enable_if<is_container<Container>::value>::type>
    explicit XorLinkedList(Container container)
    : XorLinkedList(std::make_move_iterator(container.begin()), std::make_move_iterator(container.end())) {}
    
    XorLinkedList(T *array,size_type count) : XorLinkedList(array,array+count) {}
    XorLinkedList(const T& value, size_type count);
    
    template < typename input_iterator>
    XorLinkedList(input_iterator begin,input_iterator end);
    
    //move constructor
    XorLinkedList(XorLinkedList&& another);
    //copy
    XorLinkedList(XorLinkedList const &another) : XorLinkedList(another.cbegin(),another.cend()) {}
    
#pragma mark - iterators
    
    iterator begin() noexcept                       { return iterator(nullptr, head);}
    iterator end() noexcept                         { return iterator(tail, nullptr);}
    iterator begin() const noexcept                 { return iterator(nullptr, head);}
    iterator end() const noexcept                   { return iterator(tail, nullptr);}
    const_iterator cbegin() const noexcept          { return const_iterator(nullptr, head);}
    const_iterator cend() const noexcept            { return const_iterator(tail, nullptr);}
    reverse_iterator rbegin() noexcept              { return reverse_iterator(tail, nullptr);}
    reverse_iterator rend() noexcept                { return reverse_iterator(nullptr, head);}
    reverse_iterator rbegin() const noexcept        { return reverse_iterator(tail, nullptr);}
    reverse_iterator rend() const noexcept          { return reverse_iterator(nullptr, head);}
    const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(tail, nullptr);}
    const_reverse_iterator crend() const noexcept   { return const_reverse_iterator(nullptr, head);}

#pragma mark - element access
    reference front()                       {return head->value;}
    const_reference front() const           {return head->value;}
    reference back()                        {return tail->value;}
    const_reference back() const            {return tail->value;}

#pragma mark - capacity
    size_type size() const  noexcept        {return size_;}
    bool empty() const  noexcept            {return size_ == 0;}
    size_type max_size() const noexcept     {return std::numeric_limits<size_type>::max();}
   
#pragma mark - modifiers
    void clear()  noexcept(std::is_nothrow_destructible<node>::value)   {erase(cbegin(), cend());}
    //emplace
    void erase(const_iterator from,const_iterator until);
    void erase(const_iterator pos);

    iterator insert(const_iterator pos, const T& value) {return insertNode(pos, new node(std::addressof(value)));}
    iterator insert(const_iterator pos, T&& value)      {return insertNode(pos, new node(std::move(value)));}
    iterator insert( const_iterator pos, size_type count, const T& value );
    template< class InputIt >
    iterator insert( const_iterator pos, InputIt first, InputIt last );
    iterator insert( const_iterator pos, std::initializer_list<T> ilist ) {return insert(pos, ilist.begin(), ilist.end());}
    
    void push_back(const T &v);
    void push_back(T &&v);
    //emplace_back
    void pop_back() noexcept(std::is_nothrow_destructible<node>::value){erase(--cend());}
    void push_front(const T &v);
    void push_front(T &&v);
    //emplace_front
    void pop_front() noexcept(std::is_nothrow_destructible<node>::value)    {erase(cbegin());}
    //resize
    void swap(XorLinkedList &other) {std::swap(head, other.head); std::swap(tail, other.tail); std::swap(size_, other.size_);}
    //assign
#pragma mark - operations
    //merge
    //splice
    //remove(_if)

    //NOTE: this `reverse` invalidates all iterators!
    void reverse() {std::swap(head, tail);}
    //unique
    //sort (what?)
    

    ~XorLinkedList()                        {clear();}
    
};
#pragma mark - non-member functions

//operator==,!=, <, <=, >, >=
//swap (should I pollute `std` namespace?)

#define PtrToInt(p) reinterpret_cast<uintptr_t>((void *)(p))
#define IntToPtr(i) reinterpret_cast<node *>((i))

template <typename T>
typename std::tuple< typename XorLinkedList<T>::node *, typename XorLinkedList<T>::node *, typename XorLinkedList<T>::size_type>
XorLinkedList<T>::node::newList(const T& value,size_type count) {
    if (!count) return std::make_tuple(nullptr,nullptr,0);
    node *list = new node(std::addressof(*value));
    node *tail = list;
    for (size_type i = 1; i < count; ++i) {
        tail = tail->join(new node(std::addressof(*value)));
    }
    return std::make_tuple(list,tail,count);
}

template <typename T>
template <typename input_iterator>
typename std::tuple< typename XorLinkedList<T>::node *, typename XorLinkedList<T>::node *, typename XorLinkedList<T>::size_type>
XorLinkedList<T>::node::newList(input_iterator begin,input_iterator end) {
    node *list = nullptr,*tail = nullptr;
    size_type size = 0;
    try {
        assert(begin != end && "input feed is empty");
        list = new node(std::addressof(*begin)); //cast iter to pointer
        ++begin;
        ++size;
        tail = list;
        while (begin != end){
            
            tail = tail->join(new node(std::addressof(*begin)));
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
typename std::tuple< typename XorLinkedList<T>::node *, typename XorLinkedList<T>::node *,typename XorLinkedList<T>::size_type>
XorLinkedList<T>::node::newList(std::move_iterator<input_iterator> begin,std::move_iterator<input_iterator> end) {
    node *list = nullptr,*tail = nullptr;
    size_type size = 0;
    try {
        assert(begin != end && "input feed is empty");
        list = new node(std::move(begin));
        ++begin;
        ++size;
        tail = list;
        while (begin != end){
            tail = tail->join(new node(std::move(begin)));
            ++begin;
            ++size;
        }
    } catch(...) {
        delete list;
        throw;
    }
    
    return std::make_tuple(list, tail, size);
}

template <typename T>
typename XorLinkedList<T>::node * XorLinkedList<T>::node::another(node*curr, node* one) noexcept {
    if (curr == nullptr) {
        return nullptr;
    }
    node* n = IntToPtr(PtrToInt(one) ^ curr->xorptr);
    if (n) return n;
    return nullptr; //instead of NULL
}

template <typename T>
XorLinkedList<T>::node::~node() noexcept(std::is_nothrow_destructible<T>::value) {
//    auto B = another(this,nullptr);
//    if (B) {
//        //A -> B -> C
//        //I am head A!
//        auto C = another(B,this);
//        B->xorptr = PtrToInt(C);
//        delete B;
//    }
    //        else {
    //I am tail
    //do nothing
    //        }
}

//NOTE: `this` must be head or tail
template <typename T>
typename XorLinkedList<T>::node * XorLinkedList<T>::node::join(node *newNode) noexcept {
    //A->B, C.
    //B.p = A, C.p = 0
    
    node *edgeNode = this;
    edgeNode->xorptr = edgeNode->xorptr ^ PtrToInt(newNode);
    newNode->xorptr = PtrToInt(edgeNode);
    //A->B->C
    //B.p = A^C, C.p = B^0 = B
    return newNode;
}

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
XorLinkedList<T>::XorLinkedList(const T& value, size_type count) {
    auto tuple = node::newList(value,count);
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
void XorLinkedList<T>::erase(const_iterator from,const_iterator until) {
    if (from == until) return;
    node *prev = from.prev; //last remaining node
    node *next = until.curr;
    
    if (!prev) head = next;
    if (!next) tail = prev;
    //bootstrap
    if (prev) {
        prev->xorptr ^= PtrToInt(from.curr);
        prev->xorptr ^= PtrToInt(next);
    }
    if (next) {
        next->xorptr ^= PtrToInt(until.prev);
        next->xorptr ^= PtrToInt(prev);
    }
    while (from.curr != until.curr) {
        node *deleteNode = from.curr;
        ++from;
        delete deleteNode;
        --size_;
    }
}
template <typename T>
void XorLinkedList<T>::erase(const_iterator pos) {
    const_iterator next = pos;
    ++next;
    erase(pos, next);
}

template <typename T>
typename XorLinkedList<T>::iterator XorLinkedList<T>::insertNodes(const_iterator pos, node* newHead,node *newTail, size_type new_size) {
    node *next = pos.curr;
    node *prev = (--pos).curr;
    newHead->xorptr ^= PtrToInt(prev);
    newTail->xorptr ^= PtrToInt(next);
    
    if (prev) {
        prev->xorptr ^= PtrToInt(next);
        prev->xorptr ^= PtrToInt(newHead);
    }
    if (next) {
        next->xorptr ^= PtrToInt(prev);
        next->xorptr ^= PtrToInt(newTail);
    }
    if (!prev) head = newHead;
    if (!next) tail = newTail;
    size_ += new_size;
    return iterator(prev,newHead);
}

template <typename T>
typename XorLinkedList<T>::iterator XorLinkedList<T>::insert( const_iterator pos, size_type count, const T& value ) {
    auto tuple = node::newList(value,count);
    return insertNodes(pos, std::get<0>(tuple), std::get<1>(tuple),std::get<2>(tuple));
}

template <typename T>
template< class InputIt >
typename XorLinkedList<T>::iterator XorLinkedList<T>::insert( const_iterator pos, InputIt first, InputIt last ) {
    auto tuple = node::newList(first,last);
    return insertNodes(pos, std::get<0>(tuple), std::get<1>(tuple),std::get<2>(tuple));
}


template <typename T>
void XorLinkedList<T>::push_back(const T &v){
    if (tail) {
        tail = tail->join(new node(std::addressof(v)));
    } else {
        //empty linked list
        head = new node(std::addressof(v));
        tail = head;
    }
    size_++;
}

template <typename T>
void XorLinkedList<T>::push_back(T &&v){
    if (tail) {
        tail = tail->join(new node(std::move(v)));
    } else {
        //empty linked list
        head = new node(new node(std::move(v)));
        tail = head;
    }
    size_++;
}
template <typename T>
void XorLinkedList<T>::push_front(const T &v){
    if (head) {
        head = head->join(new node(std::addressof(v)));
    } else {
        //empty linked list
        head = new node(std::addressof(v));
        tail = head;
    }
    size_++;
}

template <typename T>
void XorLinkedList<T>::push_front(T &&v){
    if (head) {
        head = head->join(new node(std::addressof(v)));
    } else {
        //empty linked list
        head = new node(new node(std::move(v)));
        tail = head;
    }
    size_++;
}
//
//template <typename T>
//void XorLinkedList<T>::pop_back() {
//    if (empty()) {
//        return;
//    }
//    node *oldTail = tail;
//    node *newTail = node::another(tail,nullptr);
//    delete oldTail;
//    tail = newTail;
//    size_--;
//}

#undef PtrToInt
#undef IntToPtr

#endif /* XorLinkedList_hpp */

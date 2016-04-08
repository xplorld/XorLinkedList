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
        
        template < typename input_iterator>
        static std::tuple<node *, node *,size_t> newList(input_iterator begin,input_iterator end);
        
        template < typename input_iterator>
        static std::tuple<node *, node *,size_t> newList(std::move_iterator<input_iterator> begin,std::move_iterator<input_iterator> end);
        
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
    size_t size_ = 0;
    
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
    
    XorLinkedList(T *array,size_t count) : XorLinkedList(array,array+count) {}
    
    template < typename input_iterator>
    XorLinkedList(input_iterator begin,input_iterator end);
    
    //move constructor
    XorLinkedList(XorLinkedList&& another);
    //copy
    XorLinkedList(XorLinkedList const &another) : XorLinkedList(another.cbegin(),another.cend()) {}
    
#pragma mark - iterators
    typedef iterator_<false> iterator;
    typedef iterator_<true> const_iterator;
    typedef reverse_iterator_<false> reverse_iterator;
    typedef reverse_iterator_<true> const_reverse_iterator;
    
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
    void clear()  noexcept(std::is_nothrow_destructible<node>::value)   {delete head; head=nullptr;tail=nullptr;size_=0;}
    //insert
    //emplace
    void erase(const_iterator from,const_iterator until);

    void push_back(const T &v);
    void push_back(T &&v);
    //emplace_back
    //void pop_back();
    //push_front
    //emplace_front
    //pop_front
    //resize
    //swap
    
#pragma mark - operations
    //merge
    //splice
    //remove(_if)
    //reverse
    //unique
    //sort (what?)
    
    //no need to delete tail, it will be deleted by Node deconstructer recursively
    //head is nullptr if empty; OK to delete
    //FIXME: `~node` is going to change
    ~XorLinkedList()                        {delete head;}
    
    };
#pragma mark - non-member functions

//operator==,!=, <, <=, >, >=
//swap (should I pollute `std` namespace?)

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
            tail = tail->join(new node(std::move(begin)));
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
    node* n = IntToPtr(PtrToInt(one) ^ curr->xorptr);
    if (n) return n;
    return nullptr; //instead of NULL
}

template <typename T>
XorLinkedList<T>::node::~node() noexcept(std::is_nothrow_destructible<T>::value) {
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

//NOTE: `this` must be an end node
template <typename T>
typename XorLinkedList<T>::node * XorLinkedList<T>::node::join(node *newNode) noexcept {
    //A->B, C.
    //B.p = A, C.p = 0
    
    node *endNode = this;
    endNode->xorptr = endNode->xorptr ^ PtrToInt(newNode);
    newNode->xorptr = PtrToInt(endNode);
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

//template <typename T>
//void XorLinkedList<T>::erase(const_iterator from,const_iterator until) {
//    while (from != until) {
//        if (from.prev) { //detach
//            auto prevNode = from.prev;
//            prevNode->xorptr ^= PtrToInt(from.curr);
//        }
//        delete from.curr;
//        from++;
//    }
//}

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


#endif /* XorLinkedList_hpp */

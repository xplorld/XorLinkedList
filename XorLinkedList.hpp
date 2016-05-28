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
#include <functional>
#include <type_traits>
#include "is_container_SFINAE.h"
#include <algorithm>

#define PtrToInt(p) reinterpret_cast<uintptr_t>((void *)(p))
#define IntToPtr(i) reinterpret_cast<node *>((i))

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
        friend void detach_nodes(node *a,node *b,node *c,node *d) {
                //a-b-...-c-d -> a-d, b-...-c
                //a and b can be null
                if (a) {
                    a->xorptr ^= PtrToInt(b);
                    a->xorptr ^= PtrToInt(d);
                }
                if (d) {
                    d->xorptr ^= PtrToInt(c);
                    d->xorptr ^= PtrToInt(a);
                }
                b->xorptr ^= PtrToInt(a);
                c->xorptr ^= PtrToInt(d);
            }
        friend void link_or_unlink_nodes(node *a,node *b) {
            //...-a, b-... -> ...-a-b-...
            //...-a-b-...  -> ...-a, b-...
            if (a) a->xorptr ^= PtrToInt(b);
            if (b) b->xorptr ^= PtrToInt(a);
        }
        uintptr_t xorptr;
        T value;
        
        node(T &&v)
        noexcept(std::is_nothrow_move_constructible<T>::value)
        : xorptr(0),value(std::move(v)){}
        
        node(const T &v)
        noexcept(std::is_nothrow_constructible<T, const T &>::value)
        : xorptr(0),value(v){} //copy
    
        
        template< typename ... Args > //shoule only be called by emplace functions
        node(std::false_type,Args && ... args) : xorptr(0), value(std::forward<Args>(args)...) {}
        
        
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
    
    iterator iteratorAt(size_type n);
    void push_back_(node *n);
    void push_front_(node *n);
    
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
#pragma mark - assignment operator
    XorLinkedList& operator=( const XorLinkedList& other ) {XorLinkedList list(other); list.swap(*this); return *this;}
    XorLinkedList& operator=( XorLinkedList&& other ) {other.swap(*this); return *this;}
    XorLinkedList& operator=( std::initializer_list<T> ilist ) { XorLinkedList list(ilist); list.swap(*this);  return *this;}
    template <typename Container, class = typename std::enable_if<is_container<Container>::value>::type>
    XorLinkedList& operator=(Container container) { XorLinkedList list(std::move(container)); list.swap(*this); return *this;}
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
    void erase_ref(const_iterator &from,const_iterator &until);
    void erase(const_iterator pos);
    void erase_ref(const_iterator &pos);
    
    iterator insert(const_iterator pos, const T& value) {return insertNode(pos, new node(std::addressof(value)));}
    iterator insert(const_iterator pos, T&& value)      {return insertNode(pos, new node(std::move(value)));}
    iterator insert( const_iterator pos, size_type count, const T& value );
    template< class InputIt >
    iterator insert( const_iterator pos, InputIt first, InputIt last );
    iterator insert( const_iterator pos, std::initializer_list<T> ilist ) {return insert(pos, ilist.begin(), ilist.end());}
    
    template< class... Args >
    iterator emplace( const_iterator pos, Args&&... args ) {
        return insertNode(pos, new node(std::false_type(),std::forward<Args>(args)...));
    }
    
    void push_back(const T &v);
    void push_back(T &&v);
    void pop_back() noexcept(std::is_nothrow_destructible<node>::value)     {erase(--cend());}
    template< typename... Args >
    void emplace_back( Args&&... args );
    
    void push_front(const T &v);
    void push_front(T &&v);
    void pop_front() noexcept(std::is_nothrow_destructible<node>::value)    {erase(cbegin());}
    template< typename... Args >
    void emplace_front( Args&&... args );
    
    //only support type with default ctors
    void resize( size_type count );
    void resize( size_type count, const value_type& value );
    
    void swap(XorLinkedList &other) {std::swap(head, other.head); std::swap(tail, other.tail); std::swap(size_, other.size_);}
    
    void assign( size_type count, const T& value );
    template< class InputIt >
    void assign( InputIt first, InputIt last );
    void assign( std::initializer_list<T> ilist );

#pragma mark - operations
    void merge( XorLinkedList& other ) {merge(other, std::less<value_type>());};
    void merge( XorLinkedList&& other ) {merge(other);};
    template <class Compare>
    void merge( XorLinkedList& other, Compare comp );
    template <class Compare>
    void merge( XorLinkedList&& other, Compare comp ) {merge(other,comp);};

    void splice( const_iterator pos, XorLinkedList& other  );
    void splice( const_iterator pos, XorLinkedList&& other ) { splice(pos, other);}
    void splice( const_iterator pos, XorLinkedList& other,  const_iterator it );
    void splice( const_iterator pos, XorLinkedList&& other, const_iterator it ) {splice(pos, other, it);}
    void splice( const_iterator pos, XorLinkedList& other,  const_iterator first, const_iterator last);
    void splice( const_iterator pos, XorLinkedList&& other, const_iterator first, const_iterator last){splice(pos, other, first, last);}
    
    void remove(const T& value);
    
    template< class UnaryPredicate >
    void remove_if( UnaryPredicate p );
    
    //NOTE: this `reverse` invalidates all iterators!
    void reverse() {std::swap(head, tail);}
    void unique() {unique(std::equal_to<value_type>());};
    template< class BinaryPredicate >
    void unique( BinaryPredicate p );
    void sort();
    
#pragma mark - desctructor
    ~XorLinkedList()                        {clear();}
#pragma mark - non-member functions
    friend bool operator==(const XorLinkedList& lhs,const XorLinkedList& rhs) {return std::equal(lhs.begin(), lhs.end(), rhs.begin());}
    friend bool operator!=(const XorLinkedList& lhs,const XorLinkedList& rhs) {return !(lhs == rhs);}
    friend bool operator<(const XorLinkedList& lhs,const XorLinkedList& rhs) {return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());}
    friend bool operator>(const XorLinkedList& lhs,const XorLinkedList& rhs) {return rhs<lhs;}
    friend bool operator<=(const XorLinkedList& lhs,const XorLinkedList& rhs) {return !(lhs > rhs);}
    friend bool operator>=(const XorLinkedList& lhs,const XorLinkedList& rhs) {return !(lhs < rhs);}
    
};


template <typename T>
typename std::tuple< typename XorLinkedList<T>::node *, typename XorLinkedList<T>::node *, typename XorLinkedList<T>::size_type>
XorLinkedList<T>::node::newList(const T& value,size_type count) {
    if (!count) return std::make_tuple(nullptr,nullptr,0);
    node *list = new node(std::addressof(value));
    node *tail = list;
    for (size_type i = 1; i < count; ++i) {
        tail = tail->join(new node(std::addressof(value)));
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

//NOTE: `this` must be head or tail
template <typename T>
typename XorLinkedList<T>::node * XorLinkedList<T>::node::join(node *newNode) noexcept {
    //A->B, C.
    //B.p = A, C.p = 0
    
    link_or_unlink_nodes(this, newNode);
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
    node *head = from.curr;
    node *tail = until.prev;
    node *next = until.curr;
    
    if (!prev) this->head = next;
    if (!next) this->tail = prev;
    detach_nodes(prev, head, tail, next);
    from.prev = nullptr;
    until.curr = nullptr;
    while (from.curr != until.curr) {
        node *deleteNode = from.curr;
        ++from;
        delete deleteNode;
        --size_;
    }
}

template <typename T>
void XorLinkedList<T>::erase_ref(const_iterator &from,const_iterator &until) {
    if (from == until) return;
    node *prev = from.prev; //last remaining node
    node *next = until.curr;
    erase(from,until);
    until.prev = prev;
    from.curr = next;
    --from;
}

template <typename T>
void XorLinkedList<T>::erase(const_iterator pos) {
    const_iterator next = pos;
    ++next;
    erase(pos, next);
}

template <typename T>
void XorLinkedList<T>::erase_ref(const_iterator &pos) {
    const_iterator next = pos;
    ++next;
    erase_ref(pos, next);
}

template <typename T>
template <class Compare>
void XorLinkedList<T>::merge( XorLinkedList& other, Compare comp ) {
    if (this == &other) return;
    iterator f1 = begin();
    iterator e1 = end();
    iterator f2 = other.begin();
    iterator e2 = other.end();
    while (f1 != e1 && f2 != e2) {
        if (comp(*f2,*f1)) { //f2 < f1
            //splice f2 parts before f1
            size_type size = 1;
            iterator m2 = std::next(f2);
            for (; m2 != e2 && comp(*m2,*f1); ++m2,++size); //f2-m2 all < f1
            splice(f1, other, f2, m2);
            //here f1,f2 invalidated
            //e1,e2 may invalidated
            f1 = begin();
            e1 = end();
            f2 = other.begin();
            e2 = other.end();
        } else {
            //f2 >= f1
            ++f1;
        }
    }
    splice(e1, other);
}


template <typename T>
void XorLinkedList<T>::splice( const_iterator pos, XorLinkedList& other ) {
    if (&other == this) return;
    if (!other.size()) return;
    //there must be something in `other`
    node *oldHead = pos.prev; // may be nullptr e.g. pos == begin()
    node *oldTail = pos.curr; // may be nullptr e.g. pos == end()
    node *newHead = other.head;
    node *newTail = other.tail;
    // oldHead - newHead - newTail - oldTail;
    link_or_unlink_nodes(oldHead,oldTail);
    link_or_unlink_nodes(oldHead, newHead);
    link_or_unlink_nodes(newTail, oldTail);
    
    if (!oldHead) head = newHead;
    if (!oldTail) tail = newTail;
    size_ += other.size_;
    
    other.head = nullptr;
    other.tail = nullptr;
    other.size_ = 0;
}

template <typename T>
void XorLinkedList<T>::splice( const_iterator pos, XorLinkedList& other, const_iterator it ) {
    splice(pos, other, it, std::next(it));
}

template <typename T>
void XorLinkedList<T>:: splice( const_iterator pos, XorLinkedList& other,  const_iterator first, const_iterator last) {
    auto distance = std::distance(first, last);
    
    node *oldHead = pos.prev; // may be nullptr e.g. pos == begin()
    node *oldTail = pos.curr; // may be nullptr e.g. pos == end()
    node *newHead = first.curr; //non-null by sepc
    node *newTail = last.prev; //non-null by spec
    node *other_new_head = first.prev; //may be nullptr
    node *other_new_tail = last.curr;  //may be nullptr
    
    
    //other_new_head - other_new_tail
    if (!other_new_head) other.head = other_new_tail;
    if (!other_new_tail) other.tail = other_new_head;
    detach_nodes(other_new_head,newHead,newTail,other_new_tail);
    
    // oldHead - newHead - newTail - oldTail;
    if (!oldHead) head = newHead;
    if (!oldTail) tail = newTail;
    link_or_unlink_nodes(oldHead,oldTail);
    link_or_unlink_nodes(oldHead,newHead);
    link_or_unlink_nodes(newTail,oldTail);
    
    size_ += distance;
    other.size_ -= distance;
}

template <typename T>
void XorLinkedList<T>::remove(const T& value) {
    for (const_iterator i = cbegin();i != cend();++i) {
        if ((*i) == value) {
            erase_ref(i);
        }
    }
}

template <typename T>
template< class UnaryPredicate >
void XorLinkedList<T>::remove_if( UnaryPredicate p ) {
    for (const_iterator i = cbegin();i != cend();++i) {
        if (p(*i)) {
            erase_ref(i);
        }
    }
}

template <typename T>
template< class BinaryPredicate >
void XorLinkedList<T>::unique( BinaryPredicate p ) {
    if (size() < 2) {
        return;
    }
    for (const_iterator i = ++cbegin();i != cend();++i) {
        if (p(i.prev->value , i.curr->value)) {
            erase_ref(i);
        }
    }
}

template <typename T>
void XorLinkedList<T>::sort() {
    //algorithm from libstdc++ 4.6
    //https://gcc.gnu.org/onlinedocs/libstdc++/libstdc++-api-4.6/a00925_source.html#l00355
    // Do nothing if the list has length 0 or 1.
    if (size() > 1)
    {
        XorLinkedList carry;
        XorLinkedList tmp[64];
        XorLinkedList * fill = &tmp[0];
        XorLinkedList * counter;
        
        do
        {
            carry.splice(carry.begin(), *this, begin());
            
            for(counter = &tmp[0];
                counter != fill && !counter->empty();
                ++counter)
            {
                counter->merge(carry);
                carry.swap(*counter);
            }
            carry.swap(*counter);
            if (counter == fill)
                ++fill;
        }
        while ( !empty() );
        
        for (counter = &tmp[1]; counter != fill; ++counter)
            counter->merge(*(counter - 1));
        swap( *(fill - 1) );
    }
}

template <typename T>
typename XorLinkedList<T>::iterator XorLinkedList<T>::insertNodes(const_iterator pos, node* newHead,node *newTail, size_type new_size) {
    node *next = pos.curr;
    node *prev = pos.prev;
    link_or_unlink_nodes(prev,next);
    link_or_unlink_nodes(prev,newHead);
    link_or_unlink_nodes(newTail,next);
    if (!prev) head = newHead;
    if (!next) tail = newTail;
    pos.prev = newTail;
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
typename XorLinkedList<T>::iterator XorLinkedList<T>::iteratorAt(size_type n) {
    return n < size() / 2
            ? std::next(begin(),n)
            : std::prev(end(),size()-n);
}

template <typename T>
void XorLinkedList<T>::push_back_(XorLinkedList::node *n) {
    if (tail) {
        tail = tail->join(n);
    } else {
        //empty linked list
        head = n;
        tail = head;
    }
    size_++;
}

template <typename T>
void XorLinkedList<T>::push_back(const T &v){
    push_back_(new node(std::addressof(v)));
}

template <typename T>
void XorLinkedList<T>::push_back(T &&v){
    push_back_(new node(std::move(v)));
    
}

template <typename T>
template < typename ... Args>
void XorLinkedList<T>::emplace_back( Args&&... args ) {
    push_back_(new node(std::false_type() , std::forward<Args>(args)...));
    
}

template <typename T>
void XorLinkedList<T>::push_front_(XorLinkedList::node *n) {
    if (head) {
        head = head->join(n);
    } else {
        //empty linked list
        head = n;
        tail = head;
    }
    size_++;
}


template <typename T>
template < typename ... Args>
void XorLinkedList<T>::emplace_front( Args&&... args ) {
    push_front_(new node(std::false_type() , std::forward<Args>(args)...));
}

template <typename T>
void XorLinkedList<T>::push_front(const T &v){
    push_front_(new node(std::addressof(v)));
}

template <typename T>
void XorLinkedList<T>::push_front(T &&v){
    push_front_(new node(std::move(v)));
}

template <typename T>
void XorLinkedList<T>::resize( size_type count ) {
    resize(count, value_type());
    //Linear in the difference between the current size and count.
//    if (count < size()) {
//        erase(iteratorAt(count), end());
//    } else {
//        count -= size();
//        XorLinkedList newList(value_type(),count);
//        splice(end(), newList);
//    }
}

template <typename T>
void XorLinkedList<T>::resize( size_type count, const value_type& value ) {
    //Linear in the difference between the current size and count.
    if (count < size()) {
        erase(iteratorAt(count), end());
    } else {
        count -= size();
        XorLinkedList newList(value, count);
        splice(end(), newList);
    }
}


template <typename T>
void XorLinkedList<T>::assign( size_type count, const T& value ) {
    clear();
    XorLinkedList list(value,count);
    list.swap(*this);
}

template <typename T>
template< class InputIt >
void XorLinkedList<T>::assign( InputIt first, InputIt last ) {
    clear();
    XorLinkedList list(first,last);
    list.swap(*this);
}

template <typename T>
void XorLinkedList<T>::assign( std::initializer_list<T> ilist ) {
    clear();
    XorLinkedList list(ilist);
    list.swap(*this);
}


#undef PtrToInt
#undef IntToPtr

#endif /* XorLinkedList_hpp */

# XorLinkedList
A C++ Library that defines a templated XOR-Linked List.

[![Build Status](https://travis-ci.org/xplorld/XorLinkedList.svg?branch=master)](https://travis-ci.org/xplorld/XorLinkedList)
### Overview

- This is a templeated XOR-linked list library. You can generate and populate a list with :
	- STL iterators (for example `Vector::begin()` and `Vector::end()`)
	- STL containers (`Vector`, `List`, `Map`, `initializer_list`...)
	- arrays
	- single element
	- or nothing.

- Implemented all `std::list` methods. Try to `#define list XorLinkedList` :)
- Inherited from `std::iterator`, so feel free to use `#include <algorithm>` or range-based for!
- Full move semantics supported, try `std::move`

### This Library

Try

	#include "XorLinkedList.hpp"

`main.cpp` is a sample and simple & naive test.

### What is a XOR-Linked List?

Good old double Linked list has 2 fields of address pointers, `prev` and `next`. Actually, this implementation is not memory efficient: we only need 1 field, by which  `prev` and `next` can be computed.

	curr->ptr = prev ^ next
	next = curr->ptr ^ prev 
	prev = curr->ptr ^ next 


### Me

`xplorld AT gmail DOT com`

### Lisence
MIT.

Welcome pull requests!

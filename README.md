# XorLinkedList
A C++ Library that defines a templated XOR-Linked List.

### Overview

This is a templeated Xor linked list library. You can generate and populate a list with STL iterators (for example `Vector::begin()` and `Vector::end()`), arrays, initializer lists, single element or nothing.

Provided `begin`, `end` that can increment and decrement.

If not used as parameters, please declare on stack not heap, to make RAII your friend!

### This Library

Try

	#include "XorLinkedList.hpp"

`main.cpp` is a sample and simple & naive test.

### What is a XOR-Linked List?

Good old double Linked list has 2 fields of address pointers, `prev` and `next`. Actually, this implementation is not memory efficient: we only need 1 field, by which  `prev` and `next` can be computed.

	curr->ptr = prev ^ next
	next = curr->ptr ^ prev 
	prev = curr->ptr ^ next 


### ToDo

- `size`?
- remove?
- new List of slice from `iterator` pair?

### Done
- `rbegin` and `rend`
- make `iterator` a real STL-compliant `std::iterator`
- `operator==`
- `const_iterator` 

### Me

`xplorld AT gmail DOT com`

### Lisence
MIT.

Welcome pull requests!
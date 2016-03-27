all: main
	./main
	

CLANG = clang++ -std=c++11

main: XorLinkedList.hpp main.cpp
	$(CLANG) main.cpp -o main
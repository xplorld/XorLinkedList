all: main
	./main
	

CLANG = clang++ -std=c++11 -Wall -Wextra

main: XorLinkedList.hpp main.cpp
	$(CLANG) main.cpp -o main
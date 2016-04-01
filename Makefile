all: main
	./main
	
ifndef CXX
CXX = clang++
endif

CXX11 = $(CXX) -pedantic -std=c++11 -Wall -Wextra

main: XorLinkedList.hpp main.cpp
	$(CXX11) main.cpp -o main
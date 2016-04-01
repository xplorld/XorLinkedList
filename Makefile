all: clean main
	./main
	
ifndef CXX
CXX = clang++
endif

CXX11 = $(CXX) -pedantic $(CPP_FLAG) -std=c++11 -Wall -Wextra

clean:
	rm -rf main
main: XorLinkedList.hpp main.cpp
	$(CXX11) main.cpp -o main
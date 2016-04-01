all: clean main
	./main
	
# ifndef CXX
CXX = clang++
# endif

ifeq ($(CXX),clang++)
	LIB_CPP_FLAG = -lc++
else
	LIB_CPP_FLAG = -lstdc++
endif

CXX11 = $(CXX) -pedantic $(LIB_CPP_FLAG) -std=c++11 -Wall -Wextra

clean:
	rm -rf main
main: XorLinkedList.hpp main.cpp
	$(CXX11) main.cpp -o main
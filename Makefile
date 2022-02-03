CPPFLAGS=-g -std=c++20 
LDFLAGS=-g


main:  graphmap
	g++ $(CPPFLAGS) -o main main.cc GraphMap.o 

graphmap:
	g++ $(CPPFLAGS) -c GraphMap.cc


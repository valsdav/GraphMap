CC=g++
CFLAGS=-I.
DEPS = GraphMap.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: main.o GraphMap.o 
	$(CC) -o main main.o GraphMap.o 


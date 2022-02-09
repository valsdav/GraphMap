CC=g++
CFLAGS=-I.
DEPS = GraphMap.h EcalHahGrid.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: main.o GraphMap.o EcalHashGrid.o
	$(CC) -o main main.o GraphMap.o 



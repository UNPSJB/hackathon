CC=g++
CFLAGS=-c -g -Wall

all: main

main: main.o
	$(CC) -g main.o -o main

main.o:
	$(CC) $(CFLAGS) main.cpp

clean:
	rm *.o main

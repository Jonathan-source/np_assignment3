CC = gcc
CC_FLAGS = -w -g



all: test client server queue


main_curses.o: main_curses.c
	$(CC) -Wall -I. -c main_curses.c

test: main_curses.o
	$(CC) -I./ -Wall main_curses.o -lncurses  -o test 

queue: queue.c queue.h
	$(CC) -Wall -I. -c queue.c

client: client.o
	$(CC) -Wall -pthread -o cchat client.o

server: server.o
	$(CC) -Wall -pthread -o cserverd server.o queue.o
	
clean:
	rm *.o *.a test cserverd cchat queue

SHELL := /bin/bash
CC = gcc
FLAGS = -std=gnu99 -O2 -DNDEBUG -g0 -msse4.2

default:	all

all:	sseTest.s sum

sseTest.s:	sseTest.c
	$(CC) $(FLAGS) -masm=intel -S sseTest.c

sum:	sum.o
	$(CC) $(FLAGS) sum.o -o sum

sum.o:	sum.c
	$(CC) $(FLAGS) sum.c -c

clean:
	rm -f *.s
	rm -f *.o
	rm -f sum

CC=gcc
CFLAGS=-fopenmp -g -std=gnu99
LDFLAGS=-fopenmp

all: v_add hello dotp

v_add: v_add.o

hello: hello.o

dotp: dotp.o

clean:
	rm -f *.o dotp hello v_add


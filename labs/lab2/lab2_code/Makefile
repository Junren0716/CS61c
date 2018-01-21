UNAME_S := $(shell uname -s)
CC=gcc
LD=gcc
CFLAGS=-ggdb -Wall -std=c99
LDFLAGS=

ifeq ($(UNAME_S), Darwin)
    MEMCHECK=valgrind --tool=memcheck --leak-check=full --track-origins=yes --dsymutil=yes --suppressions=osx_vector.supp
endif

ifeq ($(UNAME_S), Linux)
    MEMCHECK=valgrind --tool=memcheck --leak-check=full --track-origins=yes
endif

VECTOR_OBJS=vector.o vector-test.o
VECTOR_PROG=vector-test

BINARIES=$(VECTOR_PROG) lfsr bit_ops

all: $(BINARIES)

$(VECTOR_PROG): $(VECTOR_OBJS)
	$(LD) -o $(VECTOR_PROG) $(VECTOR_OBJS) $(LDFLAGS)

lfsr: lfsr.c
	$(CC) $(CFLAGS) -o lfsr lfsr.c

bit_ops: bit_ops.c
	$(CC) $(CFLAGS) -o bit_ops bit_ops.c

.c.o:
	$(CC) -c -pedantic $(CFLAGS) $<

vector-memcheck: $(VECTOR_PROG)
	$(MEMCHECK) ./$(VECTOR_PROG)

clean: 
	-rm -rf core *.o *~ "#"*"#" Makefile.bak $(BINARIES) *.dSYM
        
vector.c: vector.h
vector-test.c: vector.h


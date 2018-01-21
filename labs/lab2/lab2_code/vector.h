#ifndef CS61C_VECTOR_H_
#define CS61C_VECTOR_H_
/* vector.h originally written by Jeremy Huddleston <jeremyhu@eecs.berkeley.edu> Sp2004
 *
 * So it looks like you've decided to venture into the "other" files of this
 * lab.  Good.  C Header files (the .h extension) are a way of telling other .c
 * files what they can have access to.  You usually include stdlib.h in your
 * C programs, and this process is identical to including this .h file with the
 * one change being:
 *
 * #include "file.h" 
 * versus
 * #include <file.h>
 *	 
 * The difference is that the <> notation is for system header files and the ""
 * is for ones you provide yourself (in your local directory for instance).
 *	 
 * The header file starts off with
 * #ifndef CS61C_VECTOR_H_
 * #define CS61C_VECTOR_H_
 *	 
 * and ends with a final #endif.  This prevents the file from being included
 * more than once which could've possibly resulted in an infinite loop of
 * file inclusions.
 *
 * First, we define the 'vector_t' datatype.  This next line says that a 'vector_t'
 * is the same as a 'struct vector_t'.  So anywhere in the code after this, we
 * can use 'vector_t *' to mean a pointer to a 'struct vector_t' (which is defined in
 * vector.c).  We can get away with doing this even though we don't know what a
 * struct vector is because all struct pointers have the same representation in memory.
 */

#include <sys/types.h>

typedef struct vector_t vector_t;

/*
 *  Next, we provide the prototypes for the functions defined in vector.c.  This
 *  is a way of telling the .c files that #include this header what they will
 *  have access to.
 */

/* Create a new vector */
vector_t *vector_new();

/* Free up the memory allocated for the passed vector */
void vector_delete(vector_t *v);

/* Return the value in the vector */
int vector_get(vector_t *v, size_t loc);

/* Set a value in the vector */
void vector_set(vector_t *v, size_t loc, int value);

#endif

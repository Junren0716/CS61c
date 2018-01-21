#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

int main(int argc, char **argv) {
	vector_t *v;
	
	printf("Calling vector_new()\n");
	v = vector_new();
	
	printf("Calling vector_delete()\n");
	vector_delete(v);
	
	printf("vector_new() again\n");
	v = vector_new();

	printf("These should all return 0 (vector_get()): ");
	printf("%d ", vector_get(v, 0));
	printf("%d ", vector_get(v, 1));
	printf("%d\n", vector_get(v, 2));

	printf("Doing a bunch of vector_set()s\n");
	vector_set(v, 0, 98);
	vector_set(v, 11, 15);
	vector_set(v, 15, -23);
	vector_set(v, 24, 65);
        vector_set(v, 500, 3);
	vector_set(v, 12, -123);
	vector_set(v, 15, 21);
	vector_set(v, 25, 43);

	printf("These should be equal:\n");
	printf("98 = %d\n", vector_get(v, 0));
	printf("15 = %d\n", vector_get(v, 11));
	printf("65 = %d\n", vector_get(v, 24));
	printf("-123 = %d\n", vector_get(v, 12));
	printf("21 = %d\n", vector_get(v, 15));
	printf("43 = %d\n", vector_get(v, 25));
        printf("0 = %d\n", vector_get(v, 23));
        printf("0 = %d\n", vector_get(v, 1));
        printf("0 = %d\n", vector_get(v, 501));
        printf("3 = %d\n", vector_get(v, 500));

        vector_delete(v);

	printf("Test complete.\n");
	
	return 0;
}

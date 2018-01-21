#include <stdio.h>

// include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <x86intrin.h>
#endif

int main(int argc, char **argv)
{
    // set   A  =   |1 3|,     B  =   |3 0|       C =   |0 0|
    //              |2 4|             |0 2|             |0 0|
    double A[4] = {1,2,3,4}, B[4] = {3,0,0,2}, C[4] = {0,0,0,0};

		// We are computing C = C + A x B, which means:
		// C[0] += A[0]*B[0] + A[2]*B[1]
		// C[1] += A[1]*B[0] + A[3]*B[1]
		// C[2] += A[0]*B[2] + A[2]*B[3]
		// C[3] += A[1]*B[2] + A[3]*B[3]

    // load entire matrix into SSE vectors
	__m128d c1 = _mm_loadu_pd(C + 0); // c1 = (C[0],C[1])
	__m128d c2 = _mm_loadu_pd(C + 2); // c2 = (C[2],C[3])

	for (int i = 0; i < 2; i++)
    {
		__m128d a = _mm_loadu_pd(A + i * 2); // load next column of A
		__m128d b1 = _mm_load1_pd(B + 0 + i);
		__m128d b2 = _mm_load1_pd(B + 2 + i); // load next row of B

		c1 = _mm_add_pd(c1, _mm_mul_pd(a, b1)); // multiply and add
		c2 = _mm_add_pd(c2, _mm_mul_pd(a, b2));
    }

    // store the result back into the array
	_mm_storeu_pd(C + 0, c1); // (C[0],C[1]) = c1
	_mm_storeu_pd(C + 2, c2); // (C[2],C[3]) = c2

	// print result
    printf( "|%g %g| * |%g %g| = |%g %g|\n", A[0], A[2], B[0], B[2], C[0], C[2] );
    printf( "|%g %g|   |%g %g|   |%g %g|\n", A[1], A[3], B[1], B[3], C[1], C[3] );

    return 0;
}

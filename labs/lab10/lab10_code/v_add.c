#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define ARRAY_SIZE 10000000
#define REPEAT     100

void method_0(double* x, double* y, double* z) {
#pragma omp parallel
  {
    for(int i = 0; i < ARRAY_SIZE; ++i) {
      z[i] = x[i] + y[i];
    }
  }
}

void method_1(double* x, double* y, double* z) {
#pragma omp parallel
  {
    int gap = omp_get_num_threads();
    int n = omp_get_thread_num();
    for (int i = n; i < ARRAY_SIZE; i += gap) {
      z[i] = x[i] + y[i];
    }
  }
}

void method_2(double* x, double* y, double* z) {
#pragma omp parallel
  {
    int chunk_size = ARRAY_SIZE / omp_get_num_threads();
    int n = omp_get_thread_num();
    // If N threads, breakup the vector into N chunks
    for(int i = n * chunk_size; i < ((n + 1) * chunk_size); ++i) {
      z[i] = x[i] + y[i];
    }
  }
}

void method_3(double* x, double* y, double* z) {
#pragma omp parallel
  {
    #pragma omp for
    for(int i = 0; i < ARRAY_SIZE; ++i) {
      z[i] = x[i] + y[i];
    }
  }
}

// Edit this function
// Split up the function into 4 different methods to parallelize
void v_add(unsigned int method, double* x, double* y, double* z) {
  if (method == 1) {
    method_1(x, y, z);
  } else if (method == 2) {
    method_2(x, y, z);
  } else if (method == 3) {
    method_3(x, y, z);
  } else {
    method_0(x, y, z);
  }
}

double* gen_array(int n) {
    double* array = (double*) malloc(n*sizeof(double));
    for(int i=0; i<n; i++) {
        array[i] = rand()%10000;
    }
    return array;
}

// Double check if it is correct
int verify(unsigned int method, double* x, double* y) {
    double *z_v_add = (double*) malloc(ARRAY_SIZE*sizeof(double));
    double *z_oracle = (double*) malloc(ARRAY_SIZE*sizeof(double));
    v_add(method, x, y, z_v_add);
    for(int i=0; i<ARRAY_SIZE; i++) {
        z_oracle[i] = x[i] + y[i];
    }
    for(int i=0; i<ARRAY_SIZE; i++) {
        if(z_oracle[i] != z_v_add[i]) {
            return 0;
        }
    }
    return 1;
}


int main(int argc, char ** argv) {
    // Generate input vectors and destination vector
    double *x = gen_array(ARRAY_SIZE);
    double *y = gen_array(ARRAY_SIZE);
    double *z = (double*) malloc(ARRAY_SIZE*sizeof(double));
    unsigned int method = 0;

    if (argc == 2)
      method = atoi(argv[1]);

    // Double check v_add is correct
    if(!verify(method, x,y)) {
        printf("v_add does not match oracle\n");
        return 0;
    }
        
    // Test framework that sweeps the number of threads and times each ru
    double start_time, run_time;
    int num_threads = omp_get_max_threads();    
    for(int i=1; i<=num_threads; i++) {
        omp_set_num_threads(i);     
        start_time = omp_get_wtime();
        for(int j=0; j<REPEAT; j++) {
    v_add(method, x,y,z);
        }
        run_time = omp_get_wtime() - start_time;
        printf(" %d thread(s) took %f seconds\n",i,run_time);
    }
}
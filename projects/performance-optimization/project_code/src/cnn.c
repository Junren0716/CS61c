#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include "timestamp.c"

// Include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#include <x86intrin.h>
#endif

// Include OpenMP
#include <omp.h>

// Vol ------------------------------------------------------------------------

// Volumes are used to represent the activations (i.e., state) between the
// different layers of the CNN. They all have three dimensions. The inter-
// pretation of their content depends on the layer that produced them. Before
// the first iteration, the Volume holds the data of the image we want to
// classify (the depth are the three color dimensions). After the last stage
// of the CNN, the Volume holds the probabilities that an image is part of
// a specific category.

/*
 * Represents a three-dimensional array of numbers, and its size. The numbers
 * at (x,y,d) are stored in array w at location ((v->sx * y)+x)*v->depth+d.
 */

typedef struct vol {
  uint64_t sx,sy,depth;
  double* w;
} vol_t;

/*
 * Set the value at a specific entry of the array.
 */

static inline double get_vol(vol_t* v, int x, int y, int d) {
  return v->w[((v->sx * y)+x)*v->depth+d];
}

/*
 * Get the value at a specific entry of the array.
 */

static inline void set_vol(vol_t* v, int x, int y, int d, double val) {
  v->w[((v->sx * y)+x)*v->depth+d] = val;
}

/*
 * Allocate a new array with specific dimensions and default value v.
 */

static vol_t* make_vol(int sx, int sy, int d, double v) {
  vol_t* out = (vol_t*)malloc(sizeof(struct vol));
  out->w = (double*)malloc(sizeof(double)*(sx*sy*d));
  out->sx = sx;
  out->sy = sy;
  out->depth = d;

  for (int x = 0; x < sx; x++)
    for (int y = 0; y < sy; y++)
      for (int z = 0; z < d; z++)
        set_vol(out, x, y, z, v);

  return out;
}

/*
 * Copy the contents of one Volume to another (assuming same dimensions).
 */

static vol_t* copy_vol(vol_t* dest, vol_t* src) {
  // OPT: No need to load from memory repeatedly
  int x_dest = dest->sx;
  int y_dest = dest->sy;
  int z_dest = dest->depth;

  for (int x = 0; x < x_dest; x++)
    for (int y = 0; y < y_dest; y++)
      for (int z = 0; z < z_dest; z++)
        set_vol(dest, x, y, z, get_vol(src, x, y, z));
}

/*
 * Deallocate the array.
 */
void free_vol(vol_t* v) {
  free(v->w);
  free(v);
}

// A note about layers --------------------------------------------------------

/*
 * What follows are the different layers of the CNN. You will not have to
 * understand what these layers are actually doing. In general terms, each
 * layer performs a "forward" operation on a batch of inputs. During this
 * forward operation, the layer takes a set of input Volumes and transforms
 * them into a set of output Volumes (one output for each input). What differs
 * is the operation performed by each layer.
 *
 * In addition to the _forward function, each layer also provides a data
 * structure, holding (fixed) parameters for that layer, a make_ function to
 * allocate an instance of the layer with a particular set of parameters and
 * a load function to load training data for that layer from a file. Note that
 * you will not have to make any changes to any of these functions. The only
 * function you need to consider is the _forward function.
 */

// Convolutional Layer --------------------------------------------------------

// NOTE: ONLY NEED CHANGES IN THIS LAYER (MOST EFFECTIVE TO OPTIMIZE)

typedef struct conv_layer {
  // required
  int out_depth;
  int sx;
  int in_depth;
  int in_sx;
  int in_sy;

  // optional
  int sy;
  int stride;
  int pad;
  double l1_decay_mul;
  double l2_decay_mul;

  // computed
  int out_sx;
  int out_sy;
  double bias;
  vol_t* biases;
  vol_t** filters;

  // my_additional
  uint64_t conv_time;

} conv_layer_t;

conv_layer_t* make_conv_layer(int in_sx, int in_sy, int in_depth,
                              int sx, int filters, int stride, int pad) {
  conv_layer_t* l = (conv_layer_t*)malloc(sizeof(conv_layer_t));

  // required
  l->out_depth = filters;
  l->sx = sx;
  l->in_depth = in_depth;
  l->in_sx = in_sx;
  l->in_sy = in_sy;
    
  // optional
  l->sy = l->sx;
  l->stride = stride;
  l->pad = pad;
  l->l1_decay_mul = 0.0;
  l->l2_decay_mul = 1.0;

  // computed
  l->out_sx = floor((l->in_sx + l->pad * 2 - l->sx) / l->stride + 1);
  l->out_sy = floor((l->in_sy + l->pad * 2 - l->sy) / l->stride + 1);

  // my_additional
  l->conv_time = 0;

  // OPT: No need to load from memory repeatedly
  int inner_depth = l->in_depth;
  int in_x = l->sx;
  int in_y = l->sy;

  l->filters = (vol_t**)malloc(sizeof(vol_t*)*filters);
  for (int i = 0; i < filters; i++) {
    l->filters[i] = make_vol(in_x, in_y, inner_depth, 0.0);
  }

  l->bias = 0.0;
  l->biases = make_vol(1, 1, l->out_depth, l->bias);

  return l;
}

// Note: ONLY NEED to make optimizations to the _forward funct.

// void conv_forward(conv_layer_t* l, vol_t** in, vol_t** out, int start, int end) {
//   // Set starting time for forward part of layer
//   uint64_t forward_time = timestamp_us();

//   // OPT: No need to load from memory repeatedly
//   int outer_depth = l->out_depth;
//   int y_outer = l->out_sy;
//   int x_outer = l->out_sx;

//   for (int i = start; i <= end; i++) {
//     vol_t* V = in[i];
//     vol_t* A = out[i];
        
//     int V_sx = V->sx;
//     int V_sy = V->sy;
//     int xy_stride = l->stride;
  
//     for(int d = 0; d < outer_depth; d++) {
//       vol_t* f = l->filters[d];

//       // OPT: No need to load from memory repeatedly
//       int y_filter = f->sy;
//       int x_filter = f->sx;
//       int depth_filter = f->depth;
//       int depth_V = V->depth;

//       int x = -l->pad;
//       int y = -l->pad;
//       for(int ay = 0; ay < y_outer; y += xy_stride, ay++) {
//         x = -l->pad;
//         for(int ax=0; ax < x_outer; x += xy_stride, ax++) {
//           double a = 0.0;
//           // OPT: Use SIMD Instrinsics for speedup

//           // ---- Original Code ----
//           // for(int fy = 0; fy < y_filter; fy++) {
//           //   int oy = y + fy;
//           //   for(int fx = 0; fx < x_filter; fx++) {
//           //     int ox = x + fx;
//           //     if(oy >= 0 && oy < V_sy && ox >=0 && ox < V_sx) {
//           //       for(int fd=0;fd < depth_filter; fd++) {
//           //         a += f->w[((f->sx * fy)+fx)*depth_filter+fd] * V->w[((V_sx * oy)+ox)*V->depth+fd];
//           //       }
//           //     }
//           //   }
//           // }
//           // -----------------------

//           // Rather than splitting this code into multiple functions, why not just use conditional statements to determine results?
//           // #0: Set an initial variable for summation (to calculate 'a')

//           if (depth_filter != 16 && depth_filter != 20) {
//             for (int fx = 0; fx < x_filter; fx++) {
//               int ox =  x + fx;
//               // #2: There is no need to check all of the conditions seperately
//               //     (This allows for faster exiting from code) 
//               if (ox >= 0 && ox < V_sx) {
//                 for (int fy = 0; fy < y_filter; fy++) {
//                   int oy = y + fy;
//                   if (oy >= 0 && oy < V_sy) {
//                     a += f->w[((x_filter * fy) + fx) * depth_filter + 0] * V->w[((V_sx * oy) + ox) * depth_V + 0];
//                     a += f->w[((x_filter * fy) + fx) * depth_filter + 1] * V->w[((V_sx * oy) + ox) * depth_V + 1];
//                     a += f->w[((x_filter * fy) + fx) * depth_filter + 2] * V->w[((V_sx * oy) + ox) * depth_V + 2];
//                   }
//                 }
//               }
//             }
//           }

//           // Note: We must always initialize our values
//           __m256d a_val = _mm256_setzero_pd();      // Initializes vector values to 0
//           __m256d f_grabber = _mm256_setzero_pd();  // One part of the address to grab from
//           __m256d V_grabber = _mm256_setzero_pd();  // Second part of address to grab from
//           __m256d multiplier = _mm256_setzero_pd();  // Calculates the address and find the right value for a_val

//           // #1: Flip the orders of the x and y loops to improve efficiency
//           for (int fx = 0; fx < x_filter; fx++) {
//             int ox =  x + fx;
//             // #2: There is no need to check all of the conditions seperately
//             //     (This allows for faster exiting from code) 
//             if (ox >= 0 && ox < V_sx) {
//               for (int fy = 0; fy < y_filter; fy++) {
//                 int oy = y + fy;
//                 if (oy >= 0 && oy < V_sy) {
//                   // #3: Split this function into multiple specialized cases (based on input values)
//                   //     (Note: this is the core code for this function, why we need to only modify this)
//                   if (depth_filter == 16 || depth_filter == 20) {
//                     int fd = 0;
//                     // Can simply initialize these outside so that memory is not repeatedly called in for loop
//                     double* f_addr = &(f->w[((x_filter * fy) + fx) * depth_filter]);
//                     double* V_addr = &(V->w[((V_sx * oy) + ox) * depth_V]);
//                     // #4: Optimize this code using SIMD Instructions

//                     if (depth_filter == 16) {
//                       // In layer 4, depth_filter is 16, so loop using SIMD and unrolling (similar, simplified code in second condition)

//                       for (; fd < 16; fd += 4) {
//                         // Note:    Since we're using 256 bits, can split loop up 4 times
//                         // Note #2: 'a' is a double storing the values from an address.
//                         //          We need a way to load this value from the proper location in memory and store it in the vector.

//                         // Note: Arithmatic functions in SIMD require their unique values, so much parse code first
//                         f_grabber = _mm256_loadu_pd(f_addr + fd);
//                         V_grabber = _mm256_loadu_pd(V_addr + fd);

//                         multiplier = _mm256_mul_pd(f_grabber, V_grabber);
//                         a_val = _mm256_add_pd(a_val, multiplier);
//                         // Sum the 4 64 bit floats within the 256-bit a_val
//                         a = (double) a_val[0] + (double) a_val[1] + (double) a_val[2] + (double) a_val[3];
//                       }
//                     } else if (depth_filter == 20) {
//                       // In layer 7, depth_filter is 20, so maintain SIMD structure, but modify existing code
//                       for (; fd < 20; fd += 4) {
//                         f_grabber = _mm256_loadu_pd(f_addr + fd);
//                         V_grabber = _mm256_loadu_pd(V_addr + fd);

//                         multiplier = _mm256_mul_pd(f_grabber, V_grabber);
//                         a_val = _mm256_add_pd(a_val, multiplier);
//                         // Sum the 4 64 bit floats within the 256-bit a_val
//                         a = (double) a_val[0] + (double) a_val[1] + (double) a_val[2] + (double) a_val[3];
//                       }
//                     }
//                   } else {
//                     // In layer 1, depth_filter is simply 3, so unroll into 3 cases
//                     // OPT: Manual Loop Unrolling
//                     a += f->w[((x_filter * fy) + fx) * depth_filter + 0] * V->w[((V_sx * oy) + ox) * depth_V + 0];
//                     a += f->w[((x_filter * fy) + fx) * depth_filter + 1] * V->w[((V_sx * oy) + ox) * depth_V + 1];
//                     a += f->w[((x_filter * fy) + fx) * depth_filter + 2] * V->w[((V_sx * oy) + ox) * depth_V + 2];
//                   }
//                 }
//               }
//             }
//           }
//           // Continue with rest of the Code
//           a += l->biases->w[d];
//           set_vol(A, ax, ay, d, a);
//         }
//       }
//     }
//   }
//   // Account for difference in time since start of layer
//   l->conv_time += timestamp_us() - forward_time;
// }

// #######################################################################################
// AFTER MUCH DELIBERATION, CHOSE TO SPLIT METHODS FOR OPTIMIZATION!!!
// Note: Split this function into multiple specialized cases (based on input values)
// Call each of these functions seperately on the bottom to ensure results are proper

void conv_forward1(conv_layer_t* l, vol_t** in, vol_t** out, int start, int end) {
  // Set starting time for forward part of layer
  uint64_t forward_time = timestamp_us();

  // OPT: No need to load from memory repeatedly
  int outer_depth = l->out_depth;
  int y_outer = l->out_sy;
  int x_outer = l->out_sx;

  for (int i = start; i <= end; i++) {
    vol_t* V = in[i];
    vol_t* A = out[i];
        
    int V_sx = V->sx;
    int V_sy = V->sy;
    int xy_stride = l->stride;
  
    for(int d = 0; d < outer_depth; d++) {
      vol_t* f = l->filters[d];

      // OPT: No need to load from memory repeatedly
      int y_filter = f->sy;
      int x_filter = f->sx;
      int depth_filter = f->depth;
      int depth_V = V->depth;

      int x = -l->pad;
      int y = -l->pad;
      for(int ay = 0; ay < y_outer; y += xy_stride, ay++) {
        x = -l->pad;
        for(int ax=0; ax < x_outer; x += xy_stride, ax++) {
          double a = 0.0;
          // OPT: Use SIMD Instrinsics for speedup

          // ---- Original Code ----
          // for(int fy = 0; fy < y_filter; fy++) {
          //   int oy = y + fy;
          //   for(int fx = 0; fx < x_filter; fx++) {
          //     int ox = x + fx;
          //     if(oy >= 0 && oy < V_sy && ox >=0 && ox < V_sx) {
          //       for(int fd=0;fd < depth_filter; fd++) {
          //         a += f->w[((f->sx * fy)+fx)*depth_filter+fd] * V->w[((V_sx * oy)+ox)*V->depth+fd];
          //       }
          //     }
          //   }
          // }
          // -----------------------

          // #1: Flip the orders of the x and y loops to improve efficiency
          for (int fx = 0; fx < x_filter; fx++) {
            int ox =  x + fx;
            // #2: There is no need to check all of the conditions seperately
            //     (This allows for faster exiting from code) 
            if (ox >= 0 && ox < V_sx) {
              for (int fy = 0; fy < y_filter; fy++) {
                int oy = y + fy;
                if (oy >= 0 && oy < V_sy) {
                  // In layer 1, depth_filter is simply 3, so unroll into 3 cases
                  // OPT: Manual Loop Unrolling
                  a += f->w[((x_filter * fy) + fx) * depth_filter + 0] * V->w[((V_sx * oy) + ox) * depth_V + 0];
                  a += f->w[((x_filter * fy) + fx) * depth_filter + 1] * V->w[((V_sx * oy) + ox) * depth_V + 1];
                  a += f->w[((x_filter * fy) + fx) * depth_filter + 2] * V->w[((V_sx * oy) + ox) * depth_V + 2];
                }
              }
            }
          }
          // Continue with rest of the Code
          a += l->biases->w[d];
          set_vol(A, ax, ay, d, a);
        }
      }
    }
  }
  // Account for difference in time since start of layer
  l->conv_time += timestamp_us() - forward_time;
}

void conv_forward4(conv_layer_t* l, vol_t** in, vol_t** out, int start, int end) {
  // Set starting time for forward part of layer
  uint64_t forward_time = timestamp_us();

  // OPT: No need to load from memory repeatedly
  int outer_depth = l->out_depth;
  int y_outer = l->out_sy;
  int x_outer = l->out_sx;

  for (int i = start; i <= end; i++) {
    vol_t* V = in[i];
    vol_t* A = out[i];
        
    int V_sx = V->sx;
    int V_sy = V->sy;
    int xy_stride = l->stride;
  
    for(int d = 0; d < outer_depth; d++) {
      vol_t* f = l->filters[d];

      // OPT: No need to load from memory repeatedly
      int y_filter = f->sy;
      int x_filter = f->sx;
      int depth_filter = f->depth;
      int depth_V = V->depth;

      int x = -l->pad;
      int y = -l->pad;
      for(int ay = 0; ay < y_outer; y += xy_stride, ay++) {
        x = -l->pad;
        for(int ax=0; ax < x_outer; x += xy_stride, ax++) {
          double a = 0.0;
          // OPT: Use SIMD Instrinsics for speedup

          // ---- Original Code ----
          // for(int fy = 0; fy < y_filter; fy++) {
          //   int oy = y + fy;
          //   for(int fx = 0; fx < x_filter; fx++) {
          //     int ox = x + fx;
          //     if(oy >= 0 && oy < V_sy && ox >=0 && ox < V_sx) {
          //       for(int fd=0;fd < depth_filter; fd++) {
          //         a += f->w[((f->sx * fy)+fx)*depth_filter+fd] * V->w[((V_sx * oy)+ox)*V->depth+fd];
          //       }
          //     }
          //   }
          // }
          // -----------------------

          // Rather than splitting this code into multiple functions, why not just use conditional statements to determine results?
          // #0: Set an initial variable for summation (to calculate 'a')

          // Note: We must always initialize our values
          __m256d a_val = _mm256_setzero_pd();      // Initializes vector values to 0
          __m256d f_grabber = _mm256_setzero_pd();  // One part of the address to grab from
          __m256d V_grabber = _mm256_setzero_pd();  // Second part of address to grab from
          __m256d multiplier = _mm256_setzero_pd();  // Calculates the address and find the right value for a_val

          // #1: Flip the orders of the x and y loops to improve efficiency
          for (int fx = 0; fx < x_filter; fx++) {
            int ox =  x + fx;
            // #2: There is no need to check all of the conditions seperately
            //     (This allows for faster exiting from code) 
            if (ox >= 0 && ox < V_sx) {
              for (int fy = 0; fy < y_filter; fy++) {
                int oy = y + fy;
                if (oy >= 0 && oy < V_sy) {
                  // #3: Split this function into multiple specialized cases (based on input values)
                  //     (Note: this is the core code for this function, why we need to only modify this)
                  int fd = 0;
                  // Can simply initialize these outside so that memory is not repeatedly called in for loop
                  double* f_addr = &(f->w[((x_filter * fy) + fx) * depth_filter]);
                  double* V_addr = &(V->w[((V_sx * oy) + ox) * depth_V]);
                  // #4: Optimize this code using SIMD Instructions

                  // In layer 4, depth_filter is 16, so loop using SIMD and unrolling (similar, simplified code in second condition)
                  for (; fd < 16; fd += 4) {
                    // Note:    Since we're using 256 bits, can split loop up 4 times
                    // Note #2: 'a' is a double storing the values from an address.
                    //          We need a way to load this value from the proper location in memory and store it in the vector.

                    // Note: Arithmatic functions in SIMD require their unique values, so much parse code first
                    f_grabber = _mm256_loadu_pd(f_addr + fd);
                    V_grabber = _mm256_loadu_pd(V_addr + fd);

                    multiplier = _mm256_mul_pd(f_grabber, V_grabber);
                    a_val = _mm256_add_pd(a_val, multiplier);
                    // Sum the 4 64 bit floats within the 256-bit a_val
                    a = (double) a_val[0] + (double) a_val[1] + (double) a_val[2] + (double) a_val[3];
                  }
                }
              }
            }
          }
          // Continue with rest of the Code
          a += l->biases->w[d];
          set_vol(A, ax, ay, d, a);
        }
      }
    }
  }
  // Account for difference in time since start of layer
  l->conv_time += timestamp_us() - forward_time;
}

void conv_forward7(conv_layer_t* l, vol_t** in, vol_t** out, int start, int end) {
  // Set starting time for forward part of layer
  uint64_t forward_time = timestamp_us();

  // OPT: No need to load from memory repeatedly
  int outer_depth = l->out_depth;
  int y_outer = l->out_sy;
  int x_outer = l->out_sx;

  for (int i = start; i <= end; i++) {
    vol_t* V = in[i];
    vol_t* A = out[i];
        
    int V_sx = V->sx;
    int V_sy = V->sy;
    int xy_stride = l->stride;
  
    for(int d = 0; d < outer_depth; d++) {
      vol_t* f = l->filters[d];

      // OPT: No need to load from memory repeatedly
      int y_filter = f->sy;
      int x_filter = f->sx;
      int depth_filter = f->depth;
      int depth_V = V->depth;

      int x = -l->pad;
      int y = -l->pad;
      for(int ay = 0; ay < y_outer; y += xy_stride, ay++) {
        x = -l->pad;
        for(int ax=0; ax < x_outer; x += xy_stride, ax++) {
          double a = 0.0;
          // OPT: Use SIMD Instrinsics for speedup

          // ---- Original Code ----
          // for(int fy = 0; fy < y_filter; fy++) {
          //   int oy = y + fy;
          //   for(int fx = 0; fx < x_filter; fx++) {
          //     int ox = x + fx;
          //     if(oy >= 0 && oy < V_sy && ox >=0 && ox < V_sx) {
          //       for(int fd=0;fd < depth_filter; fd++) {
          //         a += f->w[((f->sx * fy)+fx)*depth_filter+fd] * V->w[((V_sx * oy)+ox)*V->depth+fd];
          //       }
          //     }
          //   }
          // }
          // -----------------------

          // Rather than splitting this code into multiple functions, why not just use conditional statements to determine results?
          // #0: Set an initial variable for summation (to calculate 'a')

          // Note: We must always initialize our values
          __m256d a_val = _mm256_setzero_pd();      // Initializes vector values to 0
          __m256d f_grabber = _mm256_setzero_pd();  // One part of the address to grab from
          __m256d V_grabber = _mm256_setzero_pd();  // Second part of address to grab from
          __m256d multiplier = _mm256_setzero_pd();  // Calculates the address and find the right value for a_val

          // #1: Flip the orders of the x and y loops to improve efficiency
          for (int fx = 0; fx < x_filter; fx++) {
            int ox =  x + fx;
            // #2: There is no need to check all of the conditions seperately
            //     (This allows for faster exiting from code) 
            if (ox >= 0 && ox < V_sx) {
              for (int fy = 0; fy < y_filter; fy++) {
                int oy = y + fy;
                if (oy >= 0 && oy < V_sy) {
                  // #3: Split this function into multiple specialized cases (based on input values)
                  //     (Note: this is the core code for this function, why we need to only modify this)

                  int fd = 0;
                  // Can simply initialize these outside so that memory is not repeatedly called in for loop
                  double* f_addr = &(f->w[((x_filter * fy) + fx) * depth_filter]);
                  double* V_addr = &(V->w[((V_sx * oy) + ox) * depth_V]);
                  // #4: Optimize this code using SIMD Instructions

                  // In layer 7, depth_filter is 20, so maintain SIMD structure, but modify existing code
                  for (; fd < 20; fd += 4) {
                    f_grabber = _mm256_loadu_pd(f_addr + fd);
                    V_grabber = _mm256_loadu_pd(V_addr + fd);

                    multiplier = _mm256_mul_pd(f_grabber, V_grabber);
                    a_val = _mm256_add_pd(a_val, multiplier);
                    // Sum the 4 64 bit floats within the 256-bit a_val
                    a = (double) a_val[0] + (double) a_val[1] + (double) a_val[2] + (double) a_val[3];
                  }
                }
              }
            }
          }
          // Continue with rest of the Code
          a += l->biases->w[d];
          set_vol(A, ax, ay, d, a);
        }
      }
    }
  }
  // Account for difference in time since start of layer
  l->conv_time += timestamp_us() - forward_time;
}

void conv_load(conv_layer_t* l, const char* fn) {
  // Set starting time for load part of layer
  uint64_t load_time = timestamp_us();

  int sx, sy, depth, filters;

  FILE* fin = fopen(fn, "r");

  fscanf(fin, "%d %d %d %d", &sx, &sy, &depth, &filters);
  assert(sx == l->sx);
  assert(sy == l->sy);
  assert(depth == l->in_depth);
  assert(filters == l->out_depth);

  // OPT: No need to load from memory repeatedly
  int outer_depth = l->out_depth;

  for(int d = 0; d < outer_depth; d++)
    for (int x = 0; x < sx; x++)
      for (int y = 0; y < sy; y++)
        for (int z = 0; z < depth; z++) {
          double val;
          fscanf(fin, "%lf", &val);
          set_vol(l->filters[d], x, y, z, val);
        }

  for(int d = 0; d < outer_depth; d++) {
    double val;
    fscanf(fin, "%lf", &val);
    set_vol(l->biases, 0, 0, d, val);
  }
  // Account for difference in time since start of part
  l->conv_time += timestamp_us() - load_time;

  fclose(fin);
}

// Relu Layer -----------------------------------------------------------------

typedef struct relu_layer {
  // required
  int in_depth;
  int in_sx;
  int in_sy;

  // computed
  int out_depth;
  int out_sx;
  int out_sy;

  // my_additional
  uint64_t relu_time;

} relu_layer_t;

relu_layer_t* make_relu_layer(int in_sx, int in_sy, int in_depth) {
  relu_layer_t* l = (relu_layer_t*)malloc(sizeof(relu_layer_t));

  // required
  l->in_depth = in_depth;
  l->in_sx = in_sx;
  l->in_sy = in_sy;

  // computed
  l->out_sx = l->in_sx;
  l->out_sy = l->in_sy;
  l->out_depth = l->in_depth;

  // my_additional
  l->relu_time = 0;

  return l;
}

void relu_forward(relu_layer_t* l, vol_t** in, vol_t** out, int start, int end) {
  // Set starting time for forward part of layer
  uint64_t forward_time = timestamp_us();
  int x_in = l->in_sx;
  int y_in = l->in_sy;
  int depth_in = l->in_depth;

  for (int j = start; j <= end; j++) {
    for (int i = 0; i < x_in * y_in * depth_in; i++) {
      out[j]->w[i] = (in[j]->w[i] < 0.0) ? 0.0 : in[j]->w[i];
    }
  }

  // Account for difference in time since start of part
  l->relu_time += timestamp_us() - forward_time;
}

// Pool Layer -----------------------------------------------------------------

typedef struct pool_layer {
  // required
  int sx;
  int in_depth;
  int in_sx;
  int in_sy;

  // optional
  int sy;
  int stride;
  int pad;

  // computed
  int out_depth;
  int out_sx;
  int out_sy;

  // my_additional
  uint64_t pool_time;

} pool_layer_t;

pool_layer_t* make_pool_layer(int in_sx, int in_sy, int in_depth,
                              int sx, int stride) {
  pool_layer_t* l = (pool_layer_t*)malloc(sizeof(pool_layer_t));

  // required
  l->sx = sx;
  l->in_depth = in_depth;
  l->in_sx = in_sx;
  l->in_sy = in_sy;

  // optional
  l->sy = l->sx;
  l->stride = stride;
  l->pad = 0;

  // computed
  l->out_depth = in_depth;
  l->out_sx = floor((l->in_sx + l->pad * 2 - l->sx) / l->stride + 1);
  l->out_sy = floor((l->in_sy + l->pad * 2 - l->sy) / l->stride + 1);

  // my_additional
  l->pool_time = 0;

  return l;
}

void pool_forward(pool_layer_t* l, vol_t** in, vol_t** out, int start, int end) {
  // Set starting time for forward part of layer
  uint64_t forward_time = timestamp_us();

  for (int i = start; i <= end; i++) {
    vol_t* V = in[i];
    vol_t* A = out[i];
        
    int n=0;
    for(int d=0;d<l->out_depth;d++) {
      int x = -l->pad;
      int y = -l->pad;
      for(int ax=0; ax<l->out_sx; x+=l->stride,ax++) {
        y = -l->pad;
        for(int ay=0; ay<l->out_sy; y+=l->stride,ay++) {
  
          double a = -99999;
          for(int fx=0;fx<l->sx;fx++) {
            for(int fy=0;fy<l->sy;fy++) {
              int oy = y+fy;
              int ox = x+fx;
              if(oy>=0 && oy<V->sy && ox>=0 && ox<V->sx) {
                double v = get_vol(V, ox, oy, d);
                if(v > a) { a = v; }
              }
            }
          }
          n++;
          set_vol(A, ax, ay, d, a);
        }
      }
    }
  }
  // Account for difference in time since start of part
  l->pool_time += timestamp_us() - forward_time;
}

// FC Layer -------------------------------------------------------------------

typedef struct fc_layer {
  // required
  int out_depth;
  int in_depth;
  int in_sx;
  int in_sy;

  // optional
  double l1_decay_mul;
  double l2_decay_mul;

  // computed
  int out_sx;
  int out_sy;
  int num_inputs;
  double bias;
  vol_t* biases;
  vol_t** filters;

  // my_additional
  uint64_t fc_time;

} fc_layer_t;

fc_layer_t* make_fc_layer(int in_sx, int in_sy, int in_depth,
                          int num_neurons) {
  fc_layer_t* l = (fc_layer_t*)malloc(sizeof(fc_layer_t));

  // required
  l->out_depth = num_neurons;
  l->in_depth = in_depth;
  l->in_sx = in_sx;
  l->in_sy = in_sy;
    
  // optional
  l->l1_decay_mul = 0.0;
  l->l2_decay_mul = 1.0;

  // computed
  l->num_inputs = l->in_sx * l->in_sy * l->in_depth;
  l->out_sx = 1;
  l->out_sy = 1;

  // my_additional
  l->fc_time = 0;

  l->filters = (vol_t**)malloc(sizeof(vol_t*)*num_neurons);
  for (int i = 0; i < l->out_depth; i++) {
    l->filters[i] = make_vol(1, 1, l->num_inputs, 0.0);
  }

  l->bias = 0.0;
  l->biases = make_vol(1, 1, l->out_depth, l->bias);

  return l;
}

void fc_forward(fc_layer_t* l, vol_t** in, vol_t** out, int start, int end) {
  // Set starting time for forward part of layer
  uint64_t forward_time = timestamp_us();

  int outer_depth = l->out_depth;
  int in_num = l->num_inputs;

  for (int j = start; j <= end; j++) {
    vol_t* V = in[j];
    vol_t* A = out[j];
        
    for(int i = 0; i < outer_depth; i++) {
      double a = 0.0;
      for(int d = 0; d < in_num; d++) {
        a += V->w[d] * l->filters[i]->w[d];
      }
      a += l->biases->w[i];
      A->w[i] = a;
    }
  }

  // Account for difference in time since start of part
  l->fc_time += timestamp_us() - forward_time;

}

void fc_load(fc_layer_t* l, const char* fn) {
  // Set starting time for load part of layer
  uint64_t load_time = timestamp_us();

  FILE* fin = fopen(fn, "r");

  int num_inputs;
  int out_depth;
  fscanf(fin, "%d %d", &num_inputs, &out_depth);
  assert(out_depth == l->out_depth);
  assert(num_inputs == l->num_inputs);

  int outer_depth = l->out_depth;
  int in_num = l->num_inputs;

  for(int i = 0; i < outer_depth; i++)
    for(int d = 0; d < in_num; d++) {
      double val;
      fscanf(fin, "%lf", &val);
      l->filters[i]->w[d] = val;
    }

  for(int i = 0; i < outer_depth; i++) {
    double val;
    fscanf(fin, "%lf", &val);
    l->biases->w[i] = val;
  }
  // Account for difference in time since start of part
  l->fc_time += timestamp_us() - load_time;

  fclose(fin);
}

// Softmax Layer --------------------------------------------------------------

// Maximum supported out_depth
#define MAX_ES 16

typedef struct softmax_layer {
  // required
  int in_depth;
  int in_sx;
  int in_sy;
  double* es; 

  // computed
  int out_depth;
  int out_sx;
  int out_sy;

  // my_additional
  uint64_t softmax_time;

} softmax_layer_t;

softmax_layer_t* make_softmax_layer(int in_sx, int in_sy, int in_depth) {
  softmax_layer_t* l = (softmax_layer_t*)malloc(sizeof(softmax_layer_t));

  // required
  l->in_depth = in_depth;
  l->in_sx = in_sx;
  l->in_sy = in_sy;

  // computed
  l->out_sx = 1;
  l->out_sy = 1;
  l->out_depth = l->in_sx * l->in_sy * l->in_depth;

  // my_additional
  l->softmax_time = 0;

  l->es = (double*)malloc(sizeof(double)*l->out_depth);

  return l;
}

void softmax_forward(softmax_layer_t* l, vol_t** in, vol_t** out, int start, int end) {
  // Set starting time for forward part of layer
  uint64_t forward_time = timestamp_us();

  double es[MAX_ES];

  int outer_depth = l->out_depth;

  for (int j = start; j <= end; j++) {
    vol_t* V = in[j];
    vol_t* A = out[j];
  
    // compute max activation
    double amax = V->w[0];
    for(int i=1;i<outer_depth;i++) {
      if(V->w[i] > amax) amax = V->w[i];
    }
  
    // compute exponentials (carefully to not blow up)
    double esum = 0.0;
    for(int i=0;i<outer_depth;i++) {
      double e = exp(V->w[i] - amax);
      esum += e;
      es[i] = e;
    }
  
    // normalize and output to sum to one
    for(int i=0;i<outer_depth;i++) {
      es[i] /= esum;
      A->w[i] = es[i];
    }
  }
  // Account for difference in time since start of part
  l->softmax_time += timestamp_us() - forward_time;
}

// Neural Network -------------------------------------------------------------

/*
 * This represents the CNN we will use in this project. It consists of 11
 * layers, which means that there are 12 volumes of data (where the first one
 * is the input data and the last one the classification result).
 */

#define LAYERS 11

typedef struct network {
  vol_t* v[LAYERS+1];
  conv_layer_t* l0;
  relu_layer_t* l1;
  pool_layer_t* l2;
  conv_layer_t* l3;
  relu_layer_t* l4;
  pool_layer_t* l5;
  conv_layer_t* l6;
  relu_layer_t* l7;
  pool_layer_t* l8;
  fc_layer_t* l9;
  softmax_layer_t* l10;
} network_t;

/*
 * Instantiate our specific CNN.
 */

network_t* make_network() {
  network_t* net = (network_t*)malloc(sizeof(network_t));
  net->v[0] = make_vol(32, 32, 3, 0.0);
  net->l0 = make_conv_layer(32, 32, 3, 5, 16, 1, 2);
  net->v[1] = make_vol(net->l0->out_sx, net->l0->out_sy, net->l0->out_depth, 0.0);
  net->l1 = make_relu_layer(net->v[1]->sx, net->v[1]->sy, net->v[1]->depth);
  net->v[2] = make_vol(net->l1->out_sx, net->l1->out_sy, net->l1->out_depth, 0.0);
  net->l2 = make_pool_layer(net->v[2]->sx, net->v[2]->sy, net->v[2]->depth, 2, 2);
  net->v[3] = make_vol(net->l2->out_sx, net->l2->out_sy, net->l2->out_depth, 0.0);
  net->l3 = make_conv_layer(net->v[3]->sx, net->v[3]->sy, net->v[3]->depth, 5, 20, 1, 2);
  net->v[4] = make_vol(net->l3->out_sx, net->l3->out_sy, net->l3->out_depth, 0.0);
  net->l4 = make_relu_layer(net->v[4]->sx, net->v[4]->sy, net->v[4]->depth);
  net->v[5] = make_vol(net->l4->out_sx, net->l4->out_sy, net->l4->out_depth, 0.0);
  net->l5 = make_pool_layer(net->v[5]->sx, net->v[5]->sy, net->v[5]->depth, 2, 2);
  net->v[6] = make_vol(net->l5->out_sx, net->l5->out_sy, net->l5->out_depth, 0.0);
  net->l6 = make_conv_layer(net->v[6]->sx, net->v[6]->sy, net->v[6]->depth, 5, 20, 1, 2);
  net->v[7] = make_vol(net->l6->out_sx, net->l6->out_sy, net->l6->out_depth, 0.0);
  net->l7 = make_relu_layer(net->v[7]->sx, net->v[7]->sy, net->v[7]->depth);
  net->v[8] = make_vol(net->l7->out_sx, net->l7->out_sy, net->l7->out_depth, 0.0);
  net->l8 = make_pool_layer(net->v[8]->sx, net->v[8]->sy, net->v[8]->depth, 2, 2);
  net->v[9] = make_vol(net->l8->out_sx, net->l8->out_sy, net->l8->out_depth, 0.0);
  net->l9 = make_fc_layer(net->v[9]->sx, net->v[9]->sy, net->v[9]->depth, 10);
  net->v[10] = make_vol(net->l9->out_sx, net->l9->out_sy, net->l9->out_depth, 0.0);
  net->l10 = make_softmax_layer(net->v[10]->sx, net->v[10]->sy, net->v[10]->depth);
  net->v[11] = make_vol(net->l10->out_sx, net->l10->out_sy, net->l10->out_depth, 0.0);
  return net;
}

/* 
 * Free our specific CNN.
 */

void free_network(network_t* net) {
  for (int i = 0; i < LAYERS+1; i++)
    free_vol(net->v[i]);

  free(net->l0);
  free(net->l1);
  free(net->l2);
  free(net->l3);
  free(net->l4);
  free(net->l5);
  free(net->l6);
  free(net->l7);
  free(net->l8);
  free(net->l9);
  free(net->l10);

  free(net);
}

/*
 * We organize data as "batches" of volumes. Each batch consists of a number of samples,
 * each of which contains a volume for every intermediate layer. Say we have L layers
 * and a set of N input images. Then batch[l][n] contains the volume at layer l for
 * input image n.
 *
 * By using batches, we can process multiple images at once in each run of the forward
 * functions of the different layers.
 */

typedef vol_t** batch_t;

/*
 * This function allocates a new batch for the network old_net with size images.
 */

batch_t* make_batch(network_t* old_net, int size) {
  batch_t* out = (batch_t*)malloc(sizeof(vol_t**)*(LAYERS+1));
  for (int i = 0; i < LAYERS+1; i++) {
    out[i] = (vol_t**)malloc(sizeof(vol_t*)*size);
    for (int j = 0; j < size; j++) {
      out[i][j] = make_vol(old_net->v[i]->sx, old_net->v[i]->sy, old_net->v[i]->depth, 0.0);
    }
  }
  return out;
}

/*
 * Free a previously allocated batch.
 */

void free_batch(batch_t* v, int size) {
  for (int i = 0; i < LAYERS+1; i++) {
    for (int j = 0; j < size; j++) {
      free_vol(v[i][j]);
    }
    free(v[i]);
  }
  free(v);
}

/*
 * Apply our network to a specific batch of inputs. The batch has to be given
 * as input to v and start/end are the first and the last image in that batch
 * to process (start and end are inclusive).
 */

void net_forward(network_t* net, batch_t* v, int start, int end) {
  // For conv_forward_*, use the different functions optimized for certain inputs
  conv_forward1(net->l0, v[0], v[1], start, end);
  relu_forward(net->l1, v[1], v[2], start, end);
  pool_forward(net->l2, v[2], v[3], start, end);
  conv_forward4(net->l3, v[3], v[4], start, end);
  relu_forward(net->l4, v[4], v[5], start, end);
  pool_forward(net->l5, v[5], v[6], start, end);
  conv_forward7(net->l6, v[6], v[7], start, end);
  relu_forward(net->l7, v[7], v[8], start, end);
  pool_forward(net->l8, v[8], v[9], start, end);
  fc_forward(net->l9, v[9], v[10], start, end);
  softmax_forward(net->l10, v[10], v[11], start, end);
}

/*
 * Putting everything together: Take a set of n input images as 3-dimensional
 * Volumes and process them using the CNN in batches of 1. Then look at the
 * output (which is a set of 10 labels, each of which tells us the likelihood
 * of a specific category) and classify the image as a cat iff the likelihood
 * of "cat" is larger than 50%. Writes the cat likelihood for all images into
 * an output array (0 = definitely no cat, 1 = definitely cat).
 */

#define CAT_LABEL 3
void net_classify_cats(network_t* net, vol_t** input, double* output, int n) {
  // OPT: Open MP Optimization
  //      Use pragma omp parallel and for to split the calculation amongst different threads
  // Note: Syntax based on Lecture 20, Slide 5
  #pragma omp parallel
  {
    batch_t* batch = make_batch(net, 1);

    // #pragma omp parallel for --> Proved slower based on trials
    // Guessing that we need to parallelize the make_batch and free_batch calls too
    #pragma omp for 
    for (int i = 0; i < n; i++) {
      copy_vol(batch[0][0], input[i]);
      net_forward(net, batch, 0, 0);
      output[i] = batch[11][0]->w[CAT_LABEL]; 
    }
    free_batch(batch, 1);
  }

  // Calculate (& output) the time it takes for each layer to complete
  // Note:  %lf --> Long Float
  // Note2: USE DOUBLES (Keeps code looking neater, especially when dividing)
  
  printf("\n---- Time for each segment ----\n\n");

  // Split into different layers because we have 11 layers total in our CNN
  double total_time = (double) net->l0->conv_time;
  printf("Conv. Layer A Time: %lf %s\n", (double) net->l0->conv_time / 1000, "ms");
  total_time += (double) net->l1->relu_time;
  printf("Relu. Layer A Time: %lf %s\n", (double) net->l1->relu_time / 1000, "ms");
  total_time += (double) net->l2->pool_time;
  printf("Pool. Layer A Time: %lf %s\n", (double) net->l2->pool_time / 1000, "ms");
  total_time += (double) net->l3->conv_time;
  printf("Conv. Layer B Time: %lf %s\n", (double) net->l3->conv_time / 1000, "ms");
  total_time += (double) net->l4->relu_time;
  printf("Relu. Layer B Time: %lf %s\n", (double) net->l4->relu_time / 1000, "ms");
  total_time += (double) net->l5->pool_time;
  printf("Pool. Layer B Time: %lf %s\n", (double) net->l5->pool_time / 1000, "ms");
  total_time += (double) net->l6->conv_time;
  printf("Conv. Layer C Time: %lf %s\n", (double) net->l6->conv_time / 1000, "ms");
  total_time += (double) net->l7->relu_time;
  printf("Relu. Layer C Time: %lf %s\n", (double) net->l7->relu_time / 1000, "ms");
  total_time += (double) net->l8->pool_time;
  printf("Pool. Layer C Time: %lf %s\n", (double) net->l8->pool_time / 1000, "ms");
  total_time += (double) net->l9->fc_time;
  printf("FC. Layer Time: %lf %s\n", (double) net->l9->fc_time / 1000, "ms");
  total_time += (double) net->l10->softmax_time;
  printf("SoftMax Layer Time: %lf %s\n", (double) net->l10->softmax_time / 1000, "ms");

  printf("\n----- Time for each layer -----\n\n");

  double conv_frac = (double) net->l0->conv_time + (double) net->l3->conv_time + (double) net->l6->conv_time;
  printf("Conv. Layer Time: %lf %s\n", conv_frac / 1000, "ms");
  printf("        Fraction: %lf\n\n", conv_frac / total_time * 100);

  double relu_frac = (double) net->l1->relu_time + (double) net->l4->relu_time + (double) net->l7->relu_time;
  printf("Relu. Layer Time: %lf %s\n", relu_frac / 1000, "ms");
  printf("        Fraction: %lf\n\n", relu_frac / total_time * 100);

  double pool_frac = (double) net->l2->pool_time + (double) net->l5->pool_time + (double) net->l8->pool_time;
  printf("Pool. Layer Time: %lf %s\n", pool_frac / 1000, "ms");
  printf("        Fraction: %lf\n\n", pool_frac / total_time * 100);
  
  printf("FC. Layer Time: %lf %s\n", (double) net->l9->fc_time / 1000, "ms");
  printf("        Fraction: %lf\n\n", (double) net->l9->fc_time / total_time * 100);
  
  printf("SoftMax Layer Time: %lf %s\n", (double) net->l10->softmax_time / 1000, "ms");
  printf("        Fraction: %lf\n", (double) net->l10->softmax_time / total_time * 100);

  printf("\n---------- COMPLETED ----------\n\n");
}

// IGNORE EVERYTHING BELOW THIS POINT -----------------------------------------

// Including C files in other C files is very bad style and should be avoided
// in any real application. We do it here since we want everything that you
// may edit to be in one file, without having to fix the interfaces between
// the different components of the system.

#include "util.c"
#include "main.c"

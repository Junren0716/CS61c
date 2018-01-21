// Default constants for test sizes.
const int BENCHMARK_SIZE = 1200;
const int PARTEST_SIZE = 1000;

/*
 * Run benchmark to determine Cat/s for a large data set.
 */

int do_benchmark(int argc, char** argv) {

  int num_samples = BENCHMARK_SIZE;
  if (argc > 0)
    num_samples = atoi(argv[0]);

  fprintf(stderr, "\n          *** CS 61C, Summer 2015: Project 4 ***\n\n");
  fprintf(stderr, "RUNNING BENCHMARK ON %d PICTURES...\n", num_samples);

  // Pick BENCHMARK_SIZE random samples, it doesn't matter which.
  int* samples = (int*)malloc(sizeof(int)*num_samples);
  for (int i = 0; i < num_samples; i++) {
    samples[i] = i;
  }

  double time = run_classification(samples, num_samples, NULL);

  free(samples);

  fprintf(stderr, "\nPERFORMANCE: %.2lf Cat/s\n\n", (1000.0 * (double)num_samples / time));
  return 0;
}

/*
 * Run test of classifying individual samples and check the content of every layer against
 * reference output produced by convnet.js.
 */

int do_test(int argc, char** argv) {
  int sample_num = 0;

  if (argc > 0)
    sample_num = atoi(argv[0]);

  assert(sample_num >= 0 && sample_num < 50000);

  fprintf(stderr, "Making network...\n");
  network_t* net = load_cnn_snapshot();

  batch_t* batch = make_batch(net, 1);
  load_sample(batch[0][0], sample_num);

  uint64_t start_time = timestamp_us(); 
  net_forward(net, batch, 0, 0);
  uint64_t end_time = timestamp_us();
  fprintf(stderr, "Time: %lf ms\n", (double)(end_time-start_time) / 1000.0);

  for (int i = 0; i < LAYERS+1; i++) {
    printf("LAYER%d,", i);
    dump_vol(batch[i][0]);
  }

  /*fprintf(stderr, "Classification Results:\n");
  for (int i = 0; i < 10; i++) {
    fprintf(stderr, "Category %d: %lf\n", i, net->v11->w[i]);
  }*/

  free_batch(batch, 1);
}

/*
 * Run a large-scale test to catch parallelism errors that do not occur when testing
 * on individual examples.
 */

int do_partest(int argc, char** argv) {
  int test_size = PARTEST_SIZE;

  if (argc > 0)
    test_size = atoi(argv[0]);

  srand(1234);

  int* samples = (int*)malloc(sizeof(int)*test_size);
  for (int i = 0; i < test_size; i++) {
    samples[i] = rand() % 50000;
  }

  double* kept_output;
  double time = run_classification(samples, test_size, &kept_output);

  for (int i = 0; i < test_size; i++) {
    printf("PAR%d,%lf\n", i, kept_output[i]);
  }
 
  free(samples);
}

/*
 * The actual main function.
 */

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: ./cnn <benchmark|test|partest> [args]\n");
    return 2;
  }

  if (!strcmp(argv[1], "benchmark")) {
    return do_benchmark(argc-2, argv+2);
  }

  if (!strcmp(argv[1], "test")) {
    return do_test(argc-2, argv+2);
  }

  if (!strcmp(argv[1], "partest")) {
    return do_partest(argc-2, argv+2);
  }

  fprintf(stderr, "ERROR: Unknown command\n");

  return 2;
}


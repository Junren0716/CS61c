#include <sys/time.h>

// Place where test data is stored on instructional machines.
static const char* DATA_FOLDER = "/home/ff/cs61c/su15-proj4/cifar-10-batches-bin";

// Function to dump the content of a volume for comparison.
void dump_vol(vol_t* v) {
  printf("%ld,%ld,%ld", v->sx, v->sy, v->depth);
  for (int x = 0; x < v->sx; x++)
    for (int y = 0; y < v->sy; y++)
      for (int z = 0; z < v->depth; z++) {
        printf(",%.20lf", get_vol(v, x, y, z));
      }
  printf("\n");
}

// Load the snapshot of the CNN we are going to run.
network_t* load_cnn_snapshot() {
  network_t* net = make_network();
  conv_load(net->l0, "../data/snapshot/layer1_conv.txt");
  conv_load(net->l3, "../data/snapshot/layer4_conv.txt");
  conv_load(net->l6, "../data/snapshot/layer7_conv.txt");
  fc_load(net->l9, "../data/snapshot/layer10_fc.txt");
  return net;  
}

// Load an image from the cifar10 data set.
void load_sample(vol_t *v, int sample_num) {
  fprintf(stderr, "Loading input sample %d...\n", sample_num);
  
  int batch = sample_num / 10000;
  int ix = sample_num % 10000;

  char fn[1024];
  sprintf(fn, "%s/data_batch_%d.bin", DATA_FOLDER, batch+1);

  FILE* fin = fopen(fn, "rb");
  assert(fin != NULL);

  fseek(fin, ix*3073, SEEK_SET);

  uint8_t data[3073];
  assert(fread(data, 1, 3073, fin) == 3073);

  int outp = 1;
  for (int z = 0; z < 3; z++)
    for (int y = 0; y < 32; y++)
      for (int x = 0; x < 32; x++) {
        set_vol(v, x, y, z, ((double)data[outp++])/255.0-0.5);
      }

  fclose(fin);
}

// Load an entire batch of images from the cifar10 data set (which is divided
// into 5 batches with 10,000 images each).
vol_t** load_batch(int batch) {
  fprintf(stderr, "Loading input batch %d...\n", batch);

  char fn[1024];
  sprintf(fn, "%s/data_batch_%d.bin", DATA_FOLDER, batch+1);

  FILE* fin = fopen(fn, "rb");
  assert(fin != NULL);

  vol_t** batchdata = (vol_t**)malloc(sizeof(vol_t*) * 10000);

  for (int i = 0; i < 10000; i++) {
    batchdata[i] = make_vol(32, 32, 3, 0.0);

    uint8_t data[3073];
    assert(fread(data, 1, 3073, fin) == 3073);

    int outp = 1;
    for (int z = 0; z < 3; z++)
      for (int y = 0; y < 32; y++)
        for (int x = 0; x < 32; x++) {
          set_vol(batchdata[i], x, y, z, ((double)data[outp++])/255.0-0.5);
        }
  }

  fclose(fin);

  return batchdata;
}

vol_t** batches[50];

// Perform the classification (this calls into the functions from cnn.c
double run_classification(int* samples, int n, double** keep_output) {
  fprintf(stderr, "Making network...\n");
  network_t* net = load_cnn_snapshot();

  fprintf(stderr, "Loading batches...\n");
  for (int i = 0; i < n; i++) {
    int batch = samples[i]/10000;
    if (batches[batch] == NULL) {
      batches[batch] = load_batch(batch);
    }
  }

  vol_t** input = (vol_t**)malloc(sizeof(vol_t*)*n);
  double* output = (double*)malloc(sizeof(double)*n);

  for (int i = 0; i < n; i++) {
    input[i] = batches[samples[i]/10000][samples[i]%10000];
  }

  fprintf(stderr, "Running classification...\n");
  uint64_t start_time = timestamp_us(); 
  net_classify_cats(net, input, output, n);
  uint64_t end_time = timestamp_us();

  for (int i = 0; i < n; i++) {
    samples[i] = (output[i] > 0.5) ? 0 : -1;
  }

  double dt = (double)(end_time-start_time) / 1000.0;
  fprintf(stderr, "TIME: %lf ms\n", dt);

  free_network(net);
  free(input);

  if (keep_output == NULL)
    free(output);
  else
    *keep_output = output;

  return dt;
}

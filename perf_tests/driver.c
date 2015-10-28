#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Main driver for perf tests, this is for the moment OVERLY simple and not doing anything you would
//  want for real performance tests but we start here.

// All tests should be defining these methods:
int init_wasm(int);
int run_wasm(int);
void* init_c(int);
int run_c(void*, int);
int wasm_llvm_init();

void print_time(struct timespec* start, struct timespec *end, int iterations) {
  long start_ns = start->tv_nsec;
  long end_ns = end->tv_nsec;
  double average = (end_ns - start_ns) / ((double) iterations);

  printf("Time between start/stop is %ld, average is %f\n", end_ns - start_ns, average);
}

int main(int argc, char** argv) {
  int i;
  struct timespec start, end;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <value>\n", argv[0]);
    return EXIT_FAILURE;
  }

  // Get the parameter.
  char* endptr = NULL;
  int value = strtol(argv[1], &endptr, 0);

  if (endptr == NULL || *endptr != '\0') {
    fprintf(stderr, "Problem with argument, should be integer %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  // Initialize the wasm module.
  wasm_llvm_init();

  // First run the wasm version.
  printf("Running wasm with %d\n", value);
  init_wasm(value);

  // Let us run the wasm version a certain number of times.
  int meta = 1000;
  clock_gettime(CLOCK_REALTIME, &start);
  for (i = 0; i < meta; i++) {
    run_wasm(value);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  print_time(&start, &end, meta);

  printf("Running C with %d\n", value);
  void* data = init_c(value);
  clock_gettime(CLOCK_REALTIME, &start);
  for (i = 0; i < meta; i++) {
    run_c(data, value);
  }
  clock_gettime(CLOCK_REALTIME, &end);
  print_time(&start, &end, meta);
  return EXIT_SUCCESS;
}

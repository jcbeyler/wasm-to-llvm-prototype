/*
// Copyright (c) 2015 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

// Main driver for perf tests, this is for the moment OVERLY simple and not doing anything you would
//  want for real performance tests but we start here.

// All tests should be defining these methods:
int init_wasm(int);
int run_wasm(int);
void* init_c(int);
int run_c(void*, int);
int wasm_llvm_init();

static void print_time(struct timespec* start, struct timespec *end, int iterations, double* result) {
  time_t start_s = start->tv_sec;
  time_t end_s = end->tv_sec;
  long start_ns = start->tv_nsec;
  long end_ns = end->tv_nsec;

  double start_time = start_s * 1000000000.0 + start_ns;
  double end_time = end_s * 1000000000.0 + end_ns;
  double average = (end_time - start_time) / ((double) iterations);

  if (result != NULL) {
    *result = average;
  }

  printf("Time between in average is %f\n", average);
}

void print_results(double* records, int num_runs, char* name) {
  printf("Results;%s;", name);
  int i;
  for (i = 0; i < num_runs; i++) {
    printf("%f;", records[i]);
  }
  printf("\n");
}

static void print_usage(char* name) {
  fprintf(stderr, "Usage: %s [-w] [-c] [-s <name>] <value>\n", name);
  fprintf(stderr, "\t -w only execute wasm\n");
  fprintf(stderr, "\t -c only execute c\n");
  fprintf(stderr, "\t -v <value> what value to use\n");
  fprintf(stderr, "\t -s <name> to put for the CSV results\n");
}

int main(int argc, char** argv) {
  int i;
  struct timespec start, end;

  if (argc < 2) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  // By default, we run both.
  int should_run_wasm = 1;
  int should_run_c = 1;
  int meta = 1;
  int value = 0;
  char* opt_name = NULL;

  // Handle options.
  struct option long_options[] = {
    {"wasm", no_argument, 0, 'w'},
    {"c", no_argument, 0, 'c'},
    {"name", required_argument, no_argument, 's'},
    {"value", required_argument, no_argument, 'v'},
    {NULL, no_argument, 0, 0}
  };

  while (1) {
    int idx = 0;
    int c = getopt_long(argc, argv, "wv:cs:", long_options, &idx);

    if (c == -1) {
      break;
    }

    switch (c) {
      case 'w':
        should_run_c = 0;
        break;
      case 'c':
        should_run_wasm = 0;
        break;
      case 's':
        opt_name = optarg;
        break;
      case 'v': {
          // Get the parameter.
          char* endptr = NULL;
          value = strtol(optarg, &endptr, 0);

          if (endptr == NULL || *endptr != '\0') {
            fprintf(stderr, "Problem with argument, should be integer %s\n", optarg);
            return EXIT_FAILURE;
          }
        }
        break;
      default:
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
  }

  printf("Value used for the run is %d\n", value);

  // Initialize the wasm module.
  printf("Initialization of the wasm module\n");
  wasm_llvm_init();

  // Always init: C part might rely on it.
  init_wasm(value);

  // First run the wasm version.
  int outer;

  // For now only do 5 runs and this is hard-coded...
  int num_runs = 5;
  double* records = malloc(sizeof(*records) * num_runs);
  assert(records != NULL);

  if (should_run_wasm) {
    printf("Running wasm with %d\n", value);

    // Let us run the wasm version a certain number of times.
    printf("Wasm\n");
    for (outer = 0; outer < num_runs; outer++) {
      clock_gettime(CLOCK_MONOTONIC, &start);
      for (i = 0; i < meta; i++) {
        printf("Result %d\n", run_wasm(value));
      }
      clock_gettime(CLOCK_MONOTONIC, &end);
      print_time(&start, &end, meta, records + outer);
    }

    char* name = opt_name != NULL ? opt_name : "Wasm";
    print_results(records, num_runs, name);
  }

  if (should_run_c) {
    printf("Running C with %d\n", value);
    void* data = init_c(value);

    for (outer = 0; outer < num_runs; outer++) {
      clock_gettime(CLOCK_MONOTONIC, &start);
      for (i = 0; i < meta; i++) {
        printf("C %d\n", run_c(data, value));
      }
      clock_gettime(CLOCK_MONOTONIC, &end);
      print_time(&start, &end, meta, records + outer);
    }

    char* name = opt_name != NULL ? opt_name : "C";
    print_results(records, num_runs, name);
  }

  free(records), records = NULL;

  return EXIT_SUCCESS;
}

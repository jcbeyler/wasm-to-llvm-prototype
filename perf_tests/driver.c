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
  long start_s = start->tv_sec;
  long end_s = end->tv_sec;
  long start_ns = start->tv_nsec;
  long end_ns = end->tv_nsec;

  long start_time = start_s * 1000000000 + start_ns;
  long end_time = end_s * 1000000000 + end_ns;
  double average = (end_time - start_time) / ((double) iterations);

  printf("Time between in average is %f\n", average);
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
  int meta = 10;
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (i = 0; i < meta; i++) {
    run_wasm(value);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  print_time(&start, &end, meta);

  printf("Running C with %d\n", value);
  void* data = init_c(value);
  clock_gettime(CLOCK_MONOTONIC, &start);
  for (i = 0; i < meta; i++) {
    run_c(data, value);
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  print_time(&start, &end, meta);
  return EXIT_SUCCESS;
}

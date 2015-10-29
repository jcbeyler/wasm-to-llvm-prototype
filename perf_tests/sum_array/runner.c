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

#include "sum.h"

// Wasm methods.
int wp_setter(int);
int wp_sum(int);

int init_wasm(int n) {
  // Call the setter first.
  // Wasm module only allows 4MB for the size right now.
  const int max = 1000000;
  if (n > max) {
    n = max;
    fprintf(stderr, "Asking for too much, sum.wast only allows 1M elements, setting to 1M\n");
  }

  // Set the array.
  wp_setter(n);

  return 1;
}

int run_wasm(int n) {
  int result = 0;
  // Wasm module only allows 4MB for the size right now.
  const int max = 1000000;
  int i;

  if (n > max) {
    n = max;
    fprintf(stderr, "Asking for too much, sum.wast only allows 1M elements, setting to 1M\n");
  }
  
  for (i = 0; i < 1000; i++) {
    // Sum the array.
    result += wp_sum(n);
  }

  return result;
}

void* init_c(int n) {
  // Set the array.
  return init_sum_c(n);
}

int run_c(void* data, int n) {
  int result = 0;
  // Wasm module only allows 4MB for the size right now.
  const int max = 1000000;
  int i;

  if (n > max) {
    n = max;
    fprintf(stderr, "Asking for too much, sum.wast only allows 1M elements, setting to 1M\n");
  }

  for (i = 0; i < 1000; i++) {
    // Sum the array.
    result += run_sum_c(data, n);
  }

  return result;
}


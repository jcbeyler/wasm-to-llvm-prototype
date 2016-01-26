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

#include "bubble.h"

// Wasm methods.
int wm_1_setter(int);
int wm_1_bubble_sort(int);

int init_wasm(int n) {
  return 1;
}

int run_wasm(int n) {
  int result = 0;
  // Wasm module only allows 4MB for the size right now.
  const int max = 1000000;
  int i, j;

  if (n > max) {
    n = max;
    fprintf(stderr, "Asking for too much, bubble.wast only allows 1M elements, setting to 1M\n");
  }

  for (i = 0; i < 1; i++) {
    // Set the array.
    wm_1_setter(n);

    // Bubble sort.
    wm_1_bubble_sort(n);
  }

  return 0;
}

void* init_c(int n) {
  // Set the array.
  return init_bubble_c(n);
}

int run_c(void* data, int n) {
  int result = 0;
  // Wasm module only allows 4MB for the size right now.
  const int max = 1000000;
  int i;

  if (n > max) {
    n = max;
    fprintf(stderr, "Asking for too much, bubble.wast only allows 1M elements, setting to 1M\n");
  }

  for (i = 0; i < 1; i++) {
    // Set the array.
    init_bubble_c(n);

    // Bubble sort.
    run_bubble_c(data, n);
  }
}


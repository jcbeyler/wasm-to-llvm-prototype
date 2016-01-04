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

#include "branches.h"

// Wasm methods.
int wm_1_branches(int);

int init_wasm(int n) {
  return 1;
}

int run_wasm(int n) {
  int result = 0;
  const int max = 150000;
  int i;

  if (n > max) {
    n = max;
    fprintf(stderr, "Asking for too much, branches.wast only allows 150000, setting to 150000\n");
  }

  for (i = 0; i < 1000; i++) {
    // Sum the array.
    result += wm_1_branches(n);
  }

  return result;
}

void* init_c(int n) {
  // Set the array.
  return init_branches_c(n);
}

int run_c(void* data, int n) {
  int result = 0;
  const int max = 150000;
  int i;

  if (n > max) {
    n = max;
    fprintf(stderr, "Asking for too much, branches.wast only allows 150000, setting to 150000\n");
  }

  for (i = 0; i < 1000; i++) {
    // Sum the array.
    result += run_branches_c(data, n);
  }

  return result;
}


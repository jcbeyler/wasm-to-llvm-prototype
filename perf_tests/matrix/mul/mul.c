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

#include "mul.h"

#include <stdlib.h>
#include <stdio.h>

extern void* wasm_module_0_memory_base;

void* init_mul_c(int n) {
  int offset = n * n + 4;
  int* tab1 = wasm_module_0_memory_base;
  int* tab2 = wasm_module_0_memory_base + offset; 

  int i, j;
  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      tab1[i * n + j] = i * j;
      tab2[i * n + j] = i * j;
    }
  }
  return wasm_module_0_memory_base;
}

static int multiply(int n, int a, int tab1[n][n], int tab2[n][n], int tab3[n][n]) {
  int i, j, k;

  for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {
      tab3[i][j] = a * tab1[i][j] + tab2[i][j];
    }
  }

  return tab3[n/2][n/2];
}

int run_mul_c(void* data, int n) {
  int offset = n * n * 4;
  return multiply(n, n, data, data + offset, data + offset * 2);
}

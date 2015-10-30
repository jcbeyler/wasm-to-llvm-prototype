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

#include "vector.h"

#include <stdlib.h>
#include <stdio.h>

extern void* wasm_module_0_memory_base;

void* init_vector_c(int n) {
  // Just use the linear memory from the wasm module.
  int* tab1 = wasm_module_0_memory_base;
  int* tab2 = tab1 + n;

  int i;
  for (i = 0; i < n; i++) {
    tab1[i] = i;
    tab2[i] = i;
  }

  return wasm_module_0_memory_base;
}

int run_vector_c(void* data, int n) {
  int* tab1 = data;
  int* tab2 = tab1 + n;

  int vector = 0;
  int i;
  for (i = 0; i < n; i++) {
    vector += tab1[i] * tab2[i];
  }
  return vector;
}

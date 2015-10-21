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


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <limits>


extern "C" {
  int execute_asserts(void);

  // Wrapper around the assert trap.
  int wp_assert_trap_handler(char* (*fct)(void)) {
    int idx = 0;
    char* exception = nullptr;

    static int cnt = 0;

    // Only print this out once...
    if (cnt == 0) {
      std::cerr << "Currently the trapping system does not work, ignoring" << std::endl;
      cnt++;
    }
    return -1;
  }
}

int main(void) {
  int res = -1;
  res = execute_asserts();

  if (res == -1) {
    return EXIT_SUCCESS;
    fprintf(stderr, "Executed assertion, success\n");
  } else {
    fprintf(stderr, "Executed assertion, failure for assertion line %d\n", res);
    return EXIT_FAILURE;
  }
}

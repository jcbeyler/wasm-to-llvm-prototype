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


#include <setjmp.h>
#include <fenv.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <limits>


extern "C" {
  jmp_buf env;
  int execute_asserts(void);
  void wasm_llvm_init(void);

  void fpe_handler(int arg) {
    longjmp(env, 1);
  }

  // Wrapper around the assert trap.
  int wp_assert_trap_handler(char* (*fct)(void)) {
    int idx = 0;
    char* exception = nullptr;

    // Since this does not work for all traps yet, I'll disable it for now.
    //   What is baffling my mind is I cannot get the stack overflow sigsegv caught.
    //   I tried many variations of signal/sigaction to no avail... TODO
    static int cnt = 0;
    if (cnt == 0) {
      std::cerr << "Not supporting traps yet." << std::endl;
      cnt++;
    }

    return -1;

#if 0
    sigset_t sigs;
    sigemptyset (&sigs);
    sigaddset (&sigs, SIGFPE);
    sigprocmask (SIG_UNBLOCK, &sigs, NULL);
    feclearexcept(FE_ALL_EXCEPT);
    signal(SIGFPE, fpe_handler);
    static int cnt = 0;

    if (setjmp(env) != 0) {
      std::cerr << "Recovered" << std::endl;
      return -1;
    } else {
      exception = fct();

      std::cerr << "Trap did not occur: " << exception << std::endl;
    }

    // Only print this out once...
    return 0;
#endif
  }
}

int main(void) {
  int res = -1;

  // Call the glue first.
  wasm_llvm_init();

  res = execute_asserts();

  if (res == -1) {
    return EXIT_SUCCESS;
    fprintf(stderr, "Executed assertion, success\n");
  } else {
    fprintf(stderr, "Executed assertion, failure for assertion line %d\n", res);
    return EXIT_FAILURE;
  }
}

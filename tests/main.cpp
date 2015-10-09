#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <limits>


extern "C" {
  int execute_asserts(void);

  // Wrapper around the assert trap.
  bool assert_trap_handler(char* (*fct)(void)) {
    int idx = 0;
    char* exception = nullptr;

    std::cerr << "Currently the trapping system does not work, ignoring" << std::endl;
    return true;
  }
}

int main(void) {
  int res = -1;
  res = execute_asserts();
  fprintf(stderr, "Executed asserts: %d\n", res);

  if (res == -1) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}

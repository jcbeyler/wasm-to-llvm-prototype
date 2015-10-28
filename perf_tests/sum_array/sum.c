#include "sum.h"

#include <stdlib.h>

void* init_sum_c(int n) {
  // We want n * 4-bytes.
  return malloc(4 * n);
}

int run_sum_c(void* data, int n) {
  int* tab = data;
  int sum = 0;
  int i;
  for (i = 0; i < n; i++) {
    sum += tab[i];
  }
  return sum;
}

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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <typeinfo>

extern "C" {
  static void handle_float(float val) {
    // Hack right now to support the testsuite:
    //   From what I've quickly seen, consider a floating point
    //     with nothing after the decimal point (ie 5.0)
    //   printf will either print 5 or 5.0;
    // The testsuite wants 5.
    if (ceil(val) == val) {
      int tmp = val;
      printf("%d. : f32\n", tmp);
    } else {
      printf("%f : f32\n", val);
    }
  }

  static void handle_double(double val) {
    // Hack right now to support the testsuite:
    //   From what I've quickly seen, consider a floating point
    //     with nothing after the decimal point (ie 5.0)
    //   printf will either print 5 or 5.0;
    // The testsuite wants 5.
    if (ceil(val) == val) {
      int tmp = val;
      printf("%d. : f64\n", tmp);
    } else {
      printf("%f : f64\n", val);
    }
  }

  // This is not scalable but right now, it will work.
  void spectest_print_i32(int32_t val) {
    printf("%d : i32\n", val);
  }

  void spectest_print_i64(int64_t val) {
    printf("%ld : i64\n", val);
  }

  void spectest_print_f64(double val) {
    handle_double(val);
  }

  void spectest_print_f32(float val) {
    handle_float(val);
  }

  void spectest_print_i32_f32(int32_t val, float fval) {
    printf("%d : i32\n", val);
    handle_float(fval);
  }

  void spectest_print_i64_f64(int64_t val1, double val2) {
    printf("%ld : i64\n", val1);
    handle_double(val2);
  }
}

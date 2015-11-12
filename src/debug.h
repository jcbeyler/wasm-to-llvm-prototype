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
#ifndef H_DEBUG
#define H_DEBUG

// Easy way to get debug or not and play with it...
#define DEBUG 0
#define LEX_GROUP 0
#define LEX_VERBOSITY 0
#define BISON_GROUP 0
#define BISON_VERBOSITY 0
#define PASS_DRIVER_GROUP 1
#define PASS_DRIVER_VERBOSITY 0
#define VERBOSE_LEVEL 0

#if DEBUG == 0
#define DEBUG_PRINT(GROUP, VERBOSE, ...)
#else
#define DEBUG_PRINT(GROUP, VERBOSE, ...) do { \
  if (GROUP && VERBOSE >= VERBOSE_LEVEL) { \
    fprintf(stderr, __VA_ARGS__); \
  } \
}while(0)
#endif

#define BISON_TEST (BISON_GROUP && BISON_VERBOSITY >= VERBOSE_LEVEL)

#define BISON_PRINT(...) \
    DEBUG_PRINT(BISON_GROUP, BISON_VERBOSITY, __VA_ARGS__)

#define PASS_DRIVER_PRINT(...) \
    DEBUG_PRINT(PASS_DRIVER_GROUP, PASS_DRIVER_VERBOSITY, __VA_ARGS__)

#define BISON_PRINT_TABS(n) \
    if (DEBUG && BISON_GROUP && BISON_VERBOSITY >= VERBOSE_LEVEL) { \
      for (int i = 0; i < n; i++) { \
        fprintf(stderr, "\t"); \
      } \
    }

#define BISON_TABBED_PRINT(n, ...) \
  BISON_PRINT_TABS(n); \
  BISON_PRINT(__VA_ARGS__);

void PrintLine(int line);

#endif

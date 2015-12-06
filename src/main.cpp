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

#include <iostream>
#include <unistd.h>
#include <getopt.h>

#include "debug.h"
#include "driver.h"
#include "globals.h"
#include "wasm_file.h"

int yyparse();

void PrintUsage(char* exec_name) {
  std::cerr << "Usage: " << exec_name << " <filename>" << std::endl;
  std::cerr << "\tOption is: -n/--no-opt, no verification and no optimizations\n" << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    PrintUsage(argv[0]);
    return EXIT_FAILURE;
  }

  struct option long_options[] = {
    {"no-opt", 0, 0, 'n'},
    {"help", 0, 0, 'h'},
    {nullptr, 0, 0, 0}
  };

  while (1) {
    int idx = 0;
    int c = getopt_long(argc, argv, "nh", long_options, &idx);

    if (c == -1) {
      break;
    }

    switch (c) {
      case 'n':
        std::cerr << "Disabling Verifications and Optimizations" << std::endl;
        Globals::Get()->DisableVerificationOptimization();
        break;
      case 'h':
        PrintUsage(argv[0]);
        return EXIT_SUCCESS;
      default:
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }
  }

  // Set up global variable singleton.
  Globals::Get()->SetFileName(argv[1]);

  BISON_PRINT("Parsing %s\n", argv[1]);

  FILE* f = freopen(argv[argc - 1], "r", stdin);
  
  if (f == nullptr) {
    std::cerr << "File " << argv[argc - 1] << " not opening" << std::endl;
    PrintUsage(argv[0]);
    return EXIT_FAILURE;
  }

  if (yyparse() == 0) {
    BISON_PRINT("Done Parsing %s\n", argv[1]);

    WasmFile* file = Globals::Get()->GetWasmFile();

    Driver driver(file);
    driver.Drive();
  }

  return EXIT_SUCCESS;
}

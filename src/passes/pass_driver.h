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

#ifndef H_PASS_DRIVER
#define H_PASS_DRIVER

#include <vector>

// Forward declaration.
class WasmPass;
class WasmFile;
class WasmFunction;

class PassDriver {
  protected:
    WasmFile* file_;
    std::vector<WasmPass*> passes_;

    void InitPasses();
    void RunPasses();
    void CleanUpPasses();
    void RunPassesOnFunction(WasmFunction* fct);
    void PopulatePasses();

  public:
    PassDriver(WasmFile* f) : file_(f) {
      PopulatePasses();
    }

    void Drive() {
      InitPasses();
      RunPasses();
      CleanUpPasses();
    }

    void AddPass(WasmPass* pass) {
      passes_.push_back(pass);
    }
};

#endif

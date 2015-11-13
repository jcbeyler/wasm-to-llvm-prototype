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

#include <list>
#include <vector>

#include "basic.h"
#include "debug.h"
#include "pass.h"
#include "pass_driver.h"
#include "wasm_file.h"

void PassDriver::InitPasses() {
  // Before running the passes: initialize them.
  for (auto elem : passes_) {
    elem->Init();
  }
}

void PassDriver::RunPasses() {
  // We want to go through each module.
  std::vector<WasmModule*>& modules = file_->GetWasmModules();

  for (auto module : modules) {
    std::list<WasmFunction*>& functions = module->GetWasmFunctions();

    for (auto fct : functions) {
      PASS_DRIVER_PRINT("Considering %s\n", fct->GetName().c_str());
      RunPassesOnFunction(fct);
    }
  }
}

void PassDriver::RunPassesOnFunction(WasmFunction* fct) {
  // Before running the passes.
  for (auto elem : passes_) {
    PASS_DRIVER_PRINT("Running pass %s\n", elem->GetName()); 
    if (elem->Gate(fct) == true) {
      // Call the three callbacks.
      void* data = elem->PreRun(fct);
      elem->Run(fct, data);
      elem->PostRun(data);
    }
  }
}

void PassDriver::CleanUpPasses() {
  // Before running the passes: initialize them.
  for (auto elem : passes_) {
    elem->CleanUp();
  }
}

void PassDriver::PopulatePasses() {
  UnreachablePass* unreachable = new UnreachablePass();

  AddPass(unreachable);
}

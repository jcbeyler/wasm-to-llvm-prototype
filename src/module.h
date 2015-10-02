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
#ifndef H_MODULE
#define H_MODULE


// TODO: find better.
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"


#include "export.h"

#include <list>
#include <map>
#include <vector>

// Forward declaration.
class WasmFunction;
class WasmFile;

/**
 * Base module node for Wasm, contains right now exports and functions
 */

class WasmModule {
  protected:
    llvm::Module* module_;
    llvm::legacy::PassManager* fpm_;
    WasmFile* file_;

    // Created during building.
    std::list<WasmFunction*> functions_;
    std::list<WasmExport*> exports_;

    // For reference later.
    std::map<std::string, WasmFunction*> map_functions_;
    std::vector<WasmFunction*> vector_functions_;

  public:
    WasmModule(llvm::Module* module = nullptr, llvm::legacy::PassManager* fpm = nullptr, WasmFile* file = nullptr) :
      module_(module), fpm_(fpm), file_(file) {
    }

    void AddFunction(WasmFunction* wf) {
      functions_.push_front(wf);
    }

    void AddExport(WasmExport* e) {
      exports_.push_front(e);
    }

    llvm::Module* GetModule() const {
      return module_;
    }

    void AddFunctionAndRegister(WasmFunction* wf) {
      AddFunction(wf);
      map_functions_[wf->GetName()] = wf;
    }

    void Generate();
    void Dump();
    void Initialize();

    WasmFunction* GetWasmFunction(const char* name, bool check_file = true) const;
    WasmFunction* GetWasmFunction(size_t idx) const;

    llvm::Function* GetOrCreateIntrinsic(llvm::Intrinsic::ID name, ETYPE type);
    llvm::Function* GetWasmAssertTrapFunction();
};

#endif

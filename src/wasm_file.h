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
#ifndef H_WASMFILE
#define H_WASMFILE

#include "function.h"
#include "module.h"
#include "wasm_asserts.h"

class WasmFile {
  protected:
    WasmAsserts* asserts_;
    WasmModule* module_;
    WasmModule* assert_module_;

  public:
    WasmFile() : module_(nullptr), asserts_(nullptr), assert_module_(nullptr) {
    }

    void AddModule(WasmModule* module) {
      // Currently we only support one module.
      assert(module_ == nullptr);
      module_ = module;
    }

    void AddAsserts(WasmAsserts* asserts) {
      // Currently we only support one group of asserts.
      assert(asserts_ == nullptr);
      asserts_ = asserts;
    }

    void Dump() {
      if (module_ != nullptr) {
        module_->Dump();
      }

      if (asserts_ != nullptr) {
        asserts_->Dump();
      }

      if (assert_module_ != nullptr) {
        assert_module_->Dump();
      }
    }

    void Generate() {
      if (module_ != nullptr) {
        module_->Generate();
      }

      if (asserts_ != nullptr) {
        asserts_->Generate(this);
      }
    }

    WasmFunction* GetWasmFunction(const char* name) {
      // Return function if found.
      assert(module_ != nullptr);
      WasmFunction* fct = module_->GetWasmFunction(name, false);
      return fct;
    }

    llvm::Function* GetFunction(const char* name) {
      // Return function if found.
      assert(module_ != nullptr);

      WasmFunction* fct = module_->GetWasmFunction(name, false);

      if (fct == nullptr) {
        return nullptr;
      }

      return fct->GetFunction();
    }

    WasmModule* GetAssertModule() {
      if (assert_module_ == nullptr) {
        llvm::Module* module =
          new llvm::Module("WasmAssertModule", llvm::getGlobalContext());
        assert_module_ = new WasmModule(module, nullptr, this);
      }
      return assert_module_;
    }
};

#endif

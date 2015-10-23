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
#include "wasm_assert.h"

class WasmFile {
  protected:
    WasmAsserts asserts_;
    std::vector<WasmModule*> modules_;
    WasmModule* assert_module_;
    WasmModule* glue_module_;

  public:
    WasmFile() : assert_module_(nullptr) {
    }

    void AddModule(WasmModule* module) {
      // Currently we only support one module.
      modules_.push_back(module);
      module->SetWasmFile(this);
    }

    void AddAssert(WasmAssert* a) {
      asserts_.AddAssert(a);
    }

    void Print() {
      for (auto module : modules_) {
        module->Print();
      }

      if (assert_module_ != nullptr) {
        assert_module_->Print();
      }

      if (glue_module_ != nullptr) {
        glue_module_->Print();
      }
    }

    void Dump() {
      for (auto module : modules_) {
        module->Dump();
      }

      asserts_.Dump();

      if (assert_module_ != nullptr) {
        assert_module_->Dump();
      }
    }

    void Generate() {
      for (auto module : modules_) {
        module->Generate();
      }

      asserts_.Generate(this);

      GenerateInitializeModules();
    }

    void GenerateInitializeModules();

    WasmFunction* GetWasmFunction(const char* name) {
      // Return function if found.
      for (auto module : modules_) {
        WasmFunction* fct = module->GetWasmFunction(name, false);

        if (fct != nullptr) {
          return fct;
        }
      }

      return nullptr;
    }

    llvm::Function* GetFunction(const char* name) {
      // Return function if found.
      for (auto module : modules_) {
        WasmFunction* fct = module->GetWasmFunction(name, false);

        if (fct != nullptr) {
          return fct->GetFunction();
        }
      }

      return nullptr;
    }

    WasmModule* GetAssertModule() {
      if (assert_module_ == nullptr) {
        llvm::Module* module =
          new llvm::Module("WasmAssertModule", llvm::getGlobalContext());
        assert_module_ = new WasmModule(module, nullptr, this);
      }
      return assert_module_;
    }

    llvm::Module* GetIntrinsicModule() {
      if (modules_.size() > 0) {
        return modules_[0]->GetModule();
      } else {
        WasmModule* module = new WasmModule();
        modules_.push_back(module);
        return module->GetModule();
      }
    }
};

#endif

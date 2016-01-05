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
#include "wasm_script.h"

class WasmFile {
  protected:
    WasmScript script_;
    std::vector<WasmModule*> modules_;
    WasmModule* script_module_;
    WasmModule* glue_module_;

  public:
    WasmFile() : script_module_(nullptr), glue_module_(nullptr) {
    }

    void AddModule(WasmModule* module) {
      modules_.push_back(module);
      module->SetWasmFile(this);
    }

    void AddScriptElem(WasmScriptElem* wse) {
      script_.AddScriptElem(wse);
    }

    void Print() {
      for (auto module : modules_) {
        module->Print();
      }

      if (script_module_ != nullptr) {
        script_module_->Print();
      }

      if (glue_module_ != nullptr) {
        glue_module_->Print();
      }
    }

    void Dump() {
      for (auto module : modules_) {
        module->Dump();
      }

      script_.Dump();

      if (script_module_ != nullptr) {
        script_module_->Dump();
      }
    }

    void Initialize();

    void Generate() {
      for (auto module : modules_) {
        module->Generate();
      }

      script_.Generate(this);

      GenerateInitializeModules();
    }

    void GenerateInitializeModules();

    WasmFunction* GetWasmFunction(const char* name, unsigned int line = ~0);

    llvm::Function* GetFunction(const char* name, unsigned int line = ~0) {
      // Return function if found.
      for (auto module : modules_) {
        WasmFunction* fct = module->GetWasmFunction(name, false, line);

        if (fct != nullptr) {
          return fct->GetFunction();
        }
      }

      return nullptr;
    }

    std::vector<WasmModule*>& GetWasmModules() {
      return modules_;
    }

    WasmModule* GetAssertModule() {
      if (script_module_ == nullptr) {
        llvm::Module* module =
          new llvm::Module("WasmScriptModule", llvm::getGlobalContext());
        script_module_ = new WasmModule(module, nullptr, this);
      }
      return script_module_;
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

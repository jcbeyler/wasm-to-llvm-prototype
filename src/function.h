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
#ifndef H_FUNCTION
#define H_FUNCTION

#include <list>
#include <vector>
#include <sstream>

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "debug.h"
#include "function_field.h"

using namespace llvm;

// Forward declaration.
class WasmModule;

/**
 * Definition of a function node in the Wasm format
 */
class WasmFunction {
  protected:
    std::string name_;
    Function* fct_;
    std::list<FunctionField*>* fields_;
    std::vector<ParamField*> params_;
    std::vector<Local*> locals_;
    WasmModule* module_;

    std::vector<llvm::BasicBlock*> labels_;

    // We have redundancy-ish here: since Wasm can do index or named, this helps make it
    //   transparent and easier to maintain.
    std::map<std::string, llvm::AllocaInst*> map_values_;
    std::vector<llvm::AllocaInst*> vector_values_;

    std::vector<Expression*> ast_;
    ETYPE result_;

    llvm::Value* local_base_;

    // Protected methods.
    void GetBaseMemory(llvm::IRBuilder<>& builder);

  public:
    WasmFunction(std::list<FunctionField*>* f = nullptr, const std::string& s = "$anonymous",
                 llvm::Function* fct = nullptr, WasmModule* module = nullptr, ETYPE result = VOID) :
      name_(s), fct_(fct), module_(module), fields_(f), result_(result), local_base_(nullptr) {
        // If anonymous, let's add a unique suffix.
        if (name_ == "anonymous") {
          static int cnt = 0;
          std::ostringstream oss;
          oss << name_ << "_" << cnt;
          cnt++;
          name_ = oss.str();
        }
    }

    void PushLabel(llvm::BasicBlock* bb) {
      labels_.push_back(bb);
    }

    void PopLabel() {
      labels_.pop_back();
    }

    llvm::BasicBlock* GetLabel(size_t from_last);
    llvm::BasicBlock* GetLabel(const char* name);

    void SetName(const char* s) {
      name_ = s;
    }

    const std::string& GetName() const {
      return name_;
    }

    void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "Function is name_d %s\n", name_.c_str());

      if (fields_) {
        for (std::list<FunctionField*>::const_iterator iter = fields_->begin();
            iter != fields_->end();
            iter++) {
          FunctionField* elem = *iter;
          elem->Dump(tabs + 1);
          if (elem->GoesToTheLine() == false) {
            BISON_PRINT("\n");
          }
        }
      }
    }

    llvm::Function* GetFunction() const {
      return fct_;
    }

    llvm::Value* GetLocalBase() const {
      return local_base_;
    }

    ETYPE GetResult() const {
      return result_;
    }

    WasmModule* GetModule() const {
      return module_;
    }

    llvm::AllocaInst* GetVariable(const char* name) const;
    llvm::AllocaInst* GetVariable(size_t idx) const;

    llvm::AllocaInst* CreateAlloca(const char* name, llvm::Type* type, llvm::IRBuilder<>& builder);

    void PopulateLocalHolders(llvm::IRBuilder<>& builder);
    void PopulateAllocas(llvm::IRBuilder<>& builder);
    llvm::AllocaInst* Allocate(const char* name, llvm::Type* type, llvm::IRBuilder<>& builder);

    void Generate(WasmModule* module);
    void GeneratePrototype(WasmModule* module);
    void Populate();

    FunctionType* FindReturnType() const;
    void FindParams(std::vector<llvm::Type*>& params_) const;

    llvm::Type* GetReturnType() const;
    llvm::Value* HandleReturn(llvm::Value* result, llvm::IRBuilder<>& builder) const;

};

#endif

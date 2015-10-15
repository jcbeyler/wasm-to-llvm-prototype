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
#include <string>
#include <iostream>
#include <list>

#include "debug.h"
#include "function.h"
#include "module.h"
#include "wasm_file.h"
#include "utility.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/Intrinsics.h"

llvm::Function* WasmModule::GetOrCreateIntrinsic(llvm::Intrinsic::ID id, ETYPE type) {
  llvm::Module* module = file_->GetIntrinsicModule();

  std::vector<Type*> args;

  // For the moment they are all using just integers are their arguments.
  args.push_back(ConvertType(type));

  llvm::Function* fct;

  if (module == module_) {
    fct = llvm::Intrinsic::getDeclaration(module, id, args);
  } else {
    // In this case, we don't care about this fct. What we want is get a prototype for it.
    std::string fname = llvm::Intrinsic::getName(id, args);

    // Create the function type.
    llvm::FunctionType* fct_type = llvm::Intrinsic::getType(llvm::getGlobalContext(), id, args);

    // Use the prototype instead.
    fct = Function::Create(fct_type, Function::ExternalLinkage, fname.c_str(), module_);  
  }

  return fct;
}

llvm::Function* WasmModule::GetWasmAssertTrapFunction() {
  WasmFunction* wasm_fct = GetWasmFunction("assert_trap_handler", true);;
  llvm::Function* fct;

  if (wasm_fct == nullptr) {
    std::vector<Type*> params;

    // Create the char* (*)() type.
    llvm::PointerType* ptr_type = llvm::PointerType::get(llvm::IntegerType::get(llvm::getGlobalContext(), 8), 0);

    std::vector<Type*> fct_args;
    llvm::FunctionType* fct_type = llvm::FunctionType::get(ptr_type, fct_args, false);

    llvm::PointerType* ptr_to_fct = PointerType::get(fct_type, 0);
    params.push_back(ptr_to_fct);

    // Finally create the actual function type: int foo (char* (*)()).
    llvm::FunctionType* ft = FunctionType::get(llvm::Type::getInt32Ty(getGlobalContext()), params, false);

    fct = llvm::Function::Create(ft, Function::ExternalLinkage, "assert_trap_handler", module_);

    wasm_fct = new WasmFunction(nullptr, fct->getName(), fct, this, INT_32);

    AddFunctionAndRegister(wasm_fct);
  } else {
    fct = wasm_fct->GetFunction();
  }

  assert(fct != nullptr);

  return fct;
}

void WasmModule::Generate() {
  // Make the module, which holds all the code.
  module_ = new llvm::Module(name_.c_str(), llvm::getGlobalContext());

  // Some maintenance before to do.
  Initialize();

  // For each function, go from here to LLVM.
  for (auto it : functions_) {
    WasmFunction& fct = *it;

    fct.Generate(this);

    assert((llvm::verifyFunction(*fct.GetFunction(), &llvm::outs()) == false));
  }

  // Run the optimizations.
  fpm_->run(*module_);

  for (auto it : functions_) {
    WasmFunction& fct = *it;
    assert((llvm::verifyFunction(*fct.GetFunction(), &llvm::outs()) == false));
  }

  // Handle now the exports.
  std::map<std::string, WasmFunction*> exported_functions;

  for (auto elem : exports_) {
    const std::string& name = elem->GetName();
    Variable* var = elem->GetVariable();

    WasmFunction* fct = nullptr;
    if (var->IsString()) {
      fct = map_functions_[var->GetString()];
    } else {
      size_t idx = var->GetIdx();
      assert(idx < vector_functions_.size());
      fct = vector_functions_[idx];
    }

    assert(fct != nullptr);
    exported_functions[name] = fct;

    // Update name.
    fct->SetName(name.c_str());
    llvm::Function* llvm_fct = fct->GetFunction();
    llvm_fct->setName(name);
  }

  // Clear the old map and the vector functions.
  map_functions_.clear();
  vector_functions_.clear();

  // Now we only have the exported methods.
  map_functions_ = exported_functions;
}

void WasmModule::Initialize() {
  for (auto it : functions_) {
    map_functions_[it->GetName()] = it;
    vector_functions_.push_back(it);
  }

  // Create a new pass manager attached to it.
  fpm_ = new legacy::PassManager();

  llvm::PassManagerBuilder pmb;
  pmb.OptLevel = 1;
  pmb.populateModulePassManager(*fpm_);
}

void WasmModule::Dump() {
  BISON_PRINT("Module Dump:\n");

  for (auto elem : functions_) {
    WasmFunction* f = elem;

    if (f) {
      f->Dump();
      BISON_PRINT("\n");
    }
  }

  for (auto elem: exports_) {
    WasmExport* exp = elem;

    if (exp) {
      exp->Dump();
      BISON_PRINT("\n");
    }
  }

  if (module_ != nullptr) {
    BISON_PRINT("LLVM Dump\n");
    module_->dump();
    BISON_PRINT("LLVM Dump Done\n");
  }
}

WasmFunction* WasmModule::GetWasmFunction(const char* name, bool check_file) const {
  // First look in our module.
  std::map<std::string, WasmFunction*>::const_iterator it = map_functions_.find(name);
  WasmFunction* fct = nullptr;

  if (it != map_functions_.end()) {
    fct = it->second;
  }

  // If not found and if we want to check the file (and have one).
  if (fct == nullptr) {
    // Then call it.
    if (check_file == true) {
      if (file_ != nullptr) {
        fct = file_->GetWasmFunction(name);
      }

      // If it does not exist in our module, let us create a prototype.
      if (fct != nullptr) {
        llvm::Function* llvm_fct = fct->GetFunction();
        llvm::FunctionType* ft = llvm_fct->getFunctionType();

        Function::Create(ft, llvm::Function::ExternalLinkage, name, module_);
      }
    }
  }

  return fct;
}

WasmFunction* WasmModule::GetWasmFunction(size_t idx) const {
  if (idx < vector_functions_.size()) {
    return vector_functions_[idx];
  }

  return nullptr;
}

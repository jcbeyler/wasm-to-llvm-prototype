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

#include "wasm_file.h"

void WasmFile::GenerateInitializeModules() {
  // Set the glue layer.
  llvm::Module* module = new llvm::Module("glue_wasm", llvm::getGlobalContext());
  glue_module_ = new WasmModule(module, nullptr, this);

  std::vector<llvm::Function*> fcts;

  for (auto module : modules_) {
    llvm::Function* fct = module->GetMemoryAllocator();

    if (fct != nullptr) {
      fcts.push_back(fct);
    }
  }

  // Create our entrance method: it is no argument, no return.
  std::vector<llvm::Type*> params;
  llvm::Type* result_type = llvm::Type::getVoidTy(llvm::getGlobalContext());
  llvm::FunctionType* fct_type = llvm::FunctionType::get(result_type, params, false);
  llvm::Function* fct = llvm::Function::Create(fct_type, Function::ExternalLinkage, "wasm_llvm_init", glue_module_->GetModule());

  // Generate a single basic block.
  llvm::BasicBlock* bb = llvm::BasicBlock::Create(getGlobalContext(), "entry", fct);
  llvm::IRBuilder<> builder(getGlobalContext());
  builder.SetInsertPoint(bb);

  // Now if we have some, we have work.
  if (fcts.size() > 0) {
    std::vector<Value*> args;
    for (auto f : fcts) {
      // Generate a prototype for it, they are also void/void so reuse the fct_type.
      llvm::Function::Create(fct_type, Function::ExternalLinkage, f->getName(), module);

      // Create the call.
      builder.CreateCall(f, args, "");
    }
  }

  builder.CreateRetVoid();
}

WasmFunction* WasmFile::GetWasmFunction(const char* name, unsigned int line) {
  // Return function if found.
  for (auto module : modules_) {
    // We only want modules that were defined above our own.
    unsigned int module_line = module->GetLine();

    if (module_line > line) {
      continue;
    }

    WasmFunction* fct = module->GetWasmFunction(name, false);

    if (fct != nullptr) {
      return fct;
    }
  }

  return nullptr;
}

void WasmFile::Initialize() {
  // First thing is go through each module and mangle all names.
  for (auto module : modules_) {
    module->MangleNames(this);
  }
}

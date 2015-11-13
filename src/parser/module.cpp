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

  // Push the argument type.
  if (type != VOID) {
    args.push_back(ConvertType(type));
  }

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
  // For each function, go from here to LLVM.
  for (auto it : functions_) {
    WasmFunction& fct = *it;

    fct.Generate();

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
      fct = GetWasmFunction(var->GetString(), false);
    } else {
      size_t idx = var->GetIdx();
      assert(idx < vector_functions_.size());
      fct = vector_functions_[idx];
    }

    if (fct == nullptr) {
      BISON_PRINT("Handling Export: Could not find %s\n", var->GetString());
    }
    assert(fct != nullptr);
    exported_functions[name] = fct;
  }

  // Clear the old map and the vector functions.
  map_functions_.clear();
  vector_functions_.clear();

  // Now we only have the exported methods.
  map_functions_ = exported_functions;
}

void WasmModule::Initialize() {
  // Make the module, which holds all the code.
  module_ = new llvm::Module(name_.c_str(), llvm::getGlobalContext());

  for (auto it : functions_) {
    map_functions_[it->GetName()] = it;
    vector_functions_.push_back(it);
  }

  // Create a new pass manager attached to it.
  fpm_ = new legacy::PassManager();

  llvm::PassManagerBuilder pmb;
  pmb.OptLevel = 1;
  pmb.populateModulePassManager(*fpm_);

  // Now generate the memory base.
  GenerateMemoryBaseFunction();

  // Generate the prototypes.
  for (auto it : functions_) {
    it->GeneratePrototype(this);
  }
}

std::string WasmModule::GetMemoryBaseFunctionName() const {
  std::ostringstream oss;
  oss << "set_" << name_ << "_memory_base";
  return oss.str();
}

std::string WasmModule::GetMemoryBaseName() const {
  std::ostringstream oss;
  oss << name_ << "_memory_base";
  return oss.str();
}

void WasmModule::GenerateMemoryBaseFunction() {
  if (memory_ != 0) {
    // This will work until threading is not available but it should work well.
    std::string name = GetMemoryBaseFunctionName();

    // Get base types.
    llvm::Type* char_type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
    llvm::Type* ptr_char_type = llvm::Type::getInt8PtrTy(llvm::getGlobalContext());

    // Create memory pointer.
    memory_pointer_ = new llvm::GlobalVariable(
        *module_,
        ptr_char_type,
        false,
        llvm::GlobalValue::CommonLinkage,
        ConstantPointerNull::get(char_type->getPointerTo()),
        GetMemoryBaseName().c_str(),
        nullptr,
        llvm::GlobalVariable::NotThreadLocal
        );

    // Now what we need is to create the method: it is a void fct(void) method.
    std::vector<llvm::Type*> params;
    llvm::Type* result_type = llvm::Type::getVoidTy(llvm::getGlobalContext());
    llvm::FunctionType* fct_type = llvm::FunctionType::get(result_type, params, false);
    memory_allocator_fct_ = llvm::Function::Create(fct_type, Function::ExternalLinkage, name.c_str(), GetModule());

    // Generate a single basic block.
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(getGlobalContext(), "entry", memory_allocator_fct_);
    llvm::IRBuilder<> builder(getGlobalContext());
    builder.SetInsertPoint(bb);

    // Now call malloc on it with the amount.
    llvm::Value* alloc_size = llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(32, memory_, false));
    llvm::Instruction* malloc_call = CallInst::CreateMalloc(builder.GetInsertBlock(),
        llvm::Type::getInt32Ty(llvm::getGlobalContext()),
        char_type, alloc_size, nullptr,
        nullptr, "malloc");
    builder.Insert(malloc_call, "calltmp");

    // Now set that global variable to the return of the malloc.
    builder.CreateStore(malloc_call, memory_pointer_, false);

    builder.CreateRetVoid();
  }
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

void WasmModule::RegisterMangle(const std::string& name, const std::string& hashed) {
  assert(map_hash_association_.find(name) == map_hash_association_.end());
  assert(map_reversed_hash_association_.find(hashed) == map_reversed_hash_association_.end());

  map_hash_association_[name] = hashed;
  map_reversed_hash_association_[hashed] = name;
}

WasmFunction* WasmModule::GetWasmFunction(const char* name, bool check_file, unsigned int line) const {
  WasmFunction* fct = InternalGetWasmFunction(name, check_file, line);

  if (fct == nullptr) {
    // Get the mangled name instead.
    auto iter = map_hash_association_.find(name);

    if (iter != map_hash_association_.end()) {
      fct = InternalGetWasmFunction(iter->second.c_str(), check_file, line);
    }

    if (fct == nullptr) {
      // Finally, it could be reversed that would solve it.
      auto iter = map_reversed_hash_association_.find(name);

      if (iter != map_reversed_hash_association_.end()) {
        fct = InternalGetWasmFunction(iter->second.c_str(), check_file, line);
      }
    }
  }

  return fct;
}

WasmFunction* WasmModule::InternalGetWasmFunction(const char* name, bool check_file, unsigned int line) const {
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
        fct = file_->GetWasmFunction(name, line);
      }

      // If it does not exist in our module, let us create a prototype.
      if (fct != nullptr) {
        llvm::Function* llvm_fct = fct->GetFunction();
        llvm::FunctionType* ft = llvm_fct->getFunctionType();

        Function::Create(ft, llvm::Function::ExternalLinkage, fct->GetName(), module_);
      }
    }
  }

  return fct;
}

WasmFunction* WasmModule::GetWasmFunction(const std::string& name, bool check_file, unsigned int line) const {
  WasmFunction* fct = GetWasmFunction(name.c_str(), check_file, line);
  return fct;
}

WasmFunction* WasmModule::GetWasmFunction(size_t idx) const {
  if (idx < vector_functions_.size()) {
    return vector_functions_[idx];
  }

  return nullptr;
}

void WasmModule::MangleNames(WasmFile* file) {
  // For each function, mangle its names.
  for (auto it : functions_) {
    WasmFunction& fct = *it;

    fct.MangleNames(file, this);
  }
}


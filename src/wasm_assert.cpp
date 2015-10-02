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
#include "wasm_assert.h"
#include "wasm_file.h"

void WasmAssertEq::Codegen(WasmFile* file) {
  // Now generate this function prototype: assert_eq is always the same:
    // Returns a boolean (is the invoke result equal to the expected one.

  // This method has no parameters.
  std::vector<llvm::Type*> params;

  // Then get the result: boolean.
  llvm::Type* result_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
  WasmModule* wasm_module = file->GetAssertModule();
  llvm::Module* module = wasm_module->GetModule();

  // Finally, create the function type.
  llvm::FunctionType* fct_type = llvm::FunctionType::get(result_type, params, false);
  llvm::Function* fct = llvm::Function::Create(fct_type, Function::ExternalLinkage, name_, module);

  // Now create the first bb.
  llvm::BasicBlock* bb = llvm::BasicBlock::Create(getGlobalContext(), "entry", fct);
  llvm::IRBuilder<> builder(getGlobalContext());
  builder.SetInsertPoint(bb);

  WasmFunction* wasm_fct = new WasmFunction(nullptr, fct->getName(), fct, wasm_module, INT_32);

  wasm_module->AddFunctionAndRegister(wasm_fct);

  expr_->Codegen(wasm_fct, builder);
}

void WasmAssertTrap::Codegen(WasmFile* file) {
  // TODO: this is different normally, we want to wrap it with a signal handler.
  // This method has no parameters.
  std::vector<llvm::Type*> params;

  // Then get the result: boolean.
  llvm::Type* result_type = llvm::Type::getInt8PtrTy(llvm::getGlobalContext());
  WasmModule* wasm_module = file->GetAssertModule();
  llvm::Module* module = wasm_module->GetModule();

  // Finally, create the function type.
  llvm::FunctionType* fct_type = llvm::FunctionType::get(result_type, params, false);
  llvm::Function* fct = llvm::Function::Create(fct_type, Function::ExternalLinkage, name_, module);

  // Now create the first bb.
  llvm::BasicBlock* bb = llvm::BasicBlock::Create(getGlobalContext(), "entry", fct);
  llvm::IRBuilder<> builder(getGlobalContext());
  builder.SetInsertPoint(bb);

  WasmFunction* wasm_fct = new WasmFunction(nullptr, fct->getName(), fct, wasm_module, PTR_32);

  wasm_module->AddFunctionAndRegister(wasm_fct);

  expr_->Codegen(wasm_fct, builder);
}

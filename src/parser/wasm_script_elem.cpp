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

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Intrinsics.h"

#include "binop.h"
#include "wasm_script_elem.h"
#include "wasm_file.h"

void WasmAssertReturnNan::Codegen(WasmFile* file) {
  // Just one thing before we do anything. Find out if the caller was 64-bit or not.
  ReturnExpression* return_expr = dynamic_cast<ReturnExpression*>(expr_);
  assert(return_expr != nullptr);

  IfExpression* if_expr = dynamic_cast<IfExpression*>(return_expr->GetResult());
  assert(if_expr!= nullptr);

  Binop* binop = dynamic_cast<Binop*>(if_expr->GetCondition());
  assert(binop != nullptr);

  CallExpression* call = dynamic_cast<CallExpression*>(binop->GetLeft());
  assert(call != nullptr);

  Variable* var = call->GetVariable();

  const char* name = var->GetString();

  // Get the function.
  WasmFunction* fct = file->GetWasmFunction(name);
  assert(fct != nullptr);

  ETYPE result = fct->GetResult();

  if (result == FLOAT_64) {
    // We need to just fix the right hand side of the comparison.
    ValueHolder* vh = new ValueHolder(std::numeric_limits<double>::quiet_NaN());

    Const* expr = new Const(FLOAT_64, vh);

    binop->SetRight(expr);
  }

  // Now generate the code...
  WasmAssertReturn::Codegen(file);
}

void WasmInvoke::Codegen(WasmFile* file) {
  // This method has no parameters.
  std::vector<llvm::Type*> params;

  // Then get the result: void.
  llvm::Type* result_type = llvm::Type::getVoidTy(llvm::getGlobalContext());
  WasmModule* wasm_module = file->GetAssertModule();
  llvm::Module* module = wasm_module->GetModule();

  // Finally, create the function type.
  llvm::FunctionType* fct_type = llvm::FunctionType::get(result_type, params, false);
  llvm::Function* fct = llvm::Function::Create(fct_type, Function::ExternalLinkage, mangled_name_, module);

  // Now create the first bb.
  llvm::BasicBlock* bb = llvm::BasicBlock::Create(getGlobalContext(), "entry", fct);
  llvm::IRBuilder<> builder(getGlobalContext());
  builder.SetInsertPoint(bb);

  WasmFunction* wasm_fct = new WasmFunction(nullptr, fct->getName(), fct, wasm_module, VOID);

  wasm_module->AddFunctionAndRegister(wasm_fct);

  expr_->Codegen(wasm_fct, builder);

  // Invoke from scripts have no return, so let's add a return void. Assertions below have that
  builder.CreateRetVoid();
}

void WasmAssertReturn::Codegen(WasmFile* file) {
  // Now generate this function prototype: assert_ereturn is always the same:
    // Returns an integer (-1 is success, the line number if failure)

  // This method has no parameters.
  std::vector<llvm::Type*> params;

  // Then get the result: boolean.
  llvm::Type* result_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
  WasmModule* wasm_module = file->GetAssertModule();
  llvm::Module* module = wasm_module->GetModule();

  // Finally, create the function type.
  llvm::FunctionType* fct_type = llvm::FunctionType::get(result_type, params, false);
  llvm::Function* fct = llvm::Function::Create(fct_type, Function::ExternalLinkage, mangled_name_, module);

  // Now create the first bb.
  llvm::BasicBlock* bb = llvm::BasicBlock::Create(getGlobalContext(), "entry", fct);
  llvm::IRBuilder<> builder(getGlobalContext());
  builder.SetInsertPoint(bb);

  WasmFunction* wasm_fct = new WasmFunction(nullptr, fct->getName(), fct, wasm_module, INT_32);

  wasm_module->AddFunctionAndRegister(wasm_fct);

  llvm::Value* value = expr_->Codegen(wasm_fct, builder);

  // If we have a value and it is not a terminator instruction, create the return.
  if (value != nullptr && dynamic_cast<TerminatorInst*>(value) == nullptr) {
    builder.CreateRet(value);
  }
}

void WasmAssertTrap::Codegen(WasmFile* file) {
  // TODO: this does not work: FPE will always trap and I don't see how to solve it right now.
  // This method has no parameters.
  std::vector<llvm::Type*> params;

  // Then get the result: boolean.
  llvm::Type* result_type = llvm::Type::getInt8PtrTy(llvm::getGlobalContext());
  WasmModule* wasm_module = file->GetAssertModule();
  llvm::Module* module = wasm_module->GetModule();

  // Finally, create the function type.
  llvm::FunctionType* fct_type = llvm::FunctionType::get(result_type, params, false);
  llvm::Function* fct = llvm::Function::Create(fct_type, Function::ExternalLinkage, mangled_name_, module);

  // Now create the first bb.
  llvm::BasicBlock* bb = llvm::BasicBlock::Create(getGlobalContext(), "entry", fct);
  llvm::IRBuilder<> builder(getGlobalContext());
  builder.SetInsertPoint(bb);

  WasmFunction* wasm_fct = new WasmFunction(nullptr, fct->getName(), fct, wasm_module, PTR_32);

  wasm_module->AddFunctionAndRegister(wasm_fct);

  llvm::Value* res = expr_->Codegen(wasm_fct, builder);
  builder.CreateRet(res);
}

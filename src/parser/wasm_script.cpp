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
#include "binop.h"
#include "wasm_script.h"
#include "wasm_file.h"

static CallExpression* HandleInvoke(WasmModule* wasm_module, WasmScriptElem* elem) {
  const std::string& name = elem->GetName();

  // Generate the call.
  Variable* id = new Variable(name.c_str());

  // We also want the line number for this call.
  CallExpression* call = new CallExpression(id);
  call->SetLine(elem->GetLine());

  return call;
}

static CallExpression* HandleAssert(WasmModule* wasm_module, WasmScriptElem* elem) {
  CallExpression* call = nullptr;

  // Depending on the type of the assert type, we either call it directly
  //   or go via a handler and pass the wasm_assert method.
  if (dynamic_cast<WasmAssertReturn*>(elem) != nullptr) {
    // In this case, we can handle it as if it's just an invoke.
    return HandleInvoke(wasm_module, elem);
  } else {
    // In the assert_trap case, we want to actually call the assert_trap_handler method.
    // Generate the call.
    Variable* id = new Variable("assert_trap_handler");
    std::list<Expression*>* params = new std::list<Expression*>();

    // Let's make sure we declare it at least once.
    wasm_module->GetWasmAssertTrapFunction();

    // Get a pointer to the wasm_assert function we are interested.
    WasmFunction* wasm_fct = wasm_module->GetWasmFunction(elem->GetMangledName().c_str(), true);
    assert(wasm_fct != nullptr);

    // Hold this in a ValueExpression.
    ValueExpression* value = new ValueExpression(wasm_fct->GetFunction());

    params->push_back(value);
    call = new CallExpression(id, params);
  }

  assert(call != nullptr);
  return call;
}

void WasmScript::GenerateGeneralScriptCalls(WasmFile* file) {
  // Now generate this function prototype: execute_script:
    // Returns a integer is the result of all script.
    //  If successful, returns -1.
    //  It will fail otherwise and return the line number of the failure.

  // This method has no parameters.
  std::vector<llvm::Type*> params;

  // Then get the result: integer.
  llvm::Type* result_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
  WasmModule* wasm_module = file->GetAssertModule();
  llvm::Module* module = wasm_module->GetModule();

  // Finally, create the function type.
  llvm::FunctionType* fct_type = llvm::FunctionType::get(result_type, params, false);
  const char* name = "execute_script";
  llvm::Function* fct = llvm::Function::Create(fct_type, Function::ExternalLinkage, name, module);

  // Now create the first bb.
  llvm::BasicBlock* bb = llvm::BasicBlock::Create(getGlobalContext(), "entry", fct);
  llvm::IRBuilder<> builder(getGlobalContext());
  builder.SetInsertPoint(bb);

  WasmFunction* wasm_fct = new WasmFunction(nullptr, name, fct, wasm_module, INT_32);
  const char* result_name = "result";
  Variable* result = new Variable(result_name);
  wasm_fct->Allocate(result_name,
                         llvm::Type::getInt32Ty(llvm::getGlobalContext()),
                         builder);

  // Now generate our IR and then use our codegen for it.
  for (auto elem : script_elems_) {
    Expression* expr = nullptr;

    if (dynamic_cast<WasmInvoke*>(elem) != nullptr) {
      expr = HandleInvoke(wasm_module, elem);
    } else {
      CallExpression* call = HandleAssert(wasm_module, elem);

      // Set the value in a local.
      SetLocal* set = new SetLocal(result, call);

      // In the assert_return case, we want to compare this to -1.
      Const* minus_one = new Const(INT_32, new ValueHolder(-1));
      Operation* op = new Operation(NE_OPER, INT_32);
      Binop* cmp = new Binop(op, set, minus_one);

      // Now we can generate the return 0;
      GetLocal* get = new GetLocal(result);
      ReturnExpression* return_expr = new ReturnExpression(get);

      // Finally, generate the AST for this assert.
      IfExpression* inst = new IfExpression(cmp, return_expr);

      expr = inst;
    }

    // Now we can generate it.
    expr->Codegen(wasm_fct, builder);
  }

  Const* one = new Const(INT_32,
      new ValueHolder(-1));
  ReturnExpression* return_expr = new ReturnExpression(one);

  return_expr->Codegen(wasm_fct, builder);
}

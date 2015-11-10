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

#include "expression.h"
#include "function.h"
#include "module.h"

llvm::Value* GetLocal::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  llvm::AllocaInst* alloca = nullptr;
  const char* name = nullptr;

  if (var_->IsString()) {
    name = var_->GetString();
    alloca = fct->GetVariable(name);
  } else {
    size_t idx = var_->GetIdx();
    std::ostringstream oss;
    oss << "var_idx_" << idx;
    name = oss.str().c_str();
    alloca = fct->GetVariable(idx);
  }

  assert(alloca != nullptr);

  llvm::Value* res = builder.CreateLoad(alloca->getAllocatedType(), alloca, name);
  return res;
}

llvm::Value* SetLocal::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // First generate the value code.
  llvm::Value* value = value_->Codegen(fct, builder);

  // Now call the right method to set it.
  if (var_->IsString()) {
    const char* name = var_->GetString();
    builder.CreateStore(value, fct->GetVariable(name));
  } else {
    size_t idx = var_->GetIdx();
    builder.CreateStore(value, fct->GetVariable(idx));
  }

  // Return the value.
  return value;
}

llvm::Value* Const::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  switch (type_) {
    case FLOAT_32: {
      float f = value_->GetFloat();
      return llvm::ConstantFP::get(llvm::getGlobalContext(), APFloat(f));
    }
    case FLOAT_64:
      return llvm::ConstantFP::get(llvm::getGlobalContext(), APFloat(value_->GetDouble()));
    case INT_32: {
      int val = value_->GetInteger();
      return llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(32, val, false));
    }
    case INT_64: {
      int64_t val = value_->GetInteger();
      return llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(64, val, false));
    }
    default:
      assert(0);
      break;
  }
  return nullptr;
}

WasmFunction* CallExpression::GetCallee(WasmFunction* fct) const {
  WasmModule* module = fct->GetModule();
  WasmFunction* wfct = nullptr;

  if (call_id_->IsString()) {
    const char* name = call_id_->GetString();

    // Line number is needed here in case we are going outside of the module.
    wfct = module->GetWasmFunction(name, true, line_);
  } else {
    size_t idx = call_id_->GetIdx();
    wfct  = module->GetWasmFunction(idx);
  }

  if (wfct == nullptr) {
    BISON_PRINT("Problem with finding %s\n", call_id_->GetString());
  }
  assert(wfct != nullptr);

  return wfct;
}

llvm::Value* CallExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  WasmFunction* wfct = GetCallee(fct);
  llvm::Function* callee = wfct->GetFunction();
  assert(callee != nullptr);

  // Now create the arguments for the call creation.
  std::vector<Value*> args;

  if (params_ != nullptr) {
    for (auto elem : *params_) {
      args.push_back(elem->Codegen(fct, builder));
    }
  }

  const char* return_name = "";

  if (wfct->GetResult() != VOID) {
    return_name = "calltmp";
  }

  return builder.CreateCall(callee, args, return_name);
}

llvm::Value* IfExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // Start by generating the condition.
  llvm::Value* cond_value = cond_->Codegen(fct, builder);

  llvm::Function* llvm_fct = fct->GetFunction();

  // Create the blocks for true/false/end.
  llvm::BasicBlock* true_bb = nullptr;
  llvm::BasicBlock* false_bb = nullptr;
  llvm::BasicBlock* end_bb = nullptr;

  // We should have at least a true_cond_.
  assert(true_cond_ != nullptr);

  // Add it automatically to the function.
  true_bb = BasicBlock::Create(llvm::getGlobalContext(), "true", llvm_fct);

  // The other two will wait before being emitted.
  false_bb = BasicBlock::Create(llvm::getGlobalContext(), "false");
  end_bb = BasicBlock::Create(llvm::getGlobalContext(), "end");

  builder.CreateCondBr(cond_value, true_bb, false_bb);

  // Start generating the if.
  builder.SetInsertPoint(true_bb);
  Value* true_result = true_cond_->Codegen(fct, builder);

  // If we do not finish with a terminator, generate a jump.
  if (dynamic_cast<TerminatorInst*>(true_result) == nullptr) {
    // Branch now to the end_bb.
    builder.CreateBr(end_bb);
  }

  // Come back to the true_bb.
  true_bb = builder.GetInsertBlock();

  // Handle false side.
  // First emit it.
  llvm_fct->getBasicBlockList().push_back(false_bb);
  builder.SetInsertPoint(false_bb);

  // We might not have one.
  Value* false_result = nullptr;
  if (false_cond_ != nullptr) {
    false_result = false_cond_->Codegen(fct, builder);
  }

  // Branch now to the end_bb.
  builder.CreateBr(end_bb);
  // Come back to the true_bb.
  false_bb = builder.GetInsertBlock();

  // Finally handle the merge point.
  // First emit it.
  llvm_fct->getBasicBlockList().push_back(end_bb);
  builder.SetInsertPoint(end_bb);

  // Result is the true_result except if there is an else.
  Value* result = true_result;
  if (false_result != nullptr) {
    // Now add a phi node for both sides. And that will be the result.

    // But first what type?
    llvm::Type* merge_type = true_result->getType();

    // TODO: this is not enough even... but it should bring me back here fast...
    assert(merge_type == false_result->getType());

    PHINode* merge_phi = builder.CreatePHI(merge_type, 2, "iftmp");

    merge_phi->addIncoming(true_result, true_bb);
    merge_phi->addIncoming(false_result, false_bb);

    result = merge_phi;
  }

  return result;
}

llvm::Value* LabelExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  const char* name = "end_label";

  if (var_ != nullptr) {
    // Get the string name, whether integer or not.
    name = var_->GetString();
  }

  // For now, just ignore it for code generation.
  llvm::BasicBlock* end_label = BasicBlock::Create(llvm::getGlobalContext(), name);

  fct->PushLabel(end_label);

  // Generate the code now.
  expr_->Codegen(fct, builder);

  // Now jump to the end_label.
  builder.CreateBr(end_label);

  // Now add the block and set it as insert point.
  llvm::Function* llvm_fct = fct->GetFunction();
  llvm_fct->getBasicBlockList().push_back(end_label);

  // Now set ourselves to the end of the label.
  builder.SetInsertPoint(end_label);

  // Finally pop the label.
  fct->PopLabel();
}

llvm::Value* LoopExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // For now, just ignore it for code generation.
  llvm::BasicBlock* preheader = builder.GetInsertBlock();

  // The other two will wait before being emitted.
  const char* name = (var_ != nullptr) ? var_->GetString() : "loop_block";
  llvm::BasicBlock* loop = BasicBlock::Create(llvm::getGlobalContext(), name, fct->GetFunction());

  // Push it.
  fct->PushLabel(loop);

  // Jump to the loop.
  builder.CreateBr(loop);

  // Now we are in the loop.
  builder.SetInsertPoint(loop);

  llvm::Value* value = nullptr;
  for(std::list<Expression*>::iterator it = loop_->begin();
      it != loop_->end();
      it++) {
    Expression* expr = *it;
    value = expr->Codegen(fct, builder);
  }

  // Create the unconditional branch back to the start.
  builder.CreateBr(loop);

  // Create a new block, it will be dead code but it will let LLVM land on its feet.
  BasicBlock* label_dc = BasicBlock::Create(llvm::getGlobalContext(), "label_dc ", fct->GetFunction());
  builder.SetInsertPoint(label_dc);

  fct->PopLabel();

  // All is good :).
  return value;
}

llvm::Value* BlockExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // For now, there is no reason to really care about block.
  for (std::list<Expression*>::const_iterator it = list_->begin(); it != list_->end(); it++) {
    Expression* expr = *it;
    expr->Codegen(fct, builder);
  }

  return nullptr;
}

llvm::Value* BreakExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // TODO.
  assert(expr_ == nullptr);

  llvm::BasicBlock* bb = nullptr;

  if (var_->IsString() == false) {
    size_t idx = var_->GetIdx();
    bb = fct->GetLabel(idx);
  } else {
    const char* name = var_->GetString();
    bb = fct->GetLabel(name);
  }

  assert(bb != nullptr);

  // We just want a jump to that block.
  builder.CreateBr(bb);

  // Create a new block, it will be dead code but it will let LLVM land on its feet.
  BasicBlock* break_dc  = BasicBlock::Create(llvm::getGlobalContext(), "break_dc", fct->GetFunction());
  builder.SetInsertPoint(break_dc);

  return nullptr;
}

llvm::Value* ReturnExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // Generate the code for the return, then call the handler.
  llvm::Value* result = result_->Codegen(fct, builder);
  assert(result != nullptr);
  fct->HandleReturn(result, builder);
}

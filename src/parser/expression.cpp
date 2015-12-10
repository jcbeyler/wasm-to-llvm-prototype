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

  llvm::Value* res = builder.CreateLoad(alloca->getAllocatedType(), alloca, "get_local");
  return res;
}

llvm::Value* SetLocal::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // First generate the value code.
  llvm::Value* value = value_->Codegen(fct, builder);
  assert(value != nullptr);

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
    case INT_1: {
      int val = value_->GetInteger();
      return llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(1, val, false));
    }
    case INT_8: {
      int64_t val = value_->GetInteger();
      return llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(8, val, false));
    }
    case INT_16: {
      int val = value_->GetInteger();
      return llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(16, val, false));
    }
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

  (void) fct;
  (void) builder;

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
  assert(wfct != nullptr);
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

llvm::Value* CallImportExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  WasmModule* module = fct->GetModule();
  WasmImportFunction* wif = nullptr;

  if (call_id_->IsString()) {
    const char* name = call_id_->GetString();

    // Line number is needed here in case we are going outside of the module.
    wif = module->GetWasmImportFunction(name, true, line_);
  } else {
    size_t idx = call_id_->GetIdx();
    wif  = module->GetWasmImportFunction(idx);
  }

  assert(wif != nullptr);

  llvm::Function* callee = wif->GetFunction(module);
  assert(callee != nullptr);

  std::vector<Value*> args;
  if (params_ != nullptr) {
    for (auto elem : *params_) {
      args.push_back(elem->Codegen(fct, builder));
    }
  }

  const char* return_name = "";
  if (wif->GetResult() != VOID) {
    return_name = "calltmp";
  }

  return builder.CreateCall(callee, args, return_name);
}

llvm::Value* ConditionalExpression::TransformCondition(llvm::Value* value, llvm::IRBuilder<>& builder) {
  llvm::Type* type = value->getType();

  if (type->isIntegerTy(1) == false) {
    if (type->isFloatingPointTy()) {
      llvm::Value* zero = llvm::ConstantFP::get(llvm::getGlobalContext(), APFloat(0.0));;
      // Not sure ordered is what we want but let us assume for now.
      return builder.CreateFCmpONE(value, zero, "cmp_zero");
    } else {
      llvm::Value* zero = llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(type->getIntegerBitWidth(), 0, false));
      return builder.CreateICmpNE(value, zero, "cmp_zero");
    }
  }

  // Nothing to be done.
  return value;
}

llvm::Value* IfExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // Start by generating the condition.
  llvm::Value* cond_value = cond_->Codegen(fct, builder);
  cond_value = TransformCondition(cond_value, builder);

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
  end_bb = BasicBlock::Create(llvm::getGlobalContext(), "end_if");

  builder.CreateCondBr(cond_value, true_bb, false_bb);

  // Start generating the true side.
  builder.SetInsertPoint(true_bb);
  llvm::Value* true_result = true_cond_->Codegen(fct, builder);

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
  llvm::Value* false_result = nullptr;
  if (false_cond_ != nullptr) {
    false_result = false_cond_->Codegen(fct, builder);
  }

  // Branch now to the end_bb.
  builder.CreateBr(end_bb);

  // Come back to the false_bb.
  false_bb = builder.GetInsertBlock();

  // Finally handle the merge point.
  // First emit it.
  llvm_fct->getBasicBlockList().push_back(end_bb);
  builder.SetInsertPoint(end_bb);

  // Result is the true_result except if there is an else.
  Value* result = true_result;

  if (true_result == nullptr || (dynamic_cast<TerminatorInst*>(true_result) != nullptr)) {
    result = false_result;
  } else {
    if (should_merge_ == true && false_result != nullptr) {
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

  fct->PushLabel(name, end_label);
  fct->RegisterNamedExpression(end_label, this);

  // Generate the code now.
  llvm::Value* res = expr_->Codegen(fct, builder);

  if (res != nullptr) {
    AddIncomingPhi(res, builder.GetInsertBlock());
  }

  // Now add the block and set it as insert point.
  llvm::Function* llvm_fct = fct->GetFunction();
  llvm_fct->getBasicBlockList().push_back(end_label);

  // Now jump to the end_label.
  builder.CreateBr(end_label);

  // Now set ourselves to the end of the label.
  builder.SetInsertPoint(end_label);

  res = HandlePhiNodes(builder);

  // Finally pop the label.
  fct->PopLabel();

  return res;
}

llvm::Value* LoopExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // The other two will wait before being emitted.
  const char* name = (var_ != nullptr) ? var_->GetString() : "loop_block";
  llvm::BasicBlock* loop = BasicBlock::Create(llvm::getGlobalContext(), name, fct->GetFunction());

  const char* exit_name = (exit_name_ != nullptr) ? exit_name_->GetString() : "loop_exit_block";
  llvm::BasicBlock* exit_block = BasicBlock::Create(llvm::getGlobalContext(), exit_name, fct->GetFunction());

  // Push it.
  fct->PushLabel(exit_name, exit_block);
  fct->PushLabel(name, loop);

  // Also register for the function level that this loop is this exit block.
  fct->RegisterNamedExpression(exit_block, this);

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

  // If last node from the loop is not nullptr, register it.
  if (value != nullptr) {
    // If the last value is not a terminator.
    if (dynamic_cast<TerminatorInst*>(value) == nullptr) {
      builder.CreateBr(exit_block);
      AddIncomingPhi(value, builder.GetInsertBlock());
    }
  }

  // Create a new block, it will be dead code but it will let LLVM land on its feet.
  builder.SetInsertPoint(exit_block);

  value = HandlePhiNodes(builder);

  // Pop the labels.
  fct->PopLabel();
  fct->PopLabel();

  // All is good :).
  return value;
}

void NamedExpression::AddIncomingPhi(llvm::Value* value, llvm::BasicBlock* bb) {
  assert(value != nullptr);
  if (dynamic_cast<TerminatorInst*>(value) == nullptr) {
    incoming_phis_.push_back(std::make_pair(value, bb));
  }
}

llvm::Value* NamedExpression::HandlePhiNodes(llvm::IRBuilder<>& builder) const {
  llvm::Value* value = nullptr;

  // Now handle the phi nodes if we have any.
  size_t size = incoming_phis_.size();
  if (size > 1) {
    // Let us assume the merge type is fine for all elements.
    llvm::Type* merge_type = incoming_phis_[0].first->getType();

    PHINode* merge_phi = builder.CreatePHI(merge_type, size, "merge_phi");

    for (auto p : incoming_phis_) {
      merge_phi->addIncoming(p.first, p.second);
    }

    value = merge_phi;
  } else {
    // If only one element, just return it.
    if (size == 1) {
      return incoming_phis_[0].first;
    }
  }

  return value;
}

llvm::Value* BlockExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  llvm::Value* res = nullptr;

  BasicBlock* block_code  = BasicBlock::Create(llvm::getGlobalContext(), "block", fct->GetFunction());

  // Create the exit block.
  const char* name = name_ ? name_ : "unamed_exit_block";
  BasicBlock* exit_block_code  = BasicBlock::Create(llvm::getGlobalContext(), name, fct->GetFunction());

  builder.CreateBr(block_code);

  builder.SetInsertPoint(block_code);

  // Push it.
  fct->PushLabel(name, exit_block_code);

  // Also register for the function level that this loop is this exit block.
  fct->RegisterNamedExpression(exit_block_code, this);

  // For now, there is no reason to really care about block.
  bool finished_with_termination = false;
  for (std::list<Expression*>::const_iterator it = list_->begin(); it != list_->end(); it++) {
    Expression* expr = *it;
    res = expr->Codegen(fct, builder);

    // Stop if we are jumping or returning.
    if ((dynamic_cast<ReturnExpression*>(expr) != nullptr) ||
        (dynamic_cast<BreakExpression*>(expr) != nullptr)) {
      finished_with_termination = true;
      break;
    }
  }

  // If last node from the block is not nullptr, register it.
  if (res != nullptr) {
    AddIncomingPhi(res, builder.GetInsertBlock());
  }

  // Pop it.
  fct->PopLabel();

  if (finished_with_termination == false) {
    builder.CreateBr(exit_block_code);
  } else {
    // In the case we did finish with termination, we generated the exit block but there is a possibility it is not used.
    //  However it must finish with a terminator outside and we don't know if we are going towards it.

    // So let's find out if someone is going there.
    // TODO.
  }

  builder.SetInsertPoint(exit_block_code);

  res = HandlePhiNodes(builder);

  return res;
}

llvm::Value* BreakIfExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  llvm::BasicBlock* bb = nullptr;

  if (var_->IsString() == false) {
    size_t idx = var_->GetIdx();
    bb = fct->GetLabel(idx);
  } else {
    const char* name = var_->GetString();
    bb = fct->GetLabel(name);
  }

  assert(bb != nullptr);

  // First generate the expr if there.
  llvm::Value* result = nullptr;
  if (expr_ != nullptr) {
    result = expr_->Codegen(fct, builder);
  }

  // Second generate the cond if there.
  llvm::Value* cond = nullptr;
  if (cond_ != nullptr) {
    cond = cond_->Codegen(fct, builder);
    cond = TransformCondition(cond, builder);
  }

  // Now generate the if:
  llvm::BasicBlock* true_bb = nullptr;
  llvm::BasicBlock* false_bb = nullptr;

  // Get the llvm function.
  llvm::Function* llvm_fct = fct->GetFunction();

  // Add it automatically to the function.
  true_bb = BasicBlock::Create(llvm::getGlobalContext(), "true", llvm_fct);

  // The false side will wait before being emitted.
  false_bb = BasicBlock::Create(llvm::getGlobalContext(), "false");

  builder.CreateCondBr(cond, true_bb, false_bb);

  // Generate the break.
  builder.SetInsertPoint(true_bb);

  if (result != nullptr) {
    // We should push this to the named expression so that it knows about it.
    NamedExpression* named = fct->FindNamedExpression(bb);

    // If we did not find it, we will not push the information there, it is not going to a merge point...
    if (named != nullptr) {
      named->AddIncomingPhi(result, true_bb);
    }
  }

  builder.CreateBr(bb);

  assert(cond != nullptr);

  // Now set ourselves in the false side and continue our route.
  llvm_fct->getBasicBlockList().push_back(false_bb);
  builder.SetInsertPoint(false_bb);

  // No return in this case.
  return nullptr;
}

llvm::Value* BreakExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  llvm::BasicBlock* bb = nullptr;

  if (var_->IsString() == false) {
    size_t idx = var_->GetIdx();
    bb = fct->GetLabel(idx);
  } else {
    const char* name = var_->GetString();
    bb = fct->GetLabel(name);
  }

  assert(bb != nullptr);

  if (expr_ != nullptr) {
    llvm::Value* result = expr_->Codegen(fct, builder);

    if (result != nullptr) {
      // We should push this to the named expression so that it knows about it.
      NamedExpression* named = fct->FindNamedExpression(bb);

      // If we did not find it, we will not push the information there, it is not going to a merge point...
      if (named != nullptr) {
        named->AddIncomingPhi(result, builder.GetInsertBlock());
      }
    }
  }

  // We just want a jump to that block.
  return builder.CreateBr(bb);
}

llvm::Value* ReturnExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // Generate the code for the return, then call the handler.
  llvm::Value* result = result_->Codegen(fct, builder);
  assert(result != nullptr);
  return fct->HandleReturn(result, builder);
}

llvm::Value* Unreachable::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  llvm::Intrinsic::ID intrinsic = llvm::Intrinsic::trap;
  WasmModule* wasm_module = fct->GetModule();
  llvm::Function* intrinsic_fct = wasm_module->GetOrCreateIntrinsic(intrinsic);
  std::vector<Value*> no_arg;
  return builder.CreateCall(intrinsic_fct, no_arg);
}

llvm::Value* SelectExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // First generate the condition, first, and second.
  llvm::Value* cond = cond_->Codegen(fct, builder);

  // We need to handle the unreachable case here because the LLVM builder
  //   won't like us passing a trap call...
  // This is not perfect and might need more work later down the road.
  if (dynamic_cast<Unreachable*>(cond_) != nullptr) {
    return cond;
  }

  llvm::Value* first = first_->Codegen(fct, builder);

  if (dynamic_cast<Unreachable*>(first_) != nullptr) {
    return first;
  }

  llvm::Value* second = second_->Codegen(fct, builder);

  if (dynamic_cast<Unreachable*>(second_) != nullptr) {
    return second;
  }

  cond = TransformCondition(cond, builder);

  return builder.CreateSelect(cond, first, second, "select");
}

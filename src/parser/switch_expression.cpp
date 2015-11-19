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
#include "switch_expression.h"

llvm::Value* CaseExpression::Codegen(llvm::Value* last, SwitchExpression* switch_expr, WasmFunction* fct, llvm::IRBuilder<>& builder, bool is_first) {
  // Create a BasicBlock name.
  std::string s_name = "case_";
  s_name += id_;

  // Now create the block.
  llvm::BasicBlock* case_block = BasicBlock::Create(llvm::getGlobalContext(), s_name.c_str(), fct->GetFunction());

  if (is_first == false) {
    if (last == nullptr || (dynamic_cast<TerminatorInst*>(last) == nullptr)) {
      builder.CreateBr(case_block);
    }
  }

  // Register the block.
  switch_expr->RegisterGeneratedCase(id_, case_block);

  // Now generate the code.
  builder.SetInsertPoint(case_block);

  llvm::Value* res = nullptr;
  for (auto expr : *list_) {
    res = expr->Codegen(fct, builder);
  }

  return res;
}

llvm::BasicBlock* SwitchExpression::HandleDefault(std::map<std::string, llvm::BasicBlock*>& association, WasmFunction* fct, llvm::IRBuilder<>& builder) const {
  // So the default block can be defined by a case node or an expression node.

  VariableCaseDefinition* variable_def = dynamic_cast<VariableCaseDefinition*>(default_);

  if (variable_def != nullptr) {
    // Then return the pointer from the association, it should have been created.
    return association[variable_def->GetString()];
  }

  // Otherwise we have a ExpressionCaseDefinition.
  ExpressionCaseDefinition* expr_case = dynamic_cast<ExpressionCaseDefinition*>(default_);
  assert(expr_case != nullptr);

  Expression* expr = expr_case->GetExpression();

  // Create a block for it.
  BasicBlock* default_code  = BasicBlock::Create(llvm::getGlobalContext(), "default", fct->GetFunction());
  builder.SetInsertPoint(default_code);

  // Now generate the code.
  expr->Codegen(fct, builder);

  // Now return the block.
  return default_code;
}

std::string SwitchExpression::HandleExpressionCase(ExpressionCaseDefinition* expr,
                                                   std::map<llvm::BasicBlock*, llvm::Value*>& values,
                                                   std::map<std::string, llvm::BasicBlock*>& association,
                                                   WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // Create a CaseExpression and generate the code like that.
  //  First step: find an unused name.
  std::string name = "anonymous_case";
  int idx = 0;
  std::string final_name;

  while (1) {
    std::ostringstream oss;
    oss << name << "_" << idx;
    final_name = oss.str();

    if (association.find(final_name) == association.end()) {
      break;
    }

    idx++;
  }

  // Create and generate the CaseExpression.
  std::list<Expression*> list;
  list.push_back(expr->GetExpression());
  CaseExpression case_expr(final_name.c_str(), &list);
  llvm::Value* value = case_expr.Codegen(nullptr, this, fct, builder, true);

  // Register it.
  llvm::BasicBlock* bb = builder.GetInsertBlock();
  std::string res = bb->getName();
  association[res] = bb;

  // And the value.
  values[bb] = value;

  return res;
}

llvm::Value* SwitchExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  std::vector<BasicBlock*> case_blocks;
  const char* name = name_ != nullptr ? name_ : "switch_exit";

  llvm::BasicBlock* switch_block = builder.GetInsertBlock();

  llvm::BasicBlock* exit_block = BasicBlock::Create(llvm::getGlobalContext(), name, fct->GetFunction());

  // Push it.
  fct->PushLabel(exit_block);
  fct->RegisterNamedExpression(exit_block, this);

  // Start by generating the cases.
  llvm::Value* last_result = nullptr;
  llvm::BasicBlock* default_block = nullptr;
  std::map<std::string, llvm::BasicBlock*> association;
  std::map<llvm::BasicBlock*, llvm::Value*> values;
  std::vector<llvm::BasicBlock*> case_vector;

  // First generate the cases we know about.
  if (cases_->size() > 0) {
    bool is_first = true;
    for (auto elem : *cases_) {
      llvm::Value* value = elem->Codegen(last_result, this, fct, builder, is_first);
      is_first = false;

      llvm::BasicBlock* bb = builder.GetInsertBlock();
      values[bb] = value;
      association[elem->GetIdentifier()] = bb;
      case_vector.push_back(bb);

      last_result = value;
    }

    // For the last case, we fall out the switch.
    if (last_result == nullptr || (dynamic_cast<TerminatorInst*>(last_result) == nullptr)) {
      builder.CreateBr(exit_block);

      if (last_result != nullptr) {
        AddIncomingPhi(last_result, builder.GetInsertBlock());
      }
    }
  }

  // Now we might have some expressions in the middle of the table definition.
  for (std::list<CaseDefinition*>::iterator index_it = index_table_->begin();
                                            index_it != index_table_->end();
                                            index_it++) {
    ExpressionCaseDefinition* expr = dynamic_cast<ExpressionCaseDefinition*>(*index_it);

    if (expr != nullptr) {
      // Ok we have this block to generate.
      std::string name = HandleExpressionCase(expr, values, association, fct, builder);

      // Finally, replace this ExpressionCaseDefinition with a VariableCaseDefinition
      Variable* var = new Variable(name.c_str());
      VariableCaseDefinition* var_case = new VariableCaseDefinition(var);
      *index_it = var_case;
    }
  }

  // Get the default one.
  default_block = HandleDefault(association, fct, builder);

  // Pop label.
  fct->PopLabel();

  assert(default_block != nullptr);

  // Set ourselves back to the switch block right now.
  builder.SetInsertPoint(switch_block);

  // Create the switch.
  llvm::Value* value = selector_->Codegen(fct, builder);

  SwitchInst* switch_inst = builder.CreateSwitch(value, default_block, cases_->size());

  // Now add the cases.
  int i = 0;

  // Is it defined by the index_table?
  llvm::BasicBlock* last_bb = nullptr;
  bool found_default = false;

  if (index_table_->size() != 0) {
    for (std::list<CaseDefinition*>::const_iterator index_it = index_table_->begin();
        index_it != index_table_->end();
        index_it++) {
      llvm::ConstantInt* case_value = llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(32, i, false));

      VariableCaseDefinition* var = dynamic_cast<VariableCaseDefinition*>(*index_it);
      assert(var != nullptr);

      llvm::BasicBlock* bb = association[var->GetString()];

      assert(bb != nullptr);

      switch_inst->addCase(case_value, bb);

      // Remember the basic block for next iteration.
      i++;
    }
  } else {
    for (auto elem : case_vector) {
      llvm::ConstantInt* case_value = llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(32, i, false));

      // Before adding the case, let us link them together if need be.
      if (last_bb != nullptr) {
        llvm::TerminatorInst* term = last_bb->getTerminator();
        if (term == nullptr) {
          builder.SetInsertPoint(last_bb);
          builder.CreateBr(elem);
        }
      }

      switch_inst->addCase(case_value, elem);

      // Remember the basic block for next iteration.
      last_bb = elem;
      i++;
    }
  }

  // Now set ourselves to the exit_block.
  builder.SetInsertPoint(exit_block);

  llvm::Value* res = HandlePhiNodes(builder);
  return res;
}

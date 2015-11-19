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

#ifndef H_SWITCH_EXPRESSION
#define H_SWITCH_EXPRESSION

#include <list>

#include "base_expression.h"

class CaseDefinition {
  public:
    virtual void Dump() {
    }
};

class VariableCaseDefinition : public CaseDefinition {
  protected:
    Variable* var_;

  public:
    VariableCaseDefinition(Variable* var) : var_(var) {
    }

    virtual void Dump() {
    }

    const char* GetString() const {
      return var_->GetString();
    }
};

class ExpressionCaseDefinition : public CaseDefinition {
  protected:
    Expression* expr_;

  public:
    virtual void Dump() {
    }

    ExpressionCaseDefinition(Expression* expr) : expr_(expr) {
    }

    Expression* GetExpression() const {
      return expr_;
    }
};

class CaseExpression : public Expression {
  protected:
    const char* id_;
    std::list<Expression*>* list_;

  public:
    CaseExpression(const char* id, std::list<Expression*>* list) :
      id_(id), list_(list) {
    }

    const char* GetIdentifier() const {
      return id_;
    }

    llvm::Value* Codegen(llvm::Value* last, SwitchExpression* switch_expr, WasmFunction* fct, llvm::IRBuilder<>& builder, bool is_first);
};

class SwitchExpression : public Expression, public NamedExpression {
  protected:
    const char* name_;
    Expression* selector_;
    std::list<CaseDefinition*>* index_table_;
    CaseDefinition* default_;
    std::list<CaseExpression*>* cases_;
    std::map<std::string, llvm::BasicBlock*> generated_cases_;

    llvm::BasicBlock* HandleDefault(std::map<std::string, llvm::BasicBlock*>& association,
                                    WasmFunction* fct, llvm::IRBuilder<>& builder) const;

    std::string HandleExpressionCase(ExpressionCaseDefinition* expr,
                                     std::map<llvm::BasicBlock*, llvm::Value*>& values,
                                     std::map<std::string, llvm::BasicBlock*>& association,
                                     WasmFunction* fct, llvm::IRBuilder<>& builder);

  public:
    SwitchExpression(const char* name, Expression* selector,
                     std::list<CaseDefinition*>* index_table,
                     CaseDefinition* default_case,
                     std::list<CaseExpression*>* cases) :
                     name_(name), selector_(selector), index_table_(index_table),
                     default_(default_case), cases_(cases) {
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);

    void RegisterGeneratedCase(const char* name, llvm::BasicBlock* bb) {
      generated_cases_[name] = bb;
    }
};
#endif

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
#ifndef H_BINOP
#define H_BINOP

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Intrinsics.h"

#include "base_expression.h"
#include "enums.h"
#include "operation.h"
#include "simple.h"

class Binop : public Expression {
  protected:
    Operation* operation_;
    Expression *left_, *right_;

    ETYPE HandleType(llvm::Type* lt, llvm::Type* rt);
    llvm::Value* HandleInteger(llvm::Value* lv, llvm::Value* rv, llvm::IRBuilder<>& builder);
    llvm::Value* HandleShift(WasmFunction* fct, llvm::IRBuilder<>& builder, bool sign, bool right);
    llvm::Value* HandleDivRem(WasmFunction* fct, llvm::IRBuilder<>& builder, bool sign, bool div);
    llvm::Value* HandleIntrinsic(WasmFunction* fct, llvm::IRBuilder<>& builder);

  public:
    Binop(Operation* op, Expression* l, Expression* r) :
      operation_(op), left_(l), right_(r) {
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);

    Expression* GetRight() const {
      return right_;
    }

    Expression* GetLeft() const {
      return left_;
    }

    void SetRight(Expression* r) {
      right_ = r;
    }

    void SetLeft(Expression* l) {
      left_ = l;
    }

    virtual void Dump(int tabs = 0) const;
};

#endif

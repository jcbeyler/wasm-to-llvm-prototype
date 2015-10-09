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
#ifndef H_EXPRESSION
#define H_EXPRESSION

#include <list>

#include "enums.h"
#include "utility.h"
#include "simple.h"

// Forward declaration.
class WasmFunction;

/**
 * This basic file contains a lot of classes to support the various Wasm opcodes.
 */

class Expression {
  public:
    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(Base Expression %p)", this);
    }

    virtual bool GoesToTheLine() const {
      return false;
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
      BISON_PRINT("No code generation for this expression node\n");
      return nullptr;
    }
};

class Nop : public Expression {
  public:
    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
      return nullptr;
    }
};

class Unop : public Expression {
  protected:
    Expression* only_;
    Operation* operation_;

  public:
    Unop(Operation* op, Expression* only) :
      operation_(op), only_(only) {
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(");

      if (operation_) {
        operation_->Dump();
      } else {
        BISON_PRINT("Operation is nullptr");
      }

      BISON_PRINT(" ");

      if (only_) {
        only_->Dump(0);
      } else {
        BISON_PRINT("nullptr");
      }
      BISON_PRINT(")");
    }
};

class Binop : public Expression {
  protected:
    Expression *left_, *right_;
    Operation* operation_;

    ETYPE HandleType(ETYPE type, llvm::Type* lt, llvm::Type* rt);
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

    Expression* SetRight(Expression* r) {
      right_ = r;
    }

    Expression* SetLeft(Expression* l) {
      left_ = l;
    }

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(");

      if (operation_) {
        operation_->Dump();
      } else {
        BISON_PRINT("Operation is nullptr");
      }

      BISON_PRINT(" ");

      if (left_) {
        left_->Dump(0);
      } else {
        BISON_PRINT("nullptr");
      }

      BISON_PRINT(" ");

      if (right_) {
        right_->Dump(0);
      } else {
        BISON_PRINT("nullptr");
      }
      BISON_PRINT(")");
    }
};

class GetLocal : public Expression {
  protected:
    Variable* var_;

  public:
    GetLocal(Variable* v = nullptr) : var_(v) {
    }

    virtual void Dump(int tabs) const {
      if (var_) {
        BISON_TABBED_PRINT(tabs, "(GetLocal %s)", var_->GetString());
      } else {
        BISON_TABBED_PRINT(tabs, "(GetLocal nullptr)");
      }
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class SetLocal : public Expression {
  protected:
    Variable* var_;
    Expression* value_;

  public:
    SetLocal(Variable* v, Expression* val) : var_(v), value_(val) {
    }

    virtual void Dump(int tabs) const {
      if (var_ && value_) {
        BISON_TABBED_PRINT(tabs, "(SetLocal %s ", var_->GetString());
        value_->Dump(0);
        BISON_PRINT(")");
      } else {
        BISON_TABBED_PRINT(tabs, "(SetLocal nullptr)");
      }
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class IfExpression : public Expression {
  protected:
    Expression* cond_;
    Expression* true_cond_;
    Expression* false_cond_;

  public:
    IfExpression(Expression* c, Expression* t, Expression* f = nullptr) :
      cond_(c), true_cond_(t), false_cond_(f) {
    }

    virtual void Dump(int tabs) const {
      if (cond_ && true_cond_) {
        BISON_TABBED_PRINT(tabs, "(If ");
        cond_->Dump(0);
        BISON_PRINT("\n");
        true_cond_->Dump(tabs + 1);
        BISON_PRINT("\n");
        if (false_cond_) {
          false_cond_->Dump(tabs + 1);
        }
        BISON_TABBED_PRINT(tabs, ")\n");
      } else {
        BISON_TABBED_PRINT(tabs, "(IfExpression %p %p %p)", cond_, true_cond_, false_cond_);
      }
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class Const: public Expression {
  protected:
    ETYPE type_;
    ValueHolder* value_;

  public:
    Const(ETYPE t, ValueHolder* v) : type_(t), value_(v) {
    }

    virtual void Dump(int tabs) const {
      if (value_) {
        BISON_TABBED_PRINT(tabs, "(const.%s ", GetETypeName(type_));
        value_->Dump();
        BISON_PRINT(")");
      } else {
        BISON_TABBED_PRINT(tabs, "(Const Value is nullptr)");
      }
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class CallExpression : public Expression {
  protected:
    Variable* call_id_;
    std::list<Expression*>* params_;

  public:
    CallExpression(Variable* id, std::list<Expression*> *params) :
      call_id_(id), params_(params) {
    }

    CallExpression(Variable* id, Expression* p) :
      call_id_(id) {
        params_ = new std::list<Expression*>();
        params_->push_back(p);
    }

    CallExpression(Variable* id) :
      call_id_(id), params_(nullptr) {
    }

    Variable* GetVariable() const {
      return call_id_;
    }

    llvm::Function* GetCallee(WasmFunction* fct) const;

    virtual void Dump(int tabs) const {
      BISON_PRINT("(Call ");
      if (call_id_) {
        call_id_->Dump(0);
      } else {
        BISON_PRINT("NO-ID");
      }

      if (params_) {
        for (auto elem : *params_) {
          BISON_PRINT(" ");
          elem->Dump(0);
        }
      }

      BISON_PRINT(")");
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class ReturnExpression : public Expression {
  protected:
    Expression* result_;

  public:
    ReturnExpression(Expression* expr) : result_(expr) {
    }

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(Return");
      if (result_) {
        BISON_PRINT(" ");
        result_->Dump(0);
      }
      BISON_PRINT(")");
    }

    Expression* GetResult() const {
      return result_;
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class LoopExpression : public Expression {
  protected:
    Expression* loop_;

  public:
    LoopExpression(Expression* expr) : loop_(expr) {
    }

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(Loop\n");
      if (loop_) {
        loop_->Dump(tabs + 1);
      }
      BISON_TABBED_PRINT(tabs, ")\n");
    }

    virtual bool GoesToTheLine() const {
      return true;
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class LabelExpression : public Expression {
  protected:
    Variable* var_;
    Expression* expr_;

  public:
    LabelExpression(Variable* v, Expression* e) : var_(v), expr_(e) {
    }

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(Label");

      if (var_) {
        BISON_PRINT(" ");
        var_->Dump(0);
      }

      BISON_PRINT("\n");
      if (expr_) {
        expr_->Dump(tabs + 1);
      }
      BISON_TABBED_PRINT(tabs, ")\n");
    }

    virtual bool GoesToTheLine() const {
      return true;
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class BreakExpression : public Expression {
  protected:
    Variable* var_;
    Expression* expr_;

  public:
    BreakExpression(Variable* v = nullptr, Expression* e = nullptr) : var_(v), expr_(e) {
    }

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(Break");

      if (var_) {
        BISON_PRINT(" ");
        var_->Dump(0);
      }

      if (expr_) {
        BISON_PRINT("\n");
        expr_->Dump(tabs + 1);
      }
      BISON_PRINT(")");
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class BlockExpression : public Expression {
  protected:
    std::list<Expression*>* list_;

  public:
    BlockExpression(std::list<Expression*>* l) : list_(l) {
    }

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(Block");

      if (list_) {
        BISON_PRINT("\n");
        for(std::list<Expression*>::const_iterator iter = list_->begin();
                                                   iter != list_->end();
                                                   iter++) {
          auto elem = *iter;
          elem->Dump(tabs + 1);
          BISON_PRINT("\n");
        }
        BISON_TABBED_PRINT(tabs, ")\n");
      } else {
        BISON_PRINT(")\n");
      }
    }

    virtual bool GoesToTheLine() const {
      return true;
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class StringExpression : public Expression {
  protected:
    char* s_;

  public:
    StringExpression(char* s) : s_(s) {
    }

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(String Expression %s)", s_);
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
      return builder.CreateGlobalStringPtr(s_);
    }
};

class ValueExpression : public Expression {
  protected:
    llvm::Value* value_;

  public:
    ValueExpression(llvm::Value* value) : value_(value) {
    }

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(Value Expression %p)", value_);
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
      return value_;
    }
};
#endif

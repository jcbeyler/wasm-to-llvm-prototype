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

#include "base_expression.h"
#include "enums.h"
#include "operation.h"
#include "simple.h"
#include "utility.h"

// Forward declaration.
class SwitchExpression;
class WasmFunction;

class Unop : public Expression {
  protected:
    Operation* operation_;
    Expression* only_;

  public:
    Unop(Operation* op, Expression* only) :
      operation_(op), only_(only) {
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);

    virtual void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(");

      if (operation_) {
        operation_->Dump();
      } else {
        BISON_PRINT("Operation is nullptr");
      }

      BISON_PRINT(" ");

      if (only_) {
        only_->Dump();
      } else {
        BISON_PRINT("nullptr");
      }
      BISON_PRINT(")");
    }

    virtual bool Walk(bool (*fct)(Expression*, void*), void* data) {
      assert(fct != nullptr);

      if (fct(this, data) == false) {
        return false;
      }

      if (only_ != nullptr) {
        return only_->Walk(fct, data);
      }

      return true;
    }
};


class GetLocal : public Expression {
  protected:
    Variable* var_;

  public:
    GetLocal(Variable* v = nullptr) : var_(v) {
    }

    virtual void Dump(int tabs = 0) const {
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

    virtual void Dump(int tabs = 0) const {
      if (var_ && value_) {
        BISON_TABBED_PRINT(tabs, "(SetLocal %s ", var_->GetString());
        value_->Dump();
        BISON_PRINT(")");
      } else {
        BISON_TABBED_PRINT(tabs, "(SetLocal nullptr)");
      }
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);

    virtual bool Walk(bool (*fct)(Expression*, void*), void* data) {
      assert(fct != nullptr);
      if (fct(this, data) == false) {
        return false;
      }

      if (value_ != nullptr) {
        return value_->Walk(fct, data);
      }

      return true;
    }
};

class IfExpression : public Expression {
  protected:
    Expression* cond_;
    Expression* true_cond_;
    Expression* false_cond_;
    bool should_merge_;

    llvm::Value* TransformCondition(llvm::Value* value, llvm::IRBuilder<>& builder);

  public:
    IfExpression(Expression* c, Expression* t, Expression* f = nullptr) :
      cond_(c), true_cond_(t), false_cond_(f), should_merge_(true) {
    }

    void SetTrue(Expression* expr) {
      true_cond_ = expr;
    }

    void SetFalse(Expression* expr) {
      false_cond_ = expr;
    }

    void SetShouldMerge(bool b) {
      should_merge_ = b;
    }

    virtual void Dump(int tabs = 0) const {
      if (cond_ && true_cond_) {
        BISON_TABBED_PRINT(tabs, "(If ");
        cond_->Dump();
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

    Expression* GetCondition() const {
      return cond_;
    }

    Expression* GetTrue() const {
      return true_cond_;
    }

    Expression* GetFalse() const {
      return false_cond_;
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);

    virtual bool Walk(bool (*fct)(Expression*, void*), void* data) {
      assert(fct != nullptr);
      if (fct(this, data) == false) {
        return false;
      }

      if (cond_ != nullptr) {
        if (cond_->Walk(fct, data) == false) {
          return false;
        }
      }

      if (true_cond_ != nullptr) {
        if (true_cond_->Walk(fct, data) == false) {
          return false;
        }
      }

      if (false_cond_ != nullptr) {
        if (false_cond_->Walk(fct, data) == false) {
          return false;
        }
      }

      return true;
    }
};

class Const: public Expression {
  protected:
    ETYPE type_;
    ValueHolder* value_;

  public:
    Const(ETYPE t, ValueHolder* v) : type_(t), value_(v) {
      switch (t) {
        case FLOAT_32:
          v->Convert(VH_FLOAT);
          break;
        case FLOAT_64:
          v->Convert(VH_DOUBLE);
          break;
        default:
          break;
      }
    }

    virtual void Dump(int tabs = 0) const {
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
    int line_;

  public:
    CallExpression(Variable* id, std::list<Expression*> *params) :
      call_id_(id), params_(params), line_(0) {
    }

    CallExpression(Variable* id, Expression* p) :
      call_id_(id), line_(0) {
        params_ = new std::list<Expression*>();
        params_->push_back(p);
    }

    CallExpression(Variable* id) :
      call_id_(id), params_(nullptr), line_(0) {
    }

    void SetLine(int line) {
      line_ = line;
    }

    int GetLine() const {
      return line_;
    }

    Variable* GetVariable() const {
      return call_id_;
    }

    WasmFunction* GetCallee(WasmFunction* fct) const;

    virtual void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(Call ");
      if (call_id_) {
        call_id_->Dump();
      } else {
        BISON_PRINT("NO-ID");
      }

      if (params_) {
        for (auto elem : *params_) {
          BISON_PRINT(" ");
          elem->Dump();
        }
      }

      BISON_PRINT(")");
    }

    virtual bool Walk(bool (*fct)(Expression*, void*), void* data) {
      assert(fct != nullptr);
      if (fct(this, data) == false) {
        return false;
      }

      for (auto elem : *params_) {
        if (elem->Walk(fct, data) == false) {
          return false;
        }
      }

      return true;
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class ReturnExpression : public Expression {
  protected:
    Expression* result_;

  public:
    ReturnExpression(Expression* expr) : result_(expr) {
    }

    virtual void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(Return");
      if (result_) {
        BISON_PRINT(" ");
        result_->Dump();
      }
      BISON_PRINT(")");
    }

    Expression* GetResult() const {
      return result_;
    }

    virtual bool Walk(bool (*fct)(Expression*, void*), void* data) {
      assert(fct != nullptr);

      if (fct(this, data) == false) {
        return false;
      }

      if (result_ != nullptr) {
        if (result_->Walk(fct, data) == false) {
          return false;
        }
      }

      return true;
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

// Base class for Loops and Blocks to register incoming phi nodes.
class NamedExpression {
  protected:
    std::vector<std::pair<llvm::Value*, llvm::BasicBlock*> > incoming_phis_;

  public:
    void AddIncomingPhi(llvm::Value* value, llvm::BasicBlock* bb);

    llvm::Value* HandlePhiNodes(llvm::IRBuilder<>& builder) const;
};

class LoopExpression : public Expression, public NamedExpression {
  protected:
    Variable* var_;
    Variable* exit_name_;
    std::list<Expression*>* loop_;

  public:
    LoopExpression(Variable* var, Variable* exit_name, std::list<Expression*>* list) : var_(var), exit_name_(exit_name), loop_(list) {
    }

    virtual void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(Loop");

      if (var_ != nullptr) {
        BISON_PRINT("%s", var_->GetString());
      }
      BISON_PRINT("\n");

      if (loop_ != nullptr) {
        for(std::list<Expression*>::iterator it = loop_->begin();
                                             it != loop_->end();
                                             it++) {
          Expression* expr = *it;
          expr->Dump(tabs + 1);
        }
      }
      BISON_TABBED_PRINT(tabs, ")\n");
    }

    virtual bool GoesToTheLine() const {
      return true;
    }

    virtual bool Walk(bool (*fct)(Expression*, void*), void* data) {
      assert(fct != nullptr);

      if (fct(this, data) == false) {
        return false;
      }

      for(std::list<Expression*>::iterator it = loop_->begin();
          it != loop_->end();
          it++) {
        Expression* expr = *it;
        if (expr->Walk(fct, data) == false) {
          return false;
        }
      }

      return true;
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class LabelExpression : public Expression, NamedExpression {
  protected:
    Variable* var_;
    Expression* expr_;

  public:
    LabelExpression(Variable* v, Expression* e) : var_(v), expr_(e) {
    }

    virtual void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(Label");

      if (var_) {
        BISON_PRINT(" ");
        var_->Dump();
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

    virtual bool Walk(bool (*fct)(Expression*, void*), void* data) {
      assert(fct != nullptr);

      if (fct(this, data) == false) {
        return false;
      }

      if (expr_ != nullptr) {
        if (expr_->Walk(fct, data) == false) {
          return false;
        }
      }

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

    virtual void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(Break");

      if (var_) {
        BISON_PRINT(" ");
        var_->Dump();
      }

      if (expr_) {
        BISON_PRINT("\n");
        expr_->Dump(tabs + 1);
      }
      BISON_PRINT(")");
    }

    llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class BlockExpression : public Expression, public NamedExpression {
  protected:
    std::list<Expression*>* list_;
    const char* name_;

  public:
    BlockExpression(const char* name, std::list<Expression*>* l) : name_(name), list_(l) {
    }

    virtual void Dump(int tabs = 0) const {
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

    virtual bool Walk(bool (*fct)(Expression*, void*), void* data) {
      assert(fct != nullptr);

      if (fct(this, data) == false) {
        return false;
      }

      for(std::list<Expression*>::const_iterator iter = list_->begin();
                                                 iter != list_->end();
                                                 iter++) {
        auto elem = *iter;
        if (elem->Walk(fct, data) == false) {
          return false;
        }
      }

      return false;
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

    virtual void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(String Expression %s)", s_);
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
      (void) fct;

      return builder.CreateGlobalStringPtr(s_);
    }
};

class ValueExpression : public Expression {
  protected:
    llvm::Value* value_;

  public:
    ValueExpression(llvm::Value* value) : value_(value) {
    }

    virtual void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(Value Expression %p)", value_);
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
      (void) fct;
      (void) builder;

      return value_;
    }
};

class Unreachable : public Expression {
  public:
    Unreachable() {
    }

    virtual void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(Unreachable)");
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class CaseExpression : public Expression {
  protected:
    const char* id_;
    Expression* expr_;

  public:
    CaseExpression(const char* id, Expression* expr) :
      id_(id), expr_(expr) {
    }

    const char* GetIdentifier() const {
      return id_;
    }

    llvm::Value* Codegen(llvm::Value* last_value, SwitchExpression* switch_expr, WasmFunction* fct, llvm::IRBuilder<>& builder);
};

class SwitchExpression : public Expression, public NamedExpression {
  protected:
    const char* name_;
    Expression* selector_;
    std::list<Variable*>* index_table_;
    Variable* default_;
    std::list<CaseExpression*>* cases_;
    std::map<std::string, llvm::BasicBlock*> generated_cases_;

  public:
    SwitchExpression(const char* name, Expression* selector,
                     std::list<Variable*>* index_table,
                     Variable* default_case,
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

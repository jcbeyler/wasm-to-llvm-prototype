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
#ifndef H_FUNCTION_FIELD
#define H_FUNCTION_FIELD

#include "enums.h"
#include "expression.h"
#include "local.h"
#include "utility.h"

/**
 * Function Field base classes, basically any node under the function node
 */

// Forward declarations.
class Expression;
class Local;

class FunctionField {
  public:
    virtual void Dump(int tabs = 0) {
      BISON_TABBED_PRINT(tabs, "(Base Function Field)");
    }

    virtual bool GoesToTheLine() const {
      return false;
    }
};

class ResultField : public FunctionField {
  protected:
    ETYPE type_;

  public:
    ResultField() : type_(INT_32) {
    }

    ResultField(int value) : type_(static_cast<ETYPE> (value)) {
    }

    ETYPE GetType() const {
      return type_;
    }

    virtual void Dump(int tabs = 0) {
      BISON_TABBED_PRINT(tabs, "(Result %s)", GetETypeName(type_));
    }
};

class ParamField : public FunctionField {
  protected:
    Local* local_;

  public:
    ParamField() : local_(nullptr) {
    }

    ParamField(Local* l) : local_(l) {
    }

    Local* GetLocal() const {
      return local_;
    }

    virtual void Dump(int tabs = 0) {
      if (local_ != nullptr) {
        local_->Dump("Parameter", tabs);
      } else {
        BISON_TABBED_PRINT(tabs, "(Weird, local_ is a nullptr)");
      }
    }
};

class ExpressionField : public FunctionField {
  protected:
    Expression* expression_;

  public:
    ExpressionField() : expression_(nullptr) {
    }

    ExpressionField(Expression* param) : expression_(param) {
    }

    virtual void Dump(int tabs = 0) {
      if (expression_ != nullptr) {
        expression_->Dump(tabs);
      } else {
        BISON_TABBED_PRINT(tabs, "(Weird, expression_ is a nullptr)");
      }
    }

    Expression* GetExpression() const {
      return expression_;
    }

    virtual bool GoesToTheLine() const {
      return expression_ ? expression_->GoesToTheLine() : false;
    }
};

class LocalField : public FunctionField {
  protected:
    Local* local_;

  public:
    LocalField() : local_(nullptr) {
    }

    LocalField(Local* l) : local_(l) {
    }

    Local* GetLocal() const {
      return local_;
    }

    virtual void Dump(int tabs = 0) {
      if (local_ != nullptr) {
        local_->Dump("Local", tabs);
      } else {
        BISON_TABBED_PRINT(tabs, "(Weird, local_ is a nullptr)");
      }
    }
};

#endif

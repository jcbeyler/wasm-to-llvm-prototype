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
#ifndef H_SIMPLE
#define H_SIMPLE

#include "debug.h"
#include "utility.h"

/**
 * Variable, operation and value holder containers.
 */

class Variable {
  protected:
    size_t idx_;
    char* s_;

    bool is_string_;

  public:
    Variable(int64_t t) {
      idx_ = t;
      s_ = new char[32];
      sprintf(s_, "%ld", t);
      is_string_ = false;
    }

    Variable(char* v) {
      s_ = v;
      is_string_ = true;
    }

    Variable(const char* v) {
      s_ = strdup(v);
      is_string_ = true;
    }

    bool IsString() const {
      return is_string_;
    }

    size_t GetIdx() const {
      return idx_;
    }

    void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "%s", s_);
    }

    const char* GetString() const {
      return s_;
    }
};

class Operation {
  protected:
    OPERATION op_;
    ETYPE type_;
    bool sign_or_order_;

  public:
    Operation(OPERATION o, bool sign_or_order, ETYPE t) : op_(o), sign_or_order_(sign_or_order), type_(t) {
    }

    Operation(OPERATION o, bool sign_or_order) : op_(o), sign_or_order_(sign_or_order), type_(VOID) {
    }

    Operation(OPERATION o) : op_(o), sign_or_order_(true), type_(VOID) {
      // Special case for the NE operation: by default it is unordered.
      if (o == NE_OPER) {
        sign_or_order_ = false;
      }
    }

    OPERATION GetOperation() const {
      return op_;
    }

    ETYPE GetType() const {
      return type_;
    }

    bool GetSignedOrOrdered() const {
      return sign_or_order_;
    }

    void SetSignedOrOrdered(bool b) {
      sign_or_order_ = b;
    }

    void SetType(ETYPE type) {
      type_ = type;
    }

    void Dump() {
      BISON_PRINT("%s.%s", DumpOperation(op_), GetETypeName(type_));
    }
};

class ValueHolder {
  public:
  union {
    int64_t i;
    double d;
  } value_;
  bool is_double_;

  public:
    ValueHolder(double d) {
      value_.d = d;
      is_double_ = true;
    }

    ValueHolder(int64_t i) {
      value_.i = i;
      is_double_ = false;
    }

    ValueHolder(int i) {
      value_.i = i;
      is_double_ = false;
    }

    int64_t GetInteger() const {
      if (is_double_ == true) {
        int64_t res = value_.d;
        return res;
      }
      return value_.i;
    }

    double GetDouble() const {
      // This happens if we had a double from the parser that was actually in integer form.
      if (is_double_ == false) {
        double res = value_.i;
        return res;
      }
      return value_.d;
    }

    void Dump() {
      if (is_double_) {
        BISON_PRINT("%f", value_.d);
      } else {
        BISON_PRINT("%ld", value_.i);
      }
    }
};

#endif

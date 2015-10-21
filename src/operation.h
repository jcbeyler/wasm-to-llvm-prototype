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
#ifndef H_OPERATION
#define H_OPERATION

#include "debug.h"
#include "enums.h"
#include "utility.h"

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

    virtual void Dump() {
      BISON_PRINT("%s.%s", DumpOperation(op_), GetETypeName(type_));
    }
};

class ConversionOperation : public Operation {
  protected:
    ETYPE src_;

  public:
    ConversionOperation(OPERATION op, bool sign, ETYPE dest, ETYPE src) : 
      Operation(op, sign, dest), src_(src) {
    }

    ConversionOperation(OPERATION op, bool sign = false) :
      Operation(op, sign), src_(VOID) {
    }

    OPERATION GetOperation() const {
      return op_;
    }

    ETYPE SetDest(ETYPE val) {
      type_ = val;
    }

    ETYPE SetSrc(ETYPE val) {
      src_ = val;
    }

    ETYPE GetDest() const {
      return type_;
    }

    ETYPE GetSrc() const {
      return src_;
    }

    virtual void Dump() {
      BISON_PRINT("%s.%s_%c/%s", GetETypeName(type_), DumpOperation(op_), 
                                 sign_or_order_ ? 's' : 'u', GetETypeName(src_));
    }
};

#endif

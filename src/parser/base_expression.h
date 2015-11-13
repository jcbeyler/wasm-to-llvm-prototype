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
#ifndef H_BASE_EXPRESSION
#define H_BASE_EXPRESSION

#include <stdio.h>

#include "debug.h"

// Forward declaration.
class WasmFunction;

/**
 * This basic file contains the base implemetation of an expression node.
 */

class Expression {
  public:
    virtual void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(Base Expression %p)", this);
    }

    virtual bool GoesToTheLine() const {
      return false;
    }

    virtual bool Walk(bool (*fct)(Expression*, void*), void* data) {
      assert(fct != nullptr);
      return fct(this, data);
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
      BISON_PRINT("No code generation for this expression node\n");

      (void) fct;
      (void) builder;

      return nullptr;
    }
};

#endif

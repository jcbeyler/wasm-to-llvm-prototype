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

// File contains very basic passes.
#include "basic.h"
#include "function.h"

static bool FindUnreachable(Expression* expr, void* ptr_b) {
  bool* result = static_cast<bool*>(ptr_b);

  // If we have an if, we don't know, set it back to true.
  if (dynamic_cast<IfExpression*>(expr) != nullptr) {
    *result = false;

    // Stop walking.
    return false;
  }

  // Test for unreachable.
  if (dynamic_cast<Unreachable*>(expr) != nullptr) {
    *result = true;
  }

  // Continue walking.
  return true;
}

static void HandleIfUnreachable(Expression* expr) {
  IfExpression* if_expr = dynamic_cast<IfExpression*>(expr);
  if (if_expr != nullptr) {
    Expression* true_expr = if_expr->GetTrue();
    Expression* false_expr = if_expr->GetFalse();

    if (true_expr != nullptr && false_expr != nullptr) {
      // We want to know if there is an unreachable directly in these blocks.
      //   This is not perfect because if there is a complex CFG here,
      //   this won't solve it yet.
      bool true_has_unreachable = false;
      true_expr->Walk(FindUnreachable, &true_has_unreachable);

      bool false_has_unreachable = false;
      false_expr->Walk(FindUnreachable, &false_has_unreachable);

      if (true_has_unreachable != false_has_unreachable) {
        if_expr->SetShouldMerge(false);
      }
    }
  }
}

static bool HandleUnreachable(Expression* expr, void* data) {
  HandleIfUnreachable(expr);

  // Continue walking.
  return true;
}

void UnreachablePass::Run(WasmFunction* fct, void* data) {
  // Walk the function's AST looking for unreachable nodes.
  fct->Walk(HandleUnreachable, data);
}

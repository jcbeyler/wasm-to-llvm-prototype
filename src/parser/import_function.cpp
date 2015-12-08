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

#include "import_function.h"


llvm::Function* WasmImportFunction::GetFunction() {
  if (function_ == nullptr) {
    fprintf(stderr, "Import Function not created yet: Internal name: %s Module: %s Function: %s\n", internal_name_.c_str(), module_.c_str(), function_name_.c_str()); 

    // Start by creating the mangled name.
    assert(0);
  }

  // Paranoid.
  assert(function_ != nullptr);

  // Finally we can return the function_.
  return function_;
}


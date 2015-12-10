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

#ifndef H_IMPORT_FUNCTION
#define H_IMPORT_FUNCTION

#include <list>
#include <string>
#include <sstream>

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "debug.h"
#include "function_field.h"

// Forward declarations.
class FunctionField;
class WasmModule;

/**
 * Definition of an imported function node in the Wasm format
 */
class WasmImportFunction {
  protected:
    std::string internal_name_;

    std::string module_name_;
    std::string function_name_;
    std::list<FunctionField*>* fields_;

    ETYPE result_;
    llvm::Function* function_;

    // Populate the params vector and fill the result field.
    void Populate(std::vector<llvm::Type*>& params);

  public:
    WasmImportFunction(const std::string& module, const std::string& function_name,
                       std::list<FunctionField*>* f, const std::string& s = "anonymous") :
                       internal_name_(s), module_name_(module), function_name_(function_name), fields_(f),
                       result_(VOID) {
      // If anonymous, let's add a unique suffix.
      if (internal_name_ == "imported_anonymous") {
        static int cnt = 0;
        std::ostringstream oss;
        oss << internal_name_ << "_" << cnt;
        cnt++;
        internal_name_ = oss.str();
      }
    }

    void Dump(int tab = 0) {
      BISON_TABBED_PRINT(tab, "Import Function: Internal name: %s Module: %s Function: %s\n", internal_name_.c_str(), module_name_.c_str(), function_name_.c_str());

      if (fields_ != nullptr) {
        BISON_TABBED_PRINT(tab + 1, "Function fields:");
        for(std::list<FunctionField*>::const_iterator it = fields_->begin();
                                                      it != fields_->end();
                                                      it++) {
          FunctionField* ff = *it;
          ff->Dump(tab + 2);
        }
      }
    }

    llvm::Function* GetFunction(WasmModule* module);

    ETYPE GetResult() const {
      return result_;
    }

    const std::string& GetName() {
      return internal_name_;
    }
};

#endif

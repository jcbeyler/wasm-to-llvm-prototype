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
#include "module.h"
#include "utility.h"

llvm::Function* WasmImportFunction::GetFunction(WasmModule* module) {
  if (function_ == nullptr) {
    // This probably should be done better but will work like this for now.
    std::string full_name = module_name_ + "_" + function_name_;

    // Now what we really want is the parameters of this method.
    std::vector<llvm::Type*> params;
    Populate(params);

    // Add to the name the type of the arguments for the import.
    //  Again, probably not what we want finally but this will work.
    //  My problem right now is how to handle the variadic functions well.
    for (auto elem : params) {
      full_name += "_";
      full_name += GetTypeName(elem);
    }

    BISON_PRINT("Import Function not created yet: Internal name: %s Module: %s Function: %s -> Full %s\n", internal_name_.c_str(), module_name_.c_str(), function_name_.c_str(), full_name.c_str());

    // Now get the result type.
    llvm::Type* result_type = ConvertType(result_);

    // Finally, create the function type.
    llvm::FunctionType* fct_type = llvm::FunctionType::get(result_type, params, false);

    // Now create the function.
    function_ = llvm::Function::Create(fct_type, llvm::Function::ExternalLinkage, full_name, module->GetModule());
  }

  // Paranoid.
  assert(function_ != nullptr);

  // Finally we can return the function_.
  return function_;
}

void WasmImportFunction::Populate(std::vector<llvm::Type*>& params) {
  if (fields_ != nullptr) {
    for (std::list<FunctionField*>::const_iterator it = fields_->begin();
                                                   it != fields_->end();
                                                   it++) {
      const FunctionField* ff = *it;

      // Is it a result?
      const ResultField* rf = dynamic_cast<const ResultField*>(ff);
      if (rf != nullptr) {
        // We only support one return.
        assert(result_ == VOID);

        result_ = rf->GetType();
      } else {
        // What about a parameter?
        const ParamField* pf = dynamic_cast<const ParamField*>(ff);

        // In the case of an import function, it has to either be a result or a parameter...
        assert (pf != nullptr);

        Local* local = pf->GetLocal();

        const std::deque<LocalElem*>& list = local->GetList();

        for (auto elem : list) {
          params.push_back(ConvertType(elem->GetType()));
        }
      }
    }
  }
}

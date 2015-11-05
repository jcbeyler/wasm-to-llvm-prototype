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
#include <string>
#include <iostream>

#include "debug.h"
#include "function.h"
#include "module.h"

llvm::Type* WasmFunction::GetReturnType() const {
  return ConvertType(result_);
}

void WasmFunction::FindParams(std::vector<llvm::Type*>& llvm_params) const {
  // Go through all the parameter fields.
  for (std::vector<ParamField*>::const_iterator it = params_.begin();
                                              it != params_.end();
                                              it++) {
    ParamField* pf = *it;
    Local* local = pf->GetLocal();

    const std::list<LocalElem*>& list = local->GetList();

    for (auto elem : list) {
      llvm_params.push_back(ConvertType(elem->GetType()));
    }
  }
}

void WasmFunction::Populate() {
  if (fields_ != nullptr) {
    bool already_have_result = false;
    params_.clear();

    // Go through all the function fields, and find the parameters.
    for (std::list<FunctionField*>::const_iterator it = fields_->begin();
                                                   it != fields_->end();
                                                   it++) {

      FunctionField* ff = *it;

      // First check if it is a parameter.
      ParamField* pf = dynamic_cast<ParamField*>(ff);

      if (pf != nullptr) {
        params_.push_back(pf);
      } else {
        // Second check if it is a result.
        ResultField* rf = dynamic_cast<ResultField*>(ff);

        if (rf != nullptr) {
          // Current limitation, only one result.
          if (already_have_result) {
            result_ = VOID;
          } else {
            already_have_result = true;
            result_ = rf->GetType();
          }
        } else {
          // Third is the expression fields.
          ExpressionField* ef = dynamic_cast<ExpressionField*>(ff);

          if (ef != nullptr) {
            ast_.push_back(ef->GetExpression());
          } else {
            // Third is the local fields.
            LocalField* lf = dynamic_cast<LocalField*>(ff);

            if (lf != nullptr) {
              Local* local = lf->GetLocal();

              // For simplificity, right now just remember them; then we will come back.
              //  We also assume we have only global locals... which seems to be the spec.
              locals_.push_back(local);
            }
          }
        }
      }
    }
  }
}

void WasmFunction::GeneratePrototype(WasmModule* module) {
  BISON_PRINT("Generating %s\n", name_.c_str());
  Dump(0);

  // Start by populating and finding the parameters in LLVM form.
  Populate();

  std::vector<llvm::Type*> params;
  FindParams(params);

  // Then get the result.
  llvm::Type* result_type = GetReturnType();

  // Finally, create the function type.
  llvm::FunctionType* fct_type = llvm::FunctionType::get(result_type, params, false);

  // Now create the function.
  fct_ = llvm::Function::Create(fct_type, Function::ExternalLinkage, name_, module->GetModule());
}

void WasmFunction::PopulateLocalHolders(llvm::IRBuilder<>& builder) {
  for (auto local : locals_) {
    const std::list<LocalElem*>& elems = local->GetList();

    for (auto elem : elems) {
      // Get the type.
      ETYPE etype = elem->GetType();
      // Convert to LLVM.
      llvm::Type* type = ConvertType(etype);
      assert(type != nullptr);

      Allocate(elem->GetName(), type, builder);
    }
  }
}

llvm::AllocaInst* WasmFunction::Allocate(const char* name, llvm::Type* type, llvm::IRBuilder<>& builder) {
  // If we have a name, fill it up.
  llvm::AllocaInst* alloca_var = nullptr;
  if (name != nullptr) {
    alloca_var = CreateAlloca(name, type, builder);
    map_values_[name] = alloca_var;
  } else {
    std::ostringstream oss;
    oss << "var_" << vector_values_.size();
    // Create an anonymous one.
    alloca_var = CreateAlloca(oss.str().c_str(), type, builder);
  }

  // Always fill the vector.
  // For now push nullptr there, we fill it later.
  vector_values_.push_back(alloca_var);

  return alloca_var;
}

void WasmFunction::PopulateAllocas(llvm::IRBuilder<>& builder) {
  // The problem is that:
  //    WASM allows you to do (params i32 i32 ...) where a single node can have multiple parameters.
  //
  // However LLVM has to go through each argument so we now have to go at different speeds through
  //   the two IR lists.

  // So let's explode the parameters here. If need be, we could do it once and for all if needed again
  //   but it seems not...
  std::list<LocalElem*> elems;

  for (auto it : params_) {
    ParamField& pf = *it;
    Local* local = pf.GetLocal();

    const std::list<LocalElem*>& list = local->GetList();

    for(auto elem : list) {
      elems.push_back(elem);
    }
  }

  // Go through all the parameter fields.
  std::list<LocalElem*>::const_iterator elem_it = elems.begin();

  for (auto &arg: fct_->args()) {
    assert(elem_it != elems.end());
    LocalElem* elem = *elem_it;

    // Get the type.
    ETYPE etype = elem->GetType();
    // Convert to LLVM.
    llvm::Type* type = ConvertType(etype);
    assert(type != nullptr);

    if (elem->GetName()) {
      arg.setName(elem->GetName());
    }

    llvm::AllocaInst* alloca_var = Allocate(elem->GetName(), type, builder);

    // Now create store towards that alloca.
    builder.CreateStore(&arg, alloca_var);

    elem_it++;
  }
  assert(elem_it == elems.end());

  // Generate place holders for the locals.
  PopulateLocalHolders(builder);
}

llvm::AllocaInst* WasmFunction::CreateAlloca(const char* name, llvm::Type* type, llvm::IRBuilder<>& builder) {
  return builder.CreateAlloca(type, 0, name);
}

void WasmFunction::GetBaseMemory(llvm::IRBuilder<>& builder) {
  // Only care about this if we have a memory to the module.
  //  We could check if the function would use it and use that but this should be
  //   sufficient as LLVM can remove the call if it is unused.
  if (module_->GetMemory() != 0) {
    // Ok, then what we want is ask it for the address.
    llvm::GlobalVariable* base = module_->GetBaseMemory();

    local_base_ = builder.CreateLoad(base, "local_base");
  }
}

void WasmFunction::Generate(WasmModule* module) {
  // Remember the module.
  module_ = module;

  GeneratePrototype(module);

  if (fct_ == nullptr) {
    BISON_PRINT("Problem with function definition: %s\n", name_.c_str());
    return;
  }

  llvm::BasicBlock* bb = llvm::BasicBlock::Create(getGlobalContext(), "entry", fct_);
  llvm::IRBuilder<> builder(getGlobalContext());
  builder.SetInsertPoint(bb);
  llvm::Value* last = nullptr;
  bool is_last_return = false;

  // Now that we have that, we can actually create the allocas for the input arguments and the locals of the method.
  PopulateAllocas(builder);

  // Generate the "get me the memory linear call".
  GetBaseMemory(builder);

  for (auto iter : ast_) {
    Expression* exp = iter;
    last = exp->Codegen(this, builder);
    is_last_return = (dynamic_cast<ReturnExpression*>(exp) != nullptr);
  }

  // For the last one, if it is not a return and our method has a result, this becomes our result.
  if (result_ != VOID) {
    if (is_last_return == false && last != nullptr) {
      HandleReturn(last, builder);
    }
  } else {
    builder.CreateRetVoid();
  }
}

llvm::Value* WasmFunction::HandleReturn(llvm::Value* result, llvm::IRBuilder<>& builder) const {
  return builder.CreateRet(HandleSimpleTypeCasts(result, ConvertType(result_), false, builder));
}

llvm::AllocaInst* WasmFunction::GetVariable(const char* name) const {
  std::map<std::string, llvm::AllocaInst*>::const_iterator it = map_values_.find(name);

  if (it == map_values_.end()) {
    return nullptr;
  }

  return it->second;
}

llvm::AllocaInst* WasmFunction::GetVariable(size_t idx) const {
  if (idx < vector_values_.size()) {
    return vector_values_[idx];
  }

  return nullptr;
}

llvm::BasicBlock* WasmFunction::GetLabel(size_t from_last) {
  // Get the index from the end.
  int i = labels_.size() - 1 - from_last;

  // Error checking.
  if (i < 0) {
    return nullptr;
  }

  return labels_[i];
}

llvm::BasicBlock* WasmFunction::GetLabel(const char* name) {
  for (auto elem : labels_) {
    if (elem->getName() == name) {
      return elem;
    }
  }

  return nullptr;
}

void WasmFunction::MangleNames(WasmFile* file, WasmModule* module) {
  // First mangle the function name.
  MangleFunctionName(file, module);
}

void WasmFunction::MangleFunctionName(WasmFile* file, WasmModule* module) {
  std::ostringstream oss;

  // We start by mangling it using the module hash.
  const std::string& hash = module->GetHashName();
  oss << hash << "_";

  // Then we get the function's name.
  oss << name_;

  // Get the string of this.
  std::string s = oss.str();

  std::string end_name = "";
  bool last_was_underscore = false;
  for (char c : s) {
    // Basically, we accept lower case letters.
    if (c >= 'a' && c <= 'z') {
      end_name += c;
      last_was_underscore = false;
      continue;
    }

    // And upper case.
    if (c >= 'A' && c <= 'Z') {
      end_name += c;
      last_was_underscore = false;
      continue;
    }

    // And digits.
    if (c >= '0' && c <= '9') {
      end_name += c;
      last_was_underscore = false;
      continue;
    }

    // And any other character becomes an underscore but only put one in a row.
    if (last_was_underscore == false) {
      end_name += '_';
      last_was_underscore = true;
      continue;
    }
  }

  // Check if it is not in the module already,
  //   no need to check the file since we have the module prefix.
  while (module->DoesMangledNameExist(end_name) == true) {
    // Now we add a random letter character until the name is unique.
    char c = 'a' + rand() % 26;
    end_name += c;
  }

  // Now we have a unique method, register it.
  module->RegisterMangle(name_, end_name);

  // Now mangle it.
  name_ = end_name;
}

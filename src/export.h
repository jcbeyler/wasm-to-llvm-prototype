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
#ifndef H_EXPORT
#define H_EXPORT

#include "simple.h"

class WasmExport {
  protected:
    std::string name_;
    Variable* var_;

  public:
    WasmExport(const std::string& n, Variable* v) : name_(n), var_(v) {
    }

    const std::string& GetName() const {
      return name_;
    }

    Variable* GetVariable() const {
      return var_;
    }

    void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "(export %s ", name_.c_str());
      var_->Dump(0);
      BISON_PRINT(")");
    }
};


#endif

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
#ifndef H_WASM_SCRIPT
#define H_WASM_SCRIPT

#include <deque>

#include "wasm_script_elem.h"

// Forward declaration.
class WasmScriptElem;
class WasmFile;

class WasmScript {
  protected:
    std::deque<WasmScriptElem*> script_elems_;

  public:
    WasmScript() {
    }

    void AddScriptElem(WasmScriptElem* a) {
      script_elems_.push_front(a);
    }

    void Dump() const {
      for(auto elem : script_elems_) {
        elem->Dump();
      }
    }

    void GenerateGeneralScriptCalls(WasmFile* file);

    void Generate(WasmFile* file) {
      for(auto elem : script_elems_) {
        elem->Codegen(file);
      }

      // Now we want to call each of the methods.
      GenerateGeneralScriptCalls(file);
    }
};

#endif

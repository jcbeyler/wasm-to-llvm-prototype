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

class FunctionField;

/**
 * Definition of an imported function node in the Wasm format
 */
class WasmImportFunction {
  protected:
    std::string exported_name_;

    std::string module_;
    std::string function_name_;
    std::list<FunctionField*>* fields_;

  public:
    WasmImportFunction(const std::string& module, const std::string& function_name,
                       std::list<FunctionField*>* f, const std::string& s = "anonymous") :
                       exported_name_(s), module_(module), function_name_(function_name), fields_(f) {
      // If anonymous, let's add a unique suffix.
      if (exported_name_ == "imported_anonymous") {
        static int cnt = 0;
        std::ostringstream oss;
        oss << exported_name_ << "_" << cnt;
        cnt++;
        exported_name_ = oss.str();
      }
    }
};

#endif

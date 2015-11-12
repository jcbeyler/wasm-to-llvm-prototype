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

// Forward declaration.
class WasmFunction;

class Pass {
  public:
    // Gate determines if we do this.
    virtual bool Gate(WasmFunction* fct) {
      (void) fct;

      return true;
    }

    // Run if gate returned true before running the pass.
    virtual void* PreRun(WasmFunction* fct) {
      return nullptr;
    }

    // Run if gate returned true.
    virtual void Run(WasmFunction* fct, void* data) {
      (void) fct;
      (void) data;
    }

    // Run if gate returned true.
    virtual void PostRun(void* data) {
      (void) data;
    }

    // Called before starting the driver.
    virtual void Init() {
    }

    // Called after running the driver.
    virtual void CleanUp() {
    }
};

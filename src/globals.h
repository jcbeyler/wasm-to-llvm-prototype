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

#include <memory>

class Globals {
  protected:
    int line_cnt_;
    char* name_;

    static std::unique_ptr<Globals> g_variables_;

  public:
    Globals() : line_cnt_(1), name_(nullptr) {
    }

    void IncrementLineCnt(int inc = 1) {
      line_cnt_ += inc;
    }

    int GetLineCnt() const {
      return line_cnt_;
    }

    const char* GetFileName() const {
      return name_;
    }

    void SetFileName(char* name) {
      name_ = name;
    }

    static Globals* Get() {
      Globals* res = g_variables_.get();

      if (res == nullptr) {
        g_variables_.reset(new Globals());
      }

      return g_variables_.get();
    }
};

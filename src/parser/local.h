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
#ifndef H_LOCAL
#define H_LOCAL

#include <deque>

#include "utility.h"

/**
 * Structures to handle the local node (basically a variable node)
 */

struct LocalElem {
  ETYPE type_;
  char* name_;

  LocalElem(ETYPE t = INT_32, char* s = nullptr) : type_(t), name_(s) {
  }

  ETYPE GetType() const {
    return type_;
  }

  const char* GetName() const {
    return name_;
  }

  void Dump() const {
    if (name_) {
      BISON_PRINT("%s %s", name_, GetETypeName(type_));
    } else {
      BISON_PRINT("%s", GetETypeName(type_));
    }
  }
};

class Local {
  protected:
    std::deque<LocalElem*> elems_;

  public:
    Local() {
    }

    Local(ETYPE type, char* s) {
      LocalElem* elem = new LocalElem(type, s);
      elems_.push_back(elem);
    }

    void AddElem(ETYPE type, char* s = nullptr) {
      LocalElem* elem = new LocalElem(type, s);
      elems_.push_front(elem);
    }

    void Dump(const std::string& prefix, int tabs = 0) {
      BISON_TABBED_PRINT(tabs, "(%s: ", prefix.c_str());
      size_t max = elems_.size();
      size_t i = 0;

      for(std::deque<LocalElem*>::const_iterator iter = elems_.begin();
                                                iter != elems_.end();
                                                iter++) {
        const LocalElem* elem = *iter;
        elem->Dump();

        i++;

        if (i != max) {
        BISON_PRINT(" ");
        }
      }

      BISON_PRINT(")");

      (void) prefix;
    }

    const std::deque<LocalElem*>& GetList() const {
      return elems_;
    }

    size_t GetNumElems() {
      return elems_.size();
    }

    LocalElem* GetOnlyElem() {
      if (elems_.size() != 1) {
        return nullptr;
      }
      return elems_.front();
    }
};

#endif

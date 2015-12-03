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
#ifndef H_SIMPLE
#define H_SIMPLE

#include "debug.h"
#include "utility.h"

/**
 * Variable, operation and value holder containers.
 */

class Variable {
  protected:
    size_t idx_;
    char* s_;

    bool is_string_;

  public:
    Variable(int64_t t) {
      idx_ = t;
      s_ = new char[32];
      sprintf(s_, "%ld", t);
      is_string_ = false;
    }

    Variable(char* v) {
      s_ = v;
      is_string_ = true;
    }

    Variable(const char* v) {
      s_ = strdup(v);
      is_string_ = true;
    }

    bool IsString() const {
      return is_string_;
    }

    size_t GetIdx() const {
      return idx_;
    }

    void Dump(int tabs = 0) const {
      BISON_TABBED_PRINT(tabs, "%s", s_);
    }

    const char* GetString() const {
      return s_;
    }

    void SetString(char* s) {
      s_ = s;
    }
};


class ValueHolder {
  public:
  union {
    int64_t i;
    float f;
    double d;
    char* s;
  } value_;

  VH_TYPE type_;

  public:
    ValueHolder(double d) {
      value_.d = d;
      type_ = VH_DOUBLE;
    }

    ValueHolder(float f) {
      value_.f = f;
      type_ = VH_FLOAT;
    }

    ValueHolder(int64_t i) {
      value_.i = i;
      type_ = VH_INTEGER;
    }

    ValueHolder(int i) {
      value_.i = i;
      type_ = VH_INTEGER;
    }

    ValueHolder(char* s) {
      value_.s = s;
      type_ = VH_STRING;
    }

    int64_t GetInteger() const {
      assert(type_ != VH_STRING);
      switch (type_) {
        case VH_DOUBLE: {
          int64_t res = value_.d;
          return res;
        }
        case VH_FLOAT: {
          int64_t res = value_.f;
          return res;
        }
        default:
          break;
      }
      return value_.i;
    }

    float GetFloat() const {
      // This happens if we had a double from the parser that was actually in integer form.
      assert(type_ != VH_STRING);

      switch (type_) {
        case VH_INTEGER: {
          float res = value_.i;
          return res;
        }
        case VH_DOUBLE: {
          float res = value_.d;
          return res;
        }
        default:
          break;
      }
      return value_.f;
    }

    double GetDouble() const {
      // This happens if we had a double from the parser that was actually in integer form.
      assert(type_ != VH_STRING);

      switch (type_) {
        case VH_INTEGER: {
          double res = value_.i;
          return res;
        }
        case VH_FLOAT: {
          double res = value_.f;
          return res;
        }
        default:
          break;
      }
      return value_.d;
    }

    void Dump() {
      switch (type_) {
        case VH_INTEGER:
          BISON_PRINT("%ld", value_.i);
          break;
        case VH_FLOAT:
        case VH_DOUBLE:
          BISON_PRINT("%f (%lx)", value_.d, value_.i);
          break;
        case VH_STRING:
          BISON_PRINT("%s", value_.s);
          break;
      }
    }

    bool Convert(VH_TYPE dest) {
      // Only convert if we are in type string.
      if (type_ != VH_STRING) {
        return false;
      }

      switch (dest) {
        case VH_INTEGER:
          // Should never happen.
          assert(0);
          break;
        case VH_FLOAT: {
          char* s = value_.s;
          char* end = nullptr;
          value_.f = strtof(s, &end);
          assert(end != nullptr && (*end == ')' || *end == '\0'));

          // Nan is a bit special: strtof seems to not take the sign well.
          //   Also some implementations make strtof go for silent nans instead.
          if (strstr(s, "nan") != nullptr) {
            if (*s == '-') {
              value_.f *= -1;
            }

            // Get the ( character if it exists.
            char* ptr = strchr(s, '(');

            if (ptr != nullptr) {
              // Go to next character.
              ptr++;

              int64_t hex_value = strtol(ptr, &end, 16);

              assert(end != nullptr && *end ==  ')');

              // This is safe whether strtof did it or not so let us just do it.
              value_.i |= hex_value;
            }
          }

          type_ = VH_FLOAT;
        }
        break;

        case VH_DOUBLE: {
          char* s = value_.s;
          char* end = nullptr;
          value_.d = strtod(s, &end);
          assert(end != nullptr && (*end == ')' || *end == '\0'));

          // Nan is a bit special: strtof seems to not take the sign well.
          //   Also some implementations make strtof go for silent nans instead.
          if (strstr(s, "nan") != nullptr) {
            if (*s == '-') {
              value_.d *= -1;
            }

            // Get the ( character if it exists.
            char* ptr = strchr(s, '(');

            if (ptr != nullptr) {
              // Go to next character.
              ptr++;

              int64_t hex_value = strtol(ptr, &end, 16);

              assert(end != nullptr && *end ==  ')');

              // This is safe whether strtof did it or not so let us just do it.
              value_.i |= hex_value;
            }
          }
          type_ = VH_DOUBLE;
        }
        break;

        default:
         assert(0);
         break;
      }

      return true;
    }
};

class Segment {
  protected:
    int start_;
    char* data_;

  public:
    Segment(int start, char* data) :
      start_(start), data_(data) {
    }

    int GetStart() const {
      return start_;
    }

    char* GetData() const {
      return data_;
    }

    int GetLength() const {
      // This will change soon when we handle hex.
      return strlen(data_);
    }

    void Dump(int tab = 0) {
      BISON_TABBED_PRINT(tab, "Segment %d - %s\n", start_, data_);
    }
};

#endif

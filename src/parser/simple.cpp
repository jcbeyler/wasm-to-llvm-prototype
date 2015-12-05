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

#include "enums.h"
#include "simple.h"

bool ValueHolder::Convert(VH_TYPE dest) {
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

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

#include "driver.h"
#include "pass_driver.h"
#include "wasm_file.h"

void Driver::Drive() {
  PassDriver driver(file_);

  // First initialize the file data structures.
  file_->Initialize();

  // Run the driver of passes.
  driver.Drive();

  // Then generate the file code.
  file_->Generate();

  // Dump for debug.
  file_->Print();
}

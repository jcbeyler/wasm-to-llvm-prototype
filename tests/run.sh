#!/bin/bash

# Copyright (c) 2015 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

echo "Running tests"

list=`ls tests/*wast`
echo "Test list is:"
echo $list

exe=llvm_wasm

if [ ! -e $exe ]; then
  echo "Build llvm_wasm first"
  exit 1
fi

for f in $list; do
  echo $f

  # Clean up
  rm obj/*ll obj/*s

  # Build the llvm IR
  $exe $f

  if [ $? -ne 0 ]; then
    echo "LLVM transformation of $f failed. Bailing."
    exit 1
  fi

  # Create the .s files
  for f in obj/*ll; do
    llc-3.7 $f

    if [ $? -ne 0 ]; then
      echo "LLVM transformation of $f failed. Bailing."
      exit 1
    fi
  done

  # Create the test exec.
  g++ obj/wasm_module*s tests/main.cpp -o obj/testit -std=gnu++0x

  if [ $? -ne 0 ]; then
    echo "Build of test $f failed. Bailing."
    exit 1
  fi

  # Run the test.
  obj/testit

  if [ $? -ne 0 ]; then
    echo "Test failed: $f. Bailing."
    exit 1
  fi
done

echo "Tests passed"

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

echo "Running perf tests"

exe=llvm_wasm

if [ ! -e $exe ]; then
  echo "Build llvm_wasm first"
  exit 1
fi

for name in `ls -d perf_tests/*`; do
  if [ ! -d $name ]; then
    continue
  fi

  echo "Running perf test: $name"

  # Find the wast file.
  wast=`ls $name/*wast`
  echo $wast

  if [ ! -e $wast ]; then
    echo "Skipping $wast, does not exist"
  else
    # Clean up
    rm obj/*ll obj/*s

    # Build the llvm IR
    $exe $wast

    if [ $? -ne 0 ]; then
      echo "LLVM transformation of $wast failed. Bailing."
      exit 1
    fi

    # Create the .s files
    for ll in obj/*ll; do
      llc-3.7 $ll

      if [ $? -ne 0 ]; then
        echo "LLVM transformation of $ll failed. Bailing."
        exit 1
      fi
    done

    # Create the test exec.
    gcc -O3 obj/wasm_module*s perf_tests/driver.c $name/*c -o obj/testit -lrt

    if [ $? -ne 0 ]; then
      echo "Build of test $wast failed. Bailing."
      exit 1
    fi

    args=""
    if [ -e $name/args ]; then
      args=`cat $name/args`
    fi

    # Run the test.
    obj/testit $args

    if [ $? -ne 0 ]; then
      echo "Test failed: $wast. Bailing."
      exit 1
    fi
  fi
done

echo "Tests passed"

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

echo "Running compile time tests"
echo ""
echo "Using Gcc version `gcc -v 2>&1 | grep "gcc version"`"
echo ""
echo "Using Clang version `clang -v 2>&1 | grep "clang version"`"
echo ""

echo "Test name;Wasm2LLVM Time;GCC -O3 Time;Clang -O3 Time"


exe=llvm_wasm

if [ ! -e $exe ]; then
  echo "Build llvm_wasm first"
  exit 1
fi

dir=`find perf_tests -name "*wast" -printf "%h\n" | sort -u`
dir="$dir `find compile_tests -name "*wast" -printf "%h\n" | sort -u`"

if [ $# == 1 ]; then
  dir="compile_tests/$1"
fi

for name in $dir; do
    echo "$name"
    if [ ! -d $name ]; then
      continue
    fi

    # Find the Wasm file and the C file.
    wast=`ls $name/*wast`
    c="$name/`basename $wast wast`c"

    if [ ! -e $wast ]; then
      echo "Skipping $name, wasm file does not exist"
      continue
    fi

    if [ ! -e $c ]; then
      echo "Skipping $name, c file does not exist"
      continue
    fi

    wasm_time_out=obj/wasm_time
    c_gcc_time_out=obj/c_gcc_time
    c_clang_time_out=obj/c_clang_time

    # Compile both.
    /usr/bin/time -f "%E" -o $wasm_time_out sh -c "$exe $wast; llc-3.7 obj/wasm_module_0.ll"
    /usr/bin/time -f "%E" -o $c_gcc_time_out gcc -O3 $c -S -o obj/tmp.s
    /usr/bin/time -f "%E" -o $c_clang_time_out clang -O3 $c -S -o obj/tmp.s

    echo "$name;`cat $wasm_time_out`;`cat $c_gcc_time_out`;`cat $c_clang_time_out`"
done

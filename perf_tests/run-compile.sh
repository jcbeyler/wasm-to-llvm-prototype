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

for dir in branches-gen/branches-*; do
  wasm="wasm"
  gcc="gcc"
  clang="clang"
  i=0

  wasm="$wasm;$dir"
  gcc="$gcc;$dir"
  clang="$clang;$dir"
  while [ $i -lt 5 ]; do
    wasm="$wasm;`/usr/bin/time -f "%E" -p sh -c "./llvm_wasm $dir/branches.wast; llc-3.7 obj/wasm_module_0.ll" 2>&1 | grep real | cut -f 2 -d ' '`"
    gcc="$gcc;`/usr/bin/time -f "%E" gcc -O3 $dir/branches.c -S 2>&1`"
    clang="$clang;`/usr/bin/time -f "%E" clang -O3 $dir/branches.c -S 2>&1`"
    ((i++))
  done
  echo $wasm
  echo $gcc
  echo $clang
done

cd ../..

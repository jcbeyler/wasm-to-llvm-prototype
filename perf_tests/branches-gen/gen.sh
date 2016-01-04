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

num=$1

mkdir branches-$num
cd branches-$num

rm -rf *

cp ../runner.c ../args ../branches.h .

# C/Wasm part.
cat ../start.wast > branches.wast
cat ../start.c > branches.c

i=0
while [ $i -lt $num ]; do
  cat ../inter.wast >> branches.wast
  cat ../inter.c >> branches.c
  ((i++))
done


# Handle decrements.
echo "" >> branches.c
echo "    n -= $num;" >> branches.c

echo "" >> branches.wast
echo "      (set_local 0 (i32.sub (get_local 0) (i32.const $num)))" >> branches.wast

cat ../end.wast >> branches.wast
cat ../end.c >> branches.c

cd ..

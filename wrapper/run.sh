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

if [ $# -eq 0 ]; then
  list=`cat wrapper/supported | grep -v '#'`
else
  list="$@"
fi

echo "Test list is:"
echo $list

exe=${PWD}/llvm_wasm
our_log=obj/test_output
diff_file=obj/test_output_diff

if [ ! -e $exe ]; then
  echo "Build llvm_wasm first"
  exit 1
fi

for name in $list; do
  f="testsuite/$name"
  f_dir=`dirname $f`
  f_base=`basename $f`
  echo "Testing $f_base in $f_dir"

  if [ ! -e $f ]; then
    echo "Skipping $f, does not exist"
  else
    # Clean up
    rm obj/*ll obj/*s 2> /dev/null

    # Build the llvm IR
    $exe $f

    if [ $? -ne 0 ]; then
      echo "LLVM transformation of $f failed. Bailing."
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
    g++ obj/wasm_module*s wrapper/main.cpp src/print_line.cpp libwasm/* -o obj/testit -std=gnu++0x -Isrc

    if [ $? -ne 0 ]; then
      echo "Build of test $f failed. Bailing."
      exit 1
    fi

    # Run the test.
    obj/testit $f > $our_log

    if [ $? -ne 0 ]; then 
      echo "Test failed: $f. Bailing."
      exit 1
    fi

    # First check if we have a log in our separate folder.
    our_test_log=wrapper/expected-output/$f_base.log
    
    if [ -e $our_test_log ]; then
      echo "Using our log file $our_test_log"
      # Now check if there is an output.
      diff $our_test_log $our_log > $diff_file

      if [ $? -ne 0 ]; then
        echo "Expected output is not the same for test $f:"
        cat $diff_file
        exit 1
      fi
    else
      test_log=$f_dir/expected-output/$f_base.log

      if [ -e $test_log ]; then
          diff $test_log $our_log > $diff_file

          if [ $? -ne 0 ]; then
            echo "Expected output is not the same for test $f:"
            cat $diff_file
            exit 1
          fi
      fi
    fi
  fi
done

echo "Tests passed"

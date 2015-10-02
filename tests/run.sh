#!/bin/bash

echo "Running tests"

list=`ls tests/*wasm`
echo "Test list is:"
echo $list

exe=llvm_wasm

if [ ! -e $exe ]; then
  echo "Build llvm_wasm first"
  exit 1
fi

for f in $list; do
  echo $f

  # Build the llvm IR
  $exe $f 2> obj/test.ll
 
  # Create the .s
  llc-3.7 obj/test.ll

  if [ $? -ne 0 ]; then
    echo "LLVM transformation of $f failed. Bailing."
    exit 1
  fi

  # Create the test exec.
  g++ obj/test.s tests/main.cpp -o obj/testit -std=gnu++0x

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

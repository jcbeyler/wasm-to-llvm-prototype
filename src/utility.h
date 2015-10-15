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
#ifndef H_UTILITY
#define H_UTILITY

#include "llvm/IR/IRBuilder.h"

#include "enums.h"

/** Transform our enumerations to char* for dumping. */
const char* GetETypeName(ETYPE type);
const char* DumpOperation(OPERATION op);

// Conversion of enumeration type to LLVM type.
ETYPE ConvertTypeID2ETYPE(llvm::Type* type);
llvm::Type* ConvertType(ETYPE type);

// Code generation for type conversion.
llvm::Value* HandleTypeCasts(llvm::Value* value, llvm::Type* dest_type, llvm::IRBuilder<>& builder);

llvm::Value* HandleIntegerTypeCast(llvm::Value* value, llvm::Type* dest_type, int result_bw, int dest_bw, llvm::IRBuilder<>& builder);

char* AddWasmFunctionPrefix(const char* s);
#endif

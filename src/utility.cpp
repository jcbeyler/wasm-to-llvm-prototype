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
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

#include "utility.h"

const char* GetETypeName(ETYPE type) {
  switch (type) {
    case FLOAT_32:
      return "f32";
    case FLOAT_64:
      return "f64";
    case INT_32:
      return "i32";
    case INT_64:
      return "i64";
    case PTR_32:
      return "ptr32";
    case PTR_64:
      return "ptr64";
  }
  return "Unknown";
}

const char* DumpOperation(OPERATION op) {
  switch (op) {
    case EQ_OPER:
      return "eq";
    case NE_OPER:
      return "ne";
    case LE_OPER:
      return "le";
    case LT_OPER:
      return "lt";
    case GE_OPER:
      return "ge";
    case GT_OPER:
      return "gt";
    case DIV_OPER:
      return "div";
    case ADD_OPER:
      return "add";
    case SUB_OPER:
      return "sub";
    case MUL_OPER:
      return "mul";
    case REM_OPER:
      return "rem";
    case AND_OPER:
      return "and";
    case OR_OPER:
      return "or";
    case XOR_OPER:
      return "xor";
    case SHL_OPER:
      return "shl";
    case SHR_OPER:
      return "shr";
    case CLZ_OPER:
      return "clz";
    case POPCNT_OPER:
      return "popcnt";
    case NEG_OPER:
      return "neg";
    case ABS_OPER:
      return "abs";
    case CEIL_OPER:
      return "ceil";
    case FLOOR_OPER:
      return "floor";
    case MIN_OPER:
      return "min";
    case MAX_OPER:
      return "max";
    case SQRT_OPER:
      return "sqrt";
    case TRUNC_OPER:
      return "trunc";
    case NEAREST_OPER:
      return "nearest";
    case COPYSIGN_OPER:
      return "copysign";
  }

  return "Unknown";
}

llvm::Type* ConvertType(ETYPE type) {
  switch (type) {
    case FLOAT_32:
      return llvm::Type::getFloatTy(llvm::getGlobalContext());
    case FLOAT_64:
      return llvm::Type::getDoubleTy(llvm::getGlobalContext());
    case INT_32:
      return llvm::Type::getInt32Ty(llvm::getGlobalContext());
    case INT_64:
      return llvm::Type::getInt64Ty(llvm::getGlobalContext());
    case PTR_32:
      return llvm::Type::getInt32PtrTy(llvm::getGlobalContext());
    case PTR_64:
      return llvm::Type::getInt64PtrTy(llvm::getGlobalContext());
  }
  return llvm::Type::getVoidTy(llvm::getGlobalContext());
}

llvm::Value* HandleTypeCasts(llvm::Value* value, llvm::Type* dest_type, llvm::IRBuilder<>& builder) {
  // Major issue here is casting if need be.
  // First: Get the result and function types.
  llvm::Type* result_type = value->getType();

  llvm::Type::TypeID result_type_id = result_type->getTypeID();
  llvm::Type::TypeID dest_type_id = dest_type->getTypeID();

  // This will move after.
  if (dest_type_id != result_type_id) {
    fprintf(stderr, "%d - %d\n", dest_type_id, result_type_id);
    assert(0);
    return nullptr;
  } else {
    // A bit more code to handle integer case
    if (dest_type_id == llvm::Type::IntegerTyID) {
      // Now check sizes.
      int dest_type_bw = dest_type->getIntegerBitWidth();
      int result_type_bw = result_type->getIntegerBitWidth();

      // We have a problem here.
      if (dest_type_bw != result_type_bw) {
        // Handle difference of sizes.
        value = HandleIntegerTypeCast(value, dest_type, result_type_bw, dest_type_bw, builder);
      }
    }

    // Now create the result.
    return builder.CreateRet(value);
  }
}

llvm::Value* HandleIntegerTypeCast(llvm::Value* value, llvm::Type* dest_type, int result_bw, int dest_bw, llvm::IRBuilder<>& builder) {
  if (result_bw < dest_bw) {
    // For now, we are assuming a zext is sufficient, it probably isn't :)... TODO
    return builder.CreateZExt(value, dest_type, "zext_tmp");
  }
  return nullptr;
}

ETYPE ConvertTypeID2ETYPE(llvm::Type* type) {
  llvm::Type::TypeID type_id = type->getTypeID();

  // TODO: incomplete for now.
  switch(type_id) {
    case llvm::Type::VoidTyID:
      return VOID;
    case llvm::Type::FloatTyID:
      return FLOAT_32;
    case llvm::Type::DoubleTyID:
      return FLOAT_64;
    case llvm::Type::IntegerTyID:
      switch (type->getIntegerBitWidth()) {
        case 32:
          return INT_32;
        case 64:
          return INT_64;
      }
  }

  // Should not get here.
  assert(0);
  return VOID;
}

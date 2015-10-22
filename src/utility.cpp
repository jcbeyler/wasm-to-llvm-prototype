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

#include <sstream>

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"

#include "debug.h"
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

size_t GetTypeSize(ETYPE type) {
  switch (type) {
    case FLOAT_32:
    case INT_32:
      return 32;
    case FLOAT_64:
    case INT_64:
      return 64;
    default:
      assert(0);
  }
  return 0;
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
    case EXTEND_OPER:
      return "extend";
    case WRAP_OPER:
      return "wrap";
    case DEMOTE_OPER:
      return "demote";
    case REINTERPRET_OPER:
      return "reinterpret";
    case PROMOTE_OPER:
      return "demote";
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

static llvm::Value* HandleTypeCastsFromFloats(llvm::Value* value, llvm::Type* dest_type, bool sign, llvm::IRBuilder<>& builder) {
  llvm::Type::TypeID dest_type_id = dest_type->getTypeID();

  switch (dest_type_id) {
    case llvm::Type::IntegerTyID: {
      // Let us check we have the right size though.
      int dest_type_bw = dest_type->getIntegerBitWidth();
      llvm::Type* integer_type = dest_type;

      if (dest_type_bw != 32) {
        // First go to 32-bit integer.
        integer_type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
      }

      if (sign) {
        value = builder.CreateFPToSI(value, dest_type, "fptosi");
      } else {
        value = builder.CreateFPToUI(value, dest_type, "fptoui");
      }

      if (dest_type_bw != 32) {
        // Now go to the right size.
        value = HandleIntegerTypeCast(value, dest_type, 32, 64, sign, builder);
      }
      return value;
    }
    case llvm::Type::DoubleTyID:
      return builder.CreateFPExt(value, dest_type, "fpext");
    default:
      assert(0);
  }

  return nullptr;
}

static llvm::Value* HandleTypeCastsFromDoubles(llvm::Value* value, llvm::Type* dest_type, bool sign, llvm::IRBuilder<>& builder) {
  llvm::Type::TypeID dest_type_id = dest_type->getTypeID();

  switch (dest_type_id) {
    case llvm::Type::IntegerTyID: {
      // Let us check we have the right size though.
      int dest_type_bw = dest_type->getIntegerBitWidth();
      llvm::Type* integer_type = dest_type;

      if (dest_type_bw != 64) {
        // First go to 32-bit integer.
        integer_type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
      }

      if (sign) {
        value = builder.CreateFPToSI(value, dest_type, "fptosi");
      } else {
        value = builder.CreateFPToUI(value, dest_type, "fptoui");
      }

      if (dest_type_bw != 64) {
        // Now go to the right size.
        value = HandleIntegerTypeCast(value, dest_type, 64, 32, sign, builder);
      }
      return value;
    }
    case llvm::Type::FloatTyID:
      return builder.CreateFPTrunc(value, dest_type, "fptrunc");
    default:
      assert(0);
  }
  return nullptr;
}

static llvm::Value* HandleTypeCastsFromIntegers(llvm::Value* value, int src_type_bw, llvm::Type* dest_type, bool sign, llvm::IRBuilder<>& builder) {
  llvm::Type::TypeID dest_type_id = dest_type->getTypeID();

  switch (dest_type_id) {
    case llvm::Type::IntegerTyID:
      // We should never come here...
      return HandleIntegerTypeCast(value, dest_type, src_type_bw, dest_type->getIntegerBitWidth(), sign, builder);
      break;
    case llvm::Type::FloatTyID:
      // Not sure if this is required but if it does not fit in 32-bit,
      //   go to double first.
      if (src_type_bw != 32) {
        value = HandleTypeCastsFromIntegers(value, src_type_bw,
                                            llvm::Type::getDoubleTy(llvm::getGlobalContext()),
                                            sign, builder);
        return HandleTypeCastsFromDoubles(value, dest_type, sign, builder);
      }

      if (sign == true) {
        return builder.CreateSIToFP(value, dest_type, "sitofp");
      } else {
        return builder.CreateUIToFP(value, dest_type, "uitofp");
      }
    case llvm::Type::DoubleTyID:
      if (sign == true) {
        return builder.CreateSIToFP(value, dest_type, "sitofp");
      } else {
        return builder.CreateUIToFP(value, dest_type, "uitofp");
      }
    default:
      assert(0);
  }
  return nullptr;
}

llvm::Value* HandleTypeCasts(llvm::Value* value, llvm::Type* src_type, llvm::Type* dest_type, bool sign, llvm::IRBuilder<>& builder) {
  if (dest_type != src_type) {
    llvm::Type::TypeID src_type_id = src_type->getTypeID();

    switch (src_type_id) {
      case llvm::Type::IntegerTyID: {
        int src_type_bw = src_type->getIntegerBitWidth();
        return HandleTypeCastsFromIntegers(value, src_type_bw, dest_type, sign, builder);
      }
      case llvm::Type::FloatTyID:
        return HandleTypeCastsFromFloats(value, dest_type, sign, builder);
      case llvm::Type::DoubleTyID:
        return HandleTypeCastsFromDoubles(value, dest_type, sign, builder);
      case llvm::Type::PointerTyID:
        // Probably not good to do this.
        return value;
      default: {
        llvm::Type::TypeID dest_type_id = dest_type->getTypeID();
        BISON_PRINT("HandleTypeCast failure: destination is %d and src is %d\n", dest_type_id, src_type_id);
        assert(0);
        break;
      }
    }
  } else {
    // A bit more code to handle integer case
    llvm::Type::TypeID dest_type_id = dest_type->getTypeID();
    if (dest_type_id == llvm::Type::IntegerTyID) {
      // Now check sizes.
      int dest_type_bw = dest_type->getIntegerBitWidth();
      int src_type_bw = src_type->getIntegerBitWidth();

      // We have a problem here.
      if (dest_type_bw != src_type_bw) {
        // Handle difference of sizes.
        value = HandleIntegerTypeCast(value, dest_type, src_type_bw, dest_type_bw, sign, builder);
      }
    }

    return value;
  }
}

llvm::Value* HandleSimpleTypeCasts(llvm::Value* value, llvm::Type* dest_type, bool sign, llvm::IRBuilder<>& builder) {
  // Major issue here is casting if need be.
  // First: Get the result and function types.
  llvm::Type* src_type = value->getType();
  return HandleTypeCasts(value, src_type, dest_type, sign, builder);
}

llvm::Value* HandleIntegerTypeCast(llvm::Value* value, llvm::Type* dest_type, int src_bw, int dest_bw, bool sign, llvm::IRBuilder<>& builder) {
  if (src_bw < dest_bw) {
    if (sign) {
      return builder.CreateSExt(value, dest_type, "sext_tmp");
    } else {
      return builder.CreateZExt(value, dest_type, "zext_tmp");
    }
  } else {
    if (src_bw == dest_bw) {
      // Corner case where nothing needs to be done.
      return value;
    } else {
      return builder.CreateTrunc(value, dest_type, "trunc_tmp");
    }
  }
  return nullptr;
}

ETYPE ConvertTypeID2ETYPE(llvm::Type* type) {
  llvm::Type::TypeID type_id = type->getTypeID();

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
        default:
          // Should never get here.
          assert(0);
          break;
      }
  }

  // Should not get here.
  assert(0);
  return VOID;
}

char* AddWasmFunctionPrefix(const char* s) {
  const char* prefix = "wp_";
  std::ostringstream oss;
  oss << prefix << s;
  return strdup(oss.str().c_str());
}

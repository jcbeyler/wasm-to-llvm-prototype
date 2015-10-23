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

#include "memory.h"
#include "function.h"

llvm::Type* MemoryExpression::GetAddressType() const {
  llvm::Type* type = nullptr;

  if (type_ == INT_32 || type_ == INT_64) {
    switch (size_) {
      case 8:
        type = llvm::Type::getInt8Ty(llvm::getGlobalContext());
        break;
      case 16:
        type = llvm::Type::getInt16Ty(llvm::getGlobalContext());
        break;
      case 32:
        type = llvm::Type::getInt32Ty(llvm::getGlobalContext());
        break;
      case 64:
        type = llvm::Type::getInt64Ty(llvm::getGlobalContext());
        break;
      default:
        assert(0);
        break;
    }
  } else {
    switch (size_) {
      case 32:
        type = llvm::Type::getFloatTy(llvm::getGlobalContext());
        break;
      case 64:
        type = llvm::Type::getDoubleTy(llvm::getGlobalContext());
        break;
      default:
        assert(0);
        break;
    }
  }

  return type;
}

llvm::Value* MemoryExpression::GetPointer(WasmFunction*fct, llvm::IRBuilder<>& builder) const {
  llvm::Value* address_i = address_->Codegen(fct, builder);
  llvm::Type* type = nullptr;

  if (type_ == INT_32 || type_ == INT_64) {
    switch (size_) {
      case 8:
        type = llvm::Type::getInt8PtrTy(llvm::getGlobalContext());
        break;
      case 16:
        type = llvm::Type::getInt16PtrTy(llvm::getGlobalContext());
        break;
      case 32:
        type = llvm::Type::getInt32PtrTy(llvm::getGlobalContext());
        break;
      case 64:
        type = llvm::Type::getInt64PtrTy(llvm::getGlobalContext());
        break;
      default:
        assert(0);
        break;
    }
  } else {
    switch (size_) {
      case 32:
        type = llvm::Type::getFloatPtrTy(llvm::getGlobalContext());
        break;
      case 64:
        type = llvm::Type::getDoublePtrTy(llvm::getGlobalContext());
        break;
      default:
        assert(0);
        break;
    }
  }

  assert(type != nullptr);

  // Create the base address in the same right type.
  llvm::Value* local_base = fct->GetLocalBase();
  local_base = builder.CreatePtrToInt(local_base, address_i->getType(), "base");

  address_i = builder.CreateAdd(address_i, local_base, "add_with_offset");

  return builder.CreateIntToPtr(address_i, type, "ptr");
}

llvm::Value* Load::ResizeIntegerIfNeed(llvm::Value* value,
                                        llvm::Type* value_type,
                                        ETYPE destination_type,
                                        bool sign,
                                        llvm::IRBuilder<>& builder) {
  // Get size differences.
  int value_type_bw = value_type->getIntegerBitWidth();
  size_t type_size = GetTypeSize(type_);

  if (value_type_bw != type_size) {
    // Then it depends on sign.
    value = HandleIntegerTypeCast(value, ConvertType(type_),
                                  value_type_bw, 
                                  type_size,
                                  sign, builder);
  }

  return value;
}

llvm::Value* Load::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  llvm::Value* address = GetPointer(fct, builder);

  // Create the load.
  llvm::Value* value = builder.CreateLoad(address, "load");

  // Now we need to convert this load to a size potentially.
  switch (type_) {
    case INT_32:
    case INT_64:
      value = ResizeIntegerIfNeed(value, value->getType(), type_, sign_, builder);
      break;
    case FLOAT_32:
    case FLOAT_64:
      assert(GetTypeSize(type_) == size_);
      break;
    default:
      assert(0);
      break;
  }

  return value;
}

llvm::Value* Store::ResizeIntegerIfNeed(llvm::Value* value,
                                        llvm::Type* value_type,
                                        llvm::Type* address_type,
                                        bool sign,
                                        llvm::IRBuilder<>& builder) {
  // Get size differences.
  int value_type_bw = value_type->getIntegerBitWidth();

  if (value_type_bw != size_) {
    // Then it depends on sign.
    value = HandleIntegerTypeCast(value, GetAddressType(),
                                  value_type_bw, size_,
                                  sign, builder);
  }

  return value;
}

llvm::Value* Store::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  llvm::Value* address = GetPointer(fct, builder);
  llvm::Value* value = value_->Codegen(fct, builder);

  // Check if the type of what we are storing is the same type as what we have like size.
  llvm::Type* address_type = address->getType();
  llvm::Type* value_type = value->getType();

  llvm::Type::TypeID address_type_id = address_type->getTypeID();
  llvm::Type::TypeID value_type_id = value_type->getTypeID();

  if (value_type_id == llvm::Type::IntegerTyID) {
    value = ResizeIntegerIfNeed(value, value_type, address_type, sign_, builder);
  } else {
    assert(GetTypeSize(type_) == size_);
  }

  return builder.CreateStore(value, address, "store");
}

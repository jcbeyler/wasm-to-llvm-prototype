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
#include "llvm/IR/Module.h"
#include "llvm/IR/Intrinsics.h"

#include "expression.h"
#include "function.h"
#include "module.h"

llvm::Value* Unop::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  bool is_intrinsic = false;
  OPERATION op = operation_->GetOperation();

  switch (op) {
    case CLZ_OPER:
    case CTZ_OPER:
    case POPCNT_OPER:
    case SQRT_OPER:
    case ABS_OPER:
    case CEIL_OPER:
    case FLOOR_OPER:
    case NEAREST_OPER:
      is_intrinsic = true;
      break;
    case TRUNC_OPER: {
        // Use the intrinsic value if we have the same type.
        //   Basically this is used for f32.trunc or f64.trunc...
        ConversionOperation* conversion = dynamic_cast<ConversionOperation*>(operation_);
        assert(conversion != nullptr);
        is_intrinsic = (conversion->GetSrc() == conversion->GetDest());
        break;
    }
    default:
      break;
  }

  if (is_intrinsic == true) {
    WasmModule* wasm_module = fct->GetModule();
    ETYPE type = operation_->GetType();
    llvm::Intrinsic::ID intrinsic;

    bool extra_true_arg = false;

    switch (op) {
      case CLZ_OPER:
        intrinsic = llvm::Intrinsic::ctlz;
        // We need to pass true as second argument to produce defined result for zero.
        extra_true_arg = true;
        break;
      case CTZ_OPER:
        intrinsic = llvm::Intrinsic::cttz;
        // We need to pass true as second argument to produce defined result for zero.
        extra_true_arg = true;
        break;
      case POPCNT_OPER:
        intrinsic = llvm::Intrinsic::ctpop;
        break;
      case SQRT_OPER:
        intrinsic = llvm::Intrinsic::sqrt;
        break;
      case ABS_OPER:
        intrinsic = llvm::Intrinsic::fabs;
        break;
      case CEIL_OPER:
        intrinsic = llvm::Intrinsic::ceil;
        break;
      case FLOOR_OPER:
        intrinsic = llvm::Intrinsic::floor;
        break;
      case TRUNC_OPER:
        intrinsic = llvm::Intrinsic::trunc;
        break;
      case NEAREST_OPER:
        intrinsic = llvm::Intrinsic::nearbyint;
        break;
      default:
        assert(0);
        return nullptr;
    }

    llvm::Function* intrinsic_fct = wasm_module->GetOrCreateIntrinsic(intrinsic, type);
    assert(intrinsic_fct != nullptr);

    std::vector<Value*> arg;
    arg.push_back(only_->Codegen(fct, builder));

    if (extra_true_arg) {
      llvm::Value* val_true = llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(1, 0, false));
      arg.push_back(val_true);
    }

    return builder.CreateCall(intrinsic_fct, arg, "calltmp");
  } else {
    llvm::Value* rv = only_->Codegen(fct, builder);
    ETYPE type = operation_->GetType();

    switch (op) {
      case NEG_OPER: {
        llvm::Value* lv;
        if (type == FLOAT_32) {
          lv = llvm::ConstantFP::get(llvm::getGlobalContext(), APFloat(0.0f));
        } else {
          lv = llvm::ConstantFP::get(llvm::getGlobalContext(), APFloat(0.0));
        }
        return builder.CreateFSub(lv, rv, "subtmp");
      }

      case REINTERPRET_OPER:
        return builder.CreateBitCast(rv, ConvertType(type), DumpOperation(op));

      case EXTEND_OPER:
      case TRUNC_OPER:
      case PROMOTE_OPER:
      case DEMOTE_OPER:
      case CONVERT_OPER:
      case WRAP_OPER: {
        ConversionOperation* conversion = dynamic_cast<ConversionOperation*>(operation_);
        assert(conversion != nullptr);
        return HandleTypeCasts(rv, ConvertType(conversion->GetSrc()), ConvertType(type), operation_->GetSignedOrOrdered(), builder);
      }
    }

    assert(0);
    return nullptr;
  }
}


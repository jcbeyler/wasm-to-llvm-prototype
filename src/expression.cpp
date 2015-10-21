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

using namespace llvm;

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
        is_intrinsic = (conversion->GetSrc() == conversion->GetDest());
        break;
    }
    default:
      break;
  }

  if (is_intrinsic == true) {
    WasmModule* wasm_module = fct->GetModule();
    llvm::Module* module = wasm_module->GetModule();
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
      case REINTERPRET_OPER: {
        return builder.CreateBitCast(rv, ConvertType(type), DumpOperation(op));
      }
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
      default:
        assert(0);
        return nullptr;
    }
  }
}

ETYPE Binop::HandleType(ETYPE type, llvm::Type* lt, llvm::Type* rt) {
  assert(lt == rt);

  // If type is not void, the type should be the same as lt.
  if (type != VOID) {
    ETYPE rt_type = ConvertTypeID2ETYPE(rt);
    assert(ConvertType(type) == rt);
  }

  // Return lt's type.
  return ConvertTypeID2ETYPE(rt);
}

llvm::Value* Binop::HandleInteger(llvm::Value* lv, llvm::Value* rv, llvm::IRBuilder<>& builder) {
  OPERATION op = operation_->GetOperation();
  bool is_signed = operation_->GetSignedOrOrdered();

  // TODO: supposing we are in 64 bit, I'm not yet playing with 32-bit here.
  switch (op) {
    case EQ_OPER:
      return builder.CreateICmpEQ(lv, rv, "cmptmp");
    case NE_OPER:
      return builder.CreateICmpNE(lv, rv, "cmptmp");
    case LE_OPER:
      if (!is_signed) {
        return builder.CreateICmpULE(lv, rv, "cmptmp");
      } else {
        return builder.CreateICmpSLE(lv, rv, "cmptmp");
      }
    case LT_OPER:
      if (!is_signed) {
        return builder.CreateICmpULT(lv, rv, "cmptmp");
      } else {
        return builder.CreateICmpSLT(lv, rv, "cmptmp");
      }
    case GE_OPER:
      if (!is_signed) {
        return builder.CreateICmpUGE(lv, rv, "cmptmp");
      } else {
        return builder.CreateICmpSGE(lv, rv, "cmptmp");
      }
    case GT_OPER:
      if (!is_signed) {
        return builder.CreateICmpUGT(lv, rv, "cmptmp");
      } else {
        return builder.CreateICmpSGT(lv, rv, "cmptmp");
      }
    case DIV_OPER:
      if (!is_signed) {
        return builder.CreateUDiv(lv, rv, "cmptmp");
      } else {
        return builder.CreateSDiv(lv, rv, "cmptmp");
      }
      return builder.CreateUDiv(lv, rv, "divtmp");
    case ADD_OPER:
      return builder.CreateAdd(lv, rv, "addtmp");
    case SUB_OPER:
      return builder.CreateSub(lv, rv, "subtmp");
    case MUL_OPER:
      return builder.CreateMul(lv, rv, "multmp");
    case AND_OPER:
      return builder.CreateAnd(lv, rv, "andtmp");
    case OR_OPER:
      return builder.CreateOr(lv, rv, "ortmp");
    case XOR_OPER:
      return builder.CreateXor(lv, rv, "xortmp");
    case SHL_OPER:
    case SHR_OPER:
    case REM_OPER:
      // We should not arrive here anymore.
      assert(0);
      break;
    default:
      BISON_PRINT("Operation not supported\n");
      assert(0);
      break;
  }

  return nullptr;
}

// TODO: Move this code and the shift handler somewhere else and perhaps factorize a bit more the code generation.
class ReallyDivRem : public Binop {
  protected:
    bool div_;
    bool sign_;

  public:
    ReallyDivRem(Expression* l, Expression* r, bool div = true, bool sign = false) :
      Binop(new Operation(div ? DIV_OPER : REM_OPER, sign), l, r), div_(div), sign_(sign) {
      }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
      OPERATION op = operation_->GetOperation();

      assert(left_ != nullptr && right_ != nullptr);
      Value* lv = left_->Codegen(fct, builder);
      Value* rv = right_->Codegen(fct, builder);

      if (lv == nullptr || rv == nullptr) {
        return nullptr;
      }

      // Get the type.
      ETYPE old_type = operation_->GetType();

      // If it's void, we have a bit more work to do.
      llvm::Type* lt = lv->getType();
      llvm::Type* rt = rv->getType();

      ETYPE type = HandleType(old_type, lt, rt);

      if (old_type != type) {
        operation_->SetType(type);
      }

      if (div_ == false) {
        if (!sign_) {
          return builder.CreateURem(lv, rv, "rem");
        } else {
          return builder.CreateSRem(lv, rv, "rem");
        }
      } else {
        if (!sign_) {
          return builder.CreateUDiv(lv, rv, "div");
        } else {
          return builder.CreateSDiv(lv, rv, "div");
        }
      }
    }
};

// Here we have a local version of an Expression class to handle the special case of SHR.
class ReallyShift : public Binop {
  protected:
    bool is_right_shift_;

  public:
    ReallyShift(Expression* l, Expression* r, bool right = true, bool sign = false) :
      Binop(new Operation(right ? SHR_OPER : SHL_OPER, sign), l, r), is_right_shift_(right) {
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
      OPERATION op = operation_->GetOperation();

      assert(left_ != nullptr && right_ != nullptr);
      Value* lv = left_->Codegen(fct, builder);
      Value* rv = right_->Codegen(fct, builder);

      if (lv == nullptr || rv == nullptr) {
        return nullptr;
      }

      // Get the type.
      ETYPE old_type = operation_->GetType();

      // If it's void, we have a bit more work to do.
      llvm::Type* lt = lv->getType();
      llvm::Type* rt = rv->getType();

      ETYPE type = HandleType(old_type, lt, rt);

      if (old_type != type) {
        operation_->SetType(type);
      }

      bool is_signed = operation_->GetSignedOrOrdered();

      if (is_right_shift_) {
        if (!is_signed) {
          return builder.CreateLShr(lv, rv, "lshr");
        } else {
          return builder.CreateAShr(lv, rv, "ashr");
        }
      } else {
        return builder.CreateShl(lv, rv, "shl");
      }
    }
};

llvm::Value* Binop::HandleShift(WasmFunction* fct, llvm::IRBuilder<>& builder, bool sign, bool right) {
  // We have more work to do here: check if we are above the limit.
  // Webassembly spec says: if the shift amount is above the width, then
  //   we need to explicitly put 0 here.
  // Also, we interpret the rv value as unsigned, but let's be honest:
  //   if it is negative, that means it's huge, and just set to 0.
  // Therefore, the solution that is the simplest is to just test if it is above the width:
  //    If it is negative, since we consider unsigned, it will be bigger than 32 or 64
  //    Otherwise we need to handle it.
  ETYPE type = operation_->GetType();
  int left_width = (type == INT_32) ? 32 : 64;

  // Assertion: left shift is unsigned.
  assert(right == true || sign == false);

  // shr value, amount: depends on signed or unsigned.
  if (!sign) {
    // In the case of unsigned, we do:
    /*
     *   // Testing unsigned too big or negative in one step.
     *   if (amount >= value's width)
     *    0
     *   else
     *    really do the shift
     */
    // TODO handle 64.
    ValueHolder* vh = new ValueHolder(left_width);
    Const* width = new Const(type, vh);

    Operation* op = new Operation(GE_OPER, false, type);
    Binop* cond = new Binop(op, right_, width);

    vh = new ValueHolder(left_width - 1);
    Const* width_minus_one = new Const(type, vh);

    vh = new ValueHolder(0);
    Const* zero = new Const(type, vh);

    // False path here goes back to right_ here.
    ReallyShift* rsr = new ReallyShift(left_, right_, right);

    IfExpression* width_check = new IfExpression(cond, zero, rsr);

    // Now generate the code.
    width_check->Codegen(fct, builder);
  } else {
    /*
     *  // Testing unsigned too big or negative in one step.
     *   if (amount >= value's width)
     *    shr value, 31
     *   else
     *    really do the shift
     */
    ValueHolder* vh = new ValueHolder(left_width);
    Const* width = new Const(type, vh);

    Operation* op = new Operation(GE_OPER, false, type);
    Binop* cond = new Binop(op, right_, width);

    vh = new ValueHolder(left_width - 1);
    Const* width_minus_one = new Const(type, vh);

    // False path here goes back to right_ here.
    ReallyShift* rsr = new ReallyShift(left_, width_minus_one, true, true);

    // Create the actual shift.
    ReallyShift* original_rsr = new ReallyShift(left_, right_, true, true);

    // False path here goes back to the real shift here.
    IfExpression* width_check = new IfExpression(cond, rsr, original_rsr);

    // Now generate the code.
    width_check->Codegen(fct, builder);
  }
}

llvm::Value* Binop::HandleDivRem(WasmFunction* fct, llvm::IRBuilder<>& builder, bool sign, bool div) {
  // We have more work to do here: we have a special case for 0x80000000 with -1.
  // TODO: handle 64-bit
  ETYPE type = operation_->GetType();

  ValueHolder* vh;

  if (type == INT_32) {
    vh = new ValueHolder((int) 0x80000000);
  } else {
    vh = new ValueHolder((int64_t) 0x8000000000000000L);
  }
  Const* max = new Const(type, vh);

  Operation* op = new Operation(EQ_OPER, true, type);
  Binop* cond = new Binop(op, left_, max);

  Expression* left;
  // TODO: when rereading the spec, this does not seem right for signed divide; even rem I'm not sure.
  if (div) {
    if (sign) {
      ReallyDivRem* rdr = new ReallyDivRem(left_, right_, div, sign);
      if (type == INT_32) {
        vh = new ValueHolder(0x7fffffff);
      } else {
        vh = new ValueHolder(0x7fffffffffffffffL);
      }
      left = new Const(type, vh);
    } else {
      vh = new ValueHolder(0);
      left = new Const(type, vh);
    }
  } else {
    if (sign) {
      vh = new ValueHolder(0);
      left = new Const(type, vh);
    } else {
      if (type == INT_32) {
        vh = new ValueHolder((int) 0x80000000);
      } else {
        vh = new ValueHolder((int64_t) 0x8000000000000000L);
      }
      left = new Const(type, vh);
    }
  }

  ReallyDivRem* rdr = new ReallyDivRem(left_, right_, div, sign);
  IfExpression* left_test = new IfExpression(cond, left, rdr);

  // Now we generate the test on the right side: is it -1?
  vh = new ValueHolder(-1);
  Const* minus_one = new Const(type, vh);

  op = new Operation(EQ_OPER, true, type);
  cond = new Binop(op, right_, minus_one);

  rdr = new ReallyDivRem(left_, right_, div, sign);
  IfExpression* full_test = new IfExpression(cond, left_test, rdr);

  // Now generate code.
  full_test->Codegen(fct, builder);
}

llvm::Value* Binop::HandleIntrinsic(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  WasmModule* wasm_module = fct->GetModule();
  llvm::Module* module = wasm_module->GetModule();
  ETYPE type = operation_->GetType();
  llvm::Intrinsic::ID intrinsic;
  OPERATION op = operation_->GetOperation();

  switch (op) {
    case MIN_OPER:
      intrinsic = llvm::Intrinsic::minnum;
      break;
    case MAX_OPER:
      intrinsic = llvm::Intrinsic::maxnum;
      break;
    case COPYSIGN_OPER:
      intrinsic = llvm::Intrinsic::copysign;
      break;
    default:
      break;
  }

  llvm::Function* intrinsic_fct = wasm_module->GetOrCreateIntrinsic(intrinsic, type);

  assert (intrinsic_fct != nullptr);

  // Handle paramters.
  std::vector<Value*> args;
  args.push_back(left_->Codegen(fct, builder));
  args.push_back(right_->Codegen(fct, builder));

  return builder.CreateCall(intrinsic_fct, args, "calltmp");
}

llvm::Value* Binop::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // TODO factorize this with the ReallyShift version.
  OPERATION op = operation_->GetOperation();
  ETYPE type = operation_->GetType();

  // We have a couple of exception and special handling:
  switch (op) {
    case SHR_OPER:
    case SHL_OPER: {
      // In case we have a SHR, we have work due to a difference between
      //   WASM and LLVM decisions on shift count handling.
        bool right = (op == SHR_OPER);
        // Right shift is the only one that can be signed.
        bool sign = (right == true && operation_->GetSignedOrOrdered());
        return HandleShift(fct, builder, sign, right);
    }

    case REM_OPER:
    case DIV_OPER: {
      if (type == INT_32 || type == INT_64) {
        // In case we have a SHR, we have work due to a difference between
        //   WASM and LLVM decisions on shift count handling.
        bool div = (op == DIV_OPER);
        // Right shift is the only one that can be signed.
        bool sign = operation_->GetSignedOrOrdered();
        return HandleDivRem(fct, builder, sign, div);
      }
      break;
    }
    case MAX_OPER:
    case MIN_OPER:
    case COPYSIGN_OPER:
      // Handle binop intrinsic.
      return HandleIntrinsic(fct, builder);
    default:
      break;
  }

  assert(left_ != nullptr && right_ != nullptr);
  Value* lv = left_->Codegen(fct, builder);
  Value* rv = right_->Codegen(fct, builder);

  if (lv == nullptr || rv == nullptr) {
    return nullptr;
  }

  // Get the type.
  ETYPE old_type = type;

  // If it's void, we have a bit more work to do.
  llvm::Type* lt = lv->getType();
  llvm::Type* rt = rv->getType();

  type = HandleType(old_type, lt, rt);

  if (old_type != type) {
    operation_->SetType(type);
  }

  if (type == FLOAT_32 || type == FLOAT_64) {
    bool ordered = operation_->GetSignedOrOrdered();
    switch (op) {
      case LT_OPER:
        if (ordered) {
          return builder.CreateFCmpOLT(lv, rv, "cmptmp");
        } else {
          return builder.CreateFCmpULT(lv, rv, "cmptmp");
        }
      case GT_OPER:
        if (ordered) {
          return builder.CreateFCmpOGT(lv, rv, "cmptmp");
        } else {
          return builder.CreateFCmpUGT(lv, rv, "cmptmp");
        }
      case LE_OPER:
        if (ordered) {
          return builder.CreateFCmpOLE(lv, rv, "cmptmp");
        } else {
          return builder.CreateFCmpULE(lv, rv, "cmptmp");
        }
      case GE_OPER:
        if (ordered) {
          return builder.CreateFCmpOGE(lv, rv, "cmptmp");
        } else {
          return builder.CreateFCmpUGE(lv, rv, "cmptmp");
        }
      case NE_OPER:
        if (ordered) {
          return builder.CreateFCmpONE(lv, rv, "cmptmp");
        } else {
          return builder.CreateFCmpUNE(lv, rv, "cmptmp");
        }
      case EQ_OPER:
        if (ordered) {
          return builder.CreateFCmpOEQ(lv, rv, "cmptmp");
        } else {
          return builder.CreateFCmpUEQ(lv, rv, "cmptmp");
        }
      case DIV_OPER:
        return builder.CreateFDiv(lv, rv, "divtmp");
      case ADD_OPER:
        return builder.CreateFAdd(lv, rv, "addtmp");
      case SUB_OPER:
        return builder.CreateFSub(lv, rv, "subtmp");
      case MUL_OPER:
        return builder.CreateFMul(lv, rv, "multmp");
      default:
        BISON_PRINT("Operation not supported\n");
        assert(0);
        return nullptr;
    }
  } else {
    return HandleInteger(lv, rv, builder);
  }

  return nullptr;
}

llvm::Value* GetLocal::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  llvm::AllocaInst* alloca = nullptr; 
  const char* name = nullptr;

  if (var_->IsString()) {
    name = var_->GetString();
    alloca = fct->GetVariable(name);
  } else {
    size_t idx = var_->GetIdx();
    std::ostringstream oss;
    oss << "var_idx_" << idx;
    name = oss.str().c_str();
    alloca = fct->GetVariable(idx);
  }

  assert(alloca != nullptr);

  llvm::Value* res = builder.CreateLoad(alloca->getAllocatedType(), alloca, name);
  return res;
}

llvm::Value* SetLocal::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // First generate the value code.
  llvm::Value* value = value_->Codegen(fct, builder);

  // Now call the right method to set it.
  if (var_->IsString()) {
    const char* name = var_->GetString();
    builder.CreateStore(value, fct->GetVariable(name));
  } else {
    size_t idx = var_->GetIdx();
    builder.CreateStore(value, fct->GetVariable(idx));
  }

  // Return the value.
  return value;
}

llvm::Value* Const::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  switch (type_) {
    case FLOAT_32: {
      float f = value_->GetFloat();
      return llvm::ConstantFP::get(llvm::getGlobalContext(), APFloat(f));
    }
    case FLOAT_64:
      return llvm::ConstantFP::get(llvm::getGlobalContext(), APFloat(value_->GetDouble()));
    case INT_32: {
      // TODO: always unsigned?
      int val = value_->GetInteger();
      return llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(32, val, false));
    }
    case INT_64: {
      // TODO: always unsigned?
      int64_t val = value_->GetInteger();
      return llvm::ConstantInt::get(llvm::getGlobalContext(), APInt(64, val, false));
    }
    default:
      // TODO
      assert(0);
      break;
  }
  return nullptr;
}

llvm::Function* CallExpression::GetCallee(WasmFunction* fct) const {
  WasmModule* module = fct->GetModule();
  WasmFunction* wfct = nullptr;
  llvm::Function* result = nullptr;

  if (call_id_->IsString()) {
    const char* name = call_id_->GetString();
    wfct = module->GetWasmFunction(name);
  } else {
    size_t idx = call_id_->GetIdx();
    wfct  = module->GetWasmFunction(idx);
  }

  if (wfct != nullptr) {
    result = wfct->GetFunction();
  }

  // Should never be null, however we can't right now call inter-module functions.
  assert(result != nullptr);

  return result;
}

llvm::Value* CallExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  llvm::Function* callee = GetCallee(fct);
  assert(callee != nullptr);

  // Now create the arguments for the call creation.
  std::vector<Value*> args;

  if (params_ != nullptr) {
    for (auto elem : *params_) {
      args.push_back(elem->Codegen(fct, builder));
    }
  }

  return builder.CreateCall(callee, args, "calltmp");
}

llvm::Value* IfExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // Start by generating the condition.
  llvm::Value* cond_value = cond_->Codegen(fct, builder);

  llvm::Function* llvm_fct = fct->GetFunction();

  // Create the blocks for true/false/end.
  llvm::BasicBlock* true_bb = nullptr;
  llvm::BasicBlock* false_bb = nullptr;
  llvm::BasicBlock* end_bb = nullptr;

  // We should have at least a true_cond_.
  assert(true_cond_ != nullptr);

  // Add it automatically to the function.
  true_bb = BasicBlock::Create(llvm::getGlobalContext(), "true", llvm_fct);

  // The other two will wait before being emitted.
  false_bb = BasicBlock::Create(llvm::getGlobalContext(), "false");
  end_bb = BasicBlock::Create(llvm::getGlobalContext(), "end");

  builder.CreateCondBr(cond_value, true_bb, false_bb);

  // Start generating the if.
  builder.SetInsertPoint(true_bb);
  Value* true_result = true_cond_->Codegen(fct, builder);

  // If we do not finish with a terminator, generate a jump.
  if (dynamic_cast<TerminatorInst*>(true_result) == nullptr) {
    // Branch now to the end_bb.
    builder.CreateBr(end_bb);
  }

  // Come back to the true_bb.
  true_bb = builder.GetInsertBlock();

  // Handle false side.
  // First emit it.
  llvm_fct->getBasicBlockList().push_back(false_bb);
  builder.SetInsertPoint(false_bb);

  // We might not have one.
  Value* false_result = nullptr;
  if (false_cond_ != nullptr) {
    false_result = false_cond_->Codegen(fct, builder);
  }

  // Branch now to the end_bb.
  builder.CreateBr(end_bb);
  // Come back to the true_bb.
  false_bb = builder.GetInsertBlock();

  // Finally handle the merge point.
  // First emit it.
  llvm_fct->getBasicBlockList().push_back(end_bb);
  builder.SetInsertPoint(end_bb);

  // Result is the true_result except if there is an else.
  Value* result = true_result;
  if (false_result != nullptr) {
    // Now add a phi node for both sides. And that will be the result.

    // But first what type?
    llvm::Type* merge_type = true_result->getType();

    // TODO: this is not enough even... but it should bring me back here fast...
    assert(merge_type == false_result->getType());

    PHINode* merge_phi = builder.CreatePHI(merge_type, 2, "iftmp");

    merge_phi->addIncoming(true_result, true_bb);
    merge_phi->addIncoming(false_result, false_bb);

    result = merge_phi;
  }

  return result;
}

llvm::Value* LabelExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  const char* name = "end_label";

  if (var_ != nullptr) {
    // Get the string name, whether integer or not.
    name = var_->GetString();
  }

  // For now, just ignore it for code generation.
  llvm::BasicBlock* end_label = BasicBlock::Create(llvm::getGlobalContext(), name);

  fct->PushLabel(end_label);

  // Generate the code now.
  expr_->Codegen(fct, builder);

  // Now jump to the end_label.
  builder.CreateBr(end_label);

  // Now add the block and set it as insert point.
  llvm::Function* llvm_fct = fct->GetFunction();
  llvm_fct->getBasicBlockList().push_back(end_label);

  // Now set ourselves to the end of the label.
  builder.SetInsertPoint(end_label);

  // Finally pop the label.
  fct->PopLabel();
}

llvm::Value* LoopExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // For now, just ignore it for code generation.
  llvm::BasicBlock* preheader = builder.GetInsertBlock();

  // The other two will wait before being emitted.
  llvm::BasicBlock* loop = BasicBlock::Create(llvm::getGlobalContext(), "loop", fct->GetFunction());

  // Jump to the loop.
  builder.CreateBr(loop);

  // Now we are in the loop.
  builder.SetInsertPoint(loop);

  llvm::Value* value = loop_->Codegen(fct, builder);

  // Create the unconditional branch back to the start.
  builder.CreateBr(loop);

  // Create a new block, it will be dead code but it will let LLVM land on its feet.
  BasicBlock* label_dc = BasicBlock::Create(llvm::getGlobalContext(), "label_dc ", fct->GetFunction());
  builder.SetInsertPoint(label_dc);


  // All is good :).
  return value;
}

llvm::Value* BlockExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // For now, there is no reason to really care about block.
  for (std::list<Expression*>::const_iterator it = list_->begin(); it != list_->end(); it++) {
    Expression* expr = *it;
    expr->Codegen(fct, builder);
  }

  return nullptr;
}

llvm::Value* BreakExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // TODO.
  assert(expr_ == nullptr);

  llvm::BasicBlock* bb = nullptr;

  if (var_->IsString() == false) {
    size_t idx = var_->GetIdx();
    bb = fct->GetLabel(idx);
  } else {
    const char* name = var_->GetString();
    bb = fct->GetLabel(name);
  }

  assert(bb != nullptr);

  // We just want a jump to that block.
  builder.CreateBr(bb);

  // Create a new block, it will be dead code but it will let LLVM land on its feet.
  BasicBlock* break_dc  = BasicBlock::Create(llvm::getGlobalContext(), "break_dc", fct->GetFunction());
  builder.SetInsertPoint(break_dc);

  return nullptr;
}

llvm::Value* ReturnExpression::Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
  // Generate the code for the return, then call the handler.
  llvm::Value* result = result_->Codegen(fct, builder);
  assert(result != nullptr);
  fct->HandleReturn(result, builder);
}

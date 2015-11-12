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

#include "binop.h"
#include "expression.h"
#include "function.h"
#include "module.h"

using namespace llvm;

// To help with code generation, create two subclasses of Binop for special cases.
class ReallyDivRem : public Binop {
  protected:
    bool div_;
    bool sign_;

  public:
    ReallyDivRem(Expression* l, Expression* r, bool div = true, bool sign = false, ETYPE type = VOID) :
      Binop(new Operation(div ? DIV_OPER : REM_OPER, sign, type), l, r), div_(div), sign_(sign) {
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
    ReallyShift(Expression* l, Expression* r, bool right = true, bool sign = false, ETYPE type = VOID) :
      Binop(new Operation(right ? SHR_OPER : SHL_OPER, sign, type), l, r), is_right_shift_(right) {
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

ETYPE Binop::HandleType(ETYPE type, llvm::Type* lt, llvm::Type* rt) {
  // If type is not void, the type should be the same as lt.
  llvm::Type* chosen = lt;
  llvm::Type* void_type = llvm::Type::getVoidTy(llvm::getGlobalContext());

  if (chosen == void_type) {
    chosen = rt;
  }

  // Return lt's type.
  return ConvertTypeID2ETYPE(chosen);
}

llvm::Value* Binop::HandleInteger(llvm::Value* lv, llvm::Value* rv, llvm::IRBuilder<>& builder) {
  OPERATION op = operation_->GetOperation();
  bool is_signed = operation_->GetSignedOrOrdered();

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
    ValueHolder* vh = new ValueHolder(left_width);
    Const* width = new Const(type, vh);

    Operation* op = new Operation(GE_OPER, false, type);
    Binop* cond = new Binop(op, right_, width);

    vh = new ValueHolder(left_width - 1);
    Const* width_minus_one = new Const(type, vh);

    vh = new ValueHolder(0);
    Const* zero = new Const(type, vh);

    // False path here goes back to right_ here.
    ReallyShift* rsr = new ReallyShift(left_, right_, right, sign, type);

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
    ReallyShift* rsr = new ReallyShift(left_, width_minus_one, true, true, type);

    // Create the actual shift.
    ReallyShift* original_rsr = new ReallyShift(left_, right_, true, true, type);

    // False path here goes back to the real shift here.
    IfExpression* width_check = new IfExpression(cond, rsr, original_rsr);

    // Now generate the code.
    width_check->Codegen(fct, builder);
  }
}

llvm::Value* Binop::HandleDivRem(WasmFunction* fct, llvm::IRBuilder<>& builder, bool sign, bool div) {
  // We have more work to do here: we have a special case for 0x80000000 with -1.
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
      ReallyDivRem* rdr = new ReallyDivRem(left_, right_, div, sign, type);
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

  ReallyDivRem* rdr = new ReallyDivRem(left_, right_, div, sign, type);
  IfExpression* left_test = new IfExpression(cond, left, rdr);

  // Now we generate the test on the right side: is it -1?
  vh = new ValueHolder(-1);
  Const* minus_one = new Const(type, vh);

  op = new Operation(EQ_OPER, true, type);
  cond = new Binop(op, right_, minus_one);

  rdr = new ReallyDivRem(left_, right_, div, sign, type);
  IfExpression* full_test = new IfExpression(cond, left_test, rdr);

  // Now generate code.
  return full_test->Codegen(fct, builder);
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


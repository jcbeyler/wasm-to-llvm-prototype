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
#ifndef H_MEMORY
#define H_MEMORY

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Intrinsics.h"

#include "base_expression.h"
#include "enums.h"
#include "module.h"
#include "operation.h"
#include "simple.h"

class OffsetAlignInformation {
  protected:
    bool offset_defined_;
    bool align_defined_;

    uint32_t offset_;
    uint32_t align_;

  public:
    OffsetAlignInformation() : offset_defined_(false),
                               align_defined_(false),
                               offset_(0), align_(1) {
    }

    void SetOffset(uint32_t offset) {
      assert(offset_defined_ == false);
      offset_ = offset;
      offset_defined_ = true;
    }

    void SetAlign(uint32_t align) {
      // Check that we only have one align here.
      assert(align_defined_ == false);
      align_ = align;
      align_defined_ = true;
    }

    int GetAlign() const {
      assert(align_defined_ == true);
      return align_;
    }

    int GetOffset() const {
      assert(offset_defined_ == true);
      return offset_;
    }

    int IsOffsetDefined() const {
      return offset_defined_;
    }

    int IsAlignDefined() const {
      return align_defined_;
    }
};

class MemoryExpression : public Expression {
  protected:
    Expression* address_;
    size_t size_;
    bool sign_;
    ETYPE type_;

    uint32_t offset_;
    uint32_t align_;

  public:
    MemoryExpression(size_t size) : address_(nullptr), size_(size), sign_(0), type_(VOID),
                                    offset_(0), align_(size) {
    }

    MemoryExpression(Expression* address = nullptr, size_t size = 0, bool sign = false, ETYPE type = VOID) : address_(address), size_(size), sign_(sign), type_(type),
                      offset_(0), align_(size) {
    }

    void SetOffsetAlign(OffsetAlignInformation* oai) {
      if (oai->IsOffsetDefined() == true) {
        offset_ = oai->GetOffset();
      }

      if (oai->IsAlignDefined() == true) {
        align_ = oai->GetAlign();
      }
    }

    void SetType(ETYPE t) {
      type_ = t;
    }

    void SetSign(bool b) {
      sign_ = b;
    }

    void SetAddress(Expression* address) {
      address_ = address;
    }

    void SetSize(size_t size) {
      size_ = size;
    }

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(%s.memory operation", GetETypeName(type_));

      if (address_) {
        address_->Dump();
      } else {
        BISON_PRINT("Address is nullptr");
      }

      BISON_PRINT(")");
    }

    llvm::Value* GetPointer(WasmFunction* fct, llvm::IRBuilder<>& builder) const;
    llvm::Type* GetAddressType() const;

    void UpdateSize() {
      if (size_ == 0) {
        // If we don't have a size, get the size of the type_.
        size_ = GetTypeSize(type_);
      }
    }
};

class Store : public MemoryExpression {
  protected:
    Expression* value_;

    llvm::Value* ResizeIntegerIfNeed(llvm::Value* value,
                                     llvm::Type* value_type,
                                     bool sign,
                                     llvm::IRBuilder<>& builder);

  public:
    Store(Expression* add, Expression* value) :
      MemoryExpression(add), value_(value) {
    }

    Store() : MemoryExpression(), value_(nullptr) {
    }

    Store(size_t size) : MemoryExpression(size), value_(nullptr) {
    }

    void SetValue(Expression* value) {
      value_ = value;
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(%s.store", GetETypeName(type_));

      if (size_ != 0) {
        BISON_PRINT("%lu", size_);
      }

      BISON_PRINT("_%c ", sign_ ? 's' : 'u');

      if (address_) {
        address_->Dump();
      } else {
        BISON_PRINT("Address is nullptr");
      }

      BISON_PRINT(" ");

      if (value_) {
        value_->Dump();
      } else {
        BISON_PRINT("nullptr");
      }

      BISON_PRINT(")");
    }
};

class Load : public MemoryExpression {
  protected:

    llvm::Value* ResizeIntegerIfNeed(llvm::Value* value,
                                     llvm::Type* value_type,
                                     bool sign,
                                     llvm::IRBuilder<>& builder);

  public:
    Load() : MemoryExpression() {
    }

    Load(size_t size) : MemoryExpression(size) {
    }

    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder);

    virtual void Dump(int tabs) const {
      BISON_TABBED_PRINT(tabs, "(%s.load", GetETypeName(type_));

      if (size_ != 0) {
        BISON_PRINT("%lu", size_);
      }

      BISON_PRINT("_%c ", sign_ ? 's' : 'u');

      if (address_) {
        address_->Dump();
      } else {
        BISON_PRINT("Address is nullptr");
      }

      BISON_PRINT(")");
    }
};

class MemorySize : public Expression {
  public:
    virtual llvm::Value* Codegen(WasmFunction* fct, llvm::IRBuilder<>& builder) {
      WasmModule* module = fct->GetModule();
      llvm::GlobalVariable* mem_size = module->GetMemorySize();
      return builder.CreateLoad(mem_size, "mem_size");
    }
};


#endif

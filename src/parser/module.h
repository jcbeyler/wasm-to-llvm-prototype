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
#ifndef H_MODULE
#define H_MODULE

#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/Scalar.h"

#include "export.h"

#include <list>
#include <map>
#include <vector>

// Forward declaration.
class WasmFunction;
class WasmFile;
class WasmImportFunction;

#include "function.h"
#include "import_function.h"

/**
 * Base module node for Wasm, contains right now exports and functions
 */

class WasmModule {
  protected:
    llvm::Module* module_;
    llvm::legacy::PassManager* fpm_;
    WasmFile* file_;

    // Created during building.
    std::list<WasmFunction*> functions_;
    std::list<WasmExport*> exports_;
    std::list<WasmImportFunction*> import_functions_;

    // For reference later.
    std::map<std::string, WasmFunction*> map_functions_;
    std::vector<WasmFunction*> vector_functions_;

    // Hash associations.
    std::map<std::string, std::string> map_hash_association_;
    std::map<std::string, std::string> map_reversed_hash_association_;

    // For reference later for import functions: no hash yet for them.
    std::map<std::string, WasmImportFunction*> map_import_functions_;
    std::vector<WasmImportFunction*> vector_import_functions_;

    std::string name_;
    std::string hash_name_;

    // Difference here of signess to have -1 as a possible value for non-defined memory size.
    int64_t memory_;
    uint64_t max_memory_;
    std::list<Segment*>* segments_;

    llvm::GlobalVariable* memory_pointer_;
    llvm::GlobalVariable* memory_size_;
    llvm::Function* memory_allocator_fct_;

    int line_;

    WasmFunction* InternalGetWasmFunction(const char* name, bool check_file, unsigned int line) const;
    void GenerateMemoryGlobals();
    void GenerateMemoryBaseFunction();
    void HandleSegments(llvm::IRBuilder<>& builder, llvm::Instruction* malloc_result);
    void GenerateMemoryBasedCode();

  public:
    WasmModule(llvm::Module* module = nullptr, llvm::legacy::PassManager* fpm = nullptr, WasmFile* file = nullptr) :
      module_(module), fpm_(fpm), file_(file),
      memory_(-1), max_memory_(~0), segments_(nullptr),
      memory_pointer_(nullptr), memory_size_(nullptr),
      memory_allocator_fct_(nullptr),
      line_(0) {
        static int cnt = 0;
        std::ostringstream oss;
        oss << "wasm_module_" << cnt;
        cnt++;
        name_ = oss.str();

        std::ostringstream hash_oss;;
        hash_oss << "wm_" << cnt << "_";
        hash_name_ = hash_oss.str();
    }

    void SetLine(int line) {
      line_ = line;
    }

    int GetLine() const {
      return line_;
    }

    void AddFunction(WasmFunction* wf) {
      functions_.push_front(wf);
    }

    llvm::Function* GetMemoryAllocator() {
      return memory_allocator_fct_;
    }

    void AddImportFunction(WasmImportFunction* wif) {
      import_functions_.push_front(wif);
    }

    void AddExport(WasmExport* e) {
      exports_.push_front(e);
    }

    llvm::Module* GetModule() const {
      return module_;
    }

    void SetWasmFile(WasmFile* f) {
      file_ = f;
    }

    void AddFunctionAndRegister(WasmFunction* wf) {
      AddFunction(wf);
      map_functions_[wf->GetName()] = wf;
    }

    void Print() {
      std::error_code ec;
      llvm::sys::fs::OpenFlags of = llvm::sys::fs::OpenFlags::F_Text;
      std::ostringstream oss;
      oss << "obj/" << name_ << ".ll";
      raw_fd_ostream file(oss.str().c_str(), ec, of);
      module_->print(file, NULL);
      file.close();
    }

    void AddMemory(size_t value, std::list<Segment*>* segments) {
      assert(memory_ == -1);
      memory_ = value;
      segments_ = segments;
    }

    void AddMemory(size_t value, int max, std::list<Segment*>* segments) {
      assert(memory_ == -1);
      memory_ = value;
      max_memory_ = max;
      segments_ = segments;
    }

    int GetMemory() const {
      return memory_;
    }

    llvm::GlobalVariable* GetBaseMemory() const {
      return memory_pointer_;
    }

    llvm::GlobalVariable* GetMemorySize() const {
      return memory_size_;
    }

    const std::string& GetHashName() const {
      return hash_name_;
    }

    std::list<WasmFunction*>& GetWasmFunctions() {
      return functions_;
    }

    std::string GetMemoryBaseFunctionName() const;
    std::string GetMemoryBaseName() const;
    std::string GetMemorySizeName() const;

    void Generate();
    void Dump();
    void Initialize();

    WasmFunction* GetWasmFunction(const char* name, bool check_file = true, unsigned int line = ~0) const;
    WasmFunction* GetWasmFunction(const std::string& name, bool check_file = true, unsigned int line = ~0) const;
    WasmFunction* GetWasmFunction(size_t idx) const;

    WasmImportFunction* GetWasmImportFunction(const char* name, bool check_file = true, unsigned int line = ~0) const;
    WasmImportFunction* GetWasmImportFunction(const std::string& name, bool check_file = true, unsigned int line = ~0) const;
    WasmImportFunction* GetWasmImportFunction(size_t idx) const;

    llvm::Function* GetOrCreateIntrinsic(llvm::Intrinsic::ID name, ETYPE type = VOID);
    llvm::Function* GetWasmAssertTrapFunction();

    void MangleNames(WasmFile* file);
    void RegisterMangle(const std::string& name, const std::string& mangled);
    bool DoesMangledNameExist(const std::string& name) {
      return map_reversed_hash_association_.find(name) != map_reversed_hash_association_.end();
    }
};

#endif

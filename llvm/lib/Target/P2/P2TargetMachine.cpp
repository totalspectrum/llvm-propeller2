//===-- P2TargetMachine.cpp - Define TargetMachine for P2 -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about P2 target spec.
//
//===----------------------------------------------------------------------===//

#include "P2TargetMachine.h"
#include "P2.h"
#include "P2TargetObjectFile.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

#define DEBUG_TYPE "p2"

extern "C" void LLVMInitializeP2Target() {
  // Register the target.
    RegisterTargetMachine<P2TargetMachine> X(TheP2Target);
}

P2TargetMachine::P2TargetMachine(const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     Optional<Reloc::Model> RM,
                                     Optional<CodeModel::Model> CM, CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, "e-p:32:32-i32:32", TT, CPU, FS, Options, Reloc::Static, CodeModel::Small, OL),
                        TLOF(std::make_unique<P2TargetObjectFile>()),
                        Subtarget(TT, std::string(CPU), std::string(FS), *this) {

    initAsmInfo();
}

P2TargetMachine::~P2TargetMachine() {}

namespace {
//@P2PassConfig {
/// P2 Code Generator Pass Configuration Options.
    class P2PassConfig : public TargetPassConfig {
    public:
        P2PassConfig(P2TargetMachine &TM, PassManagerBase &PM) : TargetPassConfig(TM, PM) {}

        P2TargetMachine &getP2TargetMachine() const {
            return getTM<P2TargetMachine>();
        }
    };

} // namespace

TargetPassConfig *P2TargetMachine::createPassConfig(PassManagerBase &PM) {
    return new P2PassConfig(*this, PM);
}
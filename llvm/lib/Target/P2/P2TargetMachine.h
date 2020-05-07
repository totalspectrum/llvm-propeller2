//===-- P2TargetMachine.h - Define TargetMachine for AVR -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the AVR specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_P2_P2TARGETMACHINE_H
#define LLVM_LIB_TARGET_P2_P2TARGETMACHINE_H

#include "P2Subtarget.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
    class formatted_raw_ostream;
    class P2RegisterInfo;

    class P2TargetMachine : public LLVMTargetMachine {
        std::unique_ptr<TargetLoweringObjectFile> TLOF;
        P2Subtarget Subtarget; // must have at least 1 subtarget

        // Selected ABI
        // P2ABIInfo ABI;

    public:
        P2TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                        StringRef FS, const TargetOptions &Options,
                        Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                        CodeGenOpt::Level OL, bool JIT);
        ~P2TargetMachine() override;

        const P2Subtarget *getSubtargetImpl() const { return &Subtarget; }
        const P2Subtarget *getSubtargetImpl(const Function &F) const override {
            return &Subtarget;
        }

        // Pass Pipeline Configuration
        TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

        TargetLoweringObjectFile *getObjFileLowering() const override {
            return TLOF.get();
        }
    };
} // End llvm namespace

#endif

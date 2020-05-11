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

#include "P2FrameLowering.h"
#include "P2ISelLowering.h"
#include "P2InstrInfo.h"
#include "P2Subtarget.h"

#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/CodeGen/SelectionDAGTargetInfo.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
    class formatted_raw_ostream;
    class P2RegisterInfo;
    class P2Subtarget;

    class P2TargetMachine : public LLVMTargetMachine {

        std::unique_ptr<TargetLoweringObjectFile> TLOF;
        P2Subtarget subtarget;

    public:
        P2TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                        StringRef FS, const TargetOptions &Options,
                        Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                        CodeGenOpt::Level OL, bool JIT);
        ~P2TargetMachine() override;

        const P2Subtarget *getSubtargetImpl() const;
        const P2Subtarget *getSubtargetImpl(const Function &) const override;

        const TargetFrameLowering *getFrameLowering() const {
            return subtarget.getFrameLowering();
        }
        const P2InstrInfo *getInstrInfo() const {
            return subtarget.getInstrInfo();
        }
        const TargetRegisterInfo *getRegisterInfo() const {
            return subtarget.getRegisterInfo();
        }
        const P2TargetLowering *getTargetLowering() const {
            return subtarget.getTargetLowering();
        }
        const SelectionDAGTargetInfo *getSelectionDAGInfo() const {
            return subtarget.getSelectionDAGInfo();
        }

        // Pass Pipeline Configuration
        TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

        TargetLoweringObjectFile *getObjFileLowering() const override {
            return TLOF.get();
        }
    };
} // End llvm namespace

#endif

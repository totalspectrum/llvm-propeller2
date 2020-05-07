//===-- P2FrameLowering.h - Define frame lowering for P2 ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LIB_TARGET_P2_P2FRAMELOWERING_H
#define LLVM_LIB_TARGET_P2_P2FRAMELOWERING_H

#include "P2.h"
#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class P2Subtarget;

class P2FrameLowering : public TargetFrameLowering {

public:
    explicit P2FrameLowering()
        : TargetFrameLowering(StackGrowsDown, Align(4), -4) {
    }

    bool hasFP(const MachineFunction &MF) const override;

    /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
    /// the function.
    void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
    void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

};

} // End llvm namespace

#endif
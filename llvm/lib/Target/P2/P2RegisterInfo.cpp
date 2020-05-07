//===-- P2RegisterInfo.cpp - P2 Register Information --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the P2 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "P2RegisterInfo.h"
#include "P2.h"
#include "P2MachineFunctionInfo.h"
#include "P2TargetMachine.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

#define DEBUG_TYPE "P2-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "P2GenRegisterInfo.inc"

// FIXME: Provide proper call frame setup / destroy opcodes.
P2RegisterInfo::P2RegisterInfo() : P2GenRegisterInfo(0) {}

const MCPhysReg* P2RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
    // TODO
    static const MCPhysReg CalleeSavedRegs[1] = {};

    return CalleeSavedRegs;
}

BitVector P2RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
    // TODO
    BitVector Reserved(getNumRegs());

    return Reserved;
}

void
P2RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                        int SPAdj, unsigned FIOperandNum,
                                        RegScavenger *RS) const {
    // TODO
}

Register P2RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
    // TODO
    return P2::R15;
}

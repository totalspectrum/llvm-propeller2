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

#include "P2.h"
#include "P2MachineFunctionInfo.h"
#include "P2RegisterInfo.h"
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

    // get a bunch of target info classes
    MachineInstr &MI = *II;
    DebugLoc dl = MI.getDebugLoc();
    MachineFunction &MF = *MI.getParent()->getParent();
    MachineFrameInfo &MFI = MF.getFrameInfo();
    P2FunctionInfo *P2FI = MF.getInfo<P2FunctionInfo>();
    const P2TargetMachine &TM = (const P2TargetMachine &)MF.getTarget();
    const TargetInstrInfo &inst_info = *TM.getInstrInfo();
    const TargetFrameLowering *TFI = TM.getFrameLowering();

    LLVM_DEBUG(errs() << "\nFunction : " << MF.getFunction().getName() << "\n";
        errs() << "<--------->\n" << MI);

    int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
    uint64_t stackSize = MFI.getStackSize();
    int64_t offset = MFI.getObjectOffset(FrameIndex);

    offset += MFI.getStackSize() - TFI->getOffsetOfLocalArea(); // LOA should be 0 for P2
    offset -= MI.getOperand(FIOperandNum+1).getImm(); // offset down. so each index is from the top of the current frame moving down the RAM

    MI.setDesc(inst_info.get(P2::MOVrr)); // change our psesudo instruction to a mov
    MI.getOperand(FIOperandNum).ChangeToRegister(P2::SP, false); // change the abstract frame index register to our real frame pointer register
    MI.RemoveOperand(2); // remove the 3rd operand this instruction

    assert(offset > 0 && "Invalid offset"); // offset should be positive

    Register dst_reg = MI.getOperand(0).getReg();
    II++; // skip forward by 1 instruction

    MachineInstr *mov_inst = BuildMI(*MI.getParent(), II, dl, inst_info.get(P2::ADDri), dst_reg)
                            .addReg(dst_reg, RegState::Kill)
                            .addImm(offset);
    //mov_inst->getOperand(3).setIsDead();

    LLVM_DEBUG(errs() << "FrameIndex : " << FrameIndex << "\n"
                        << "stackSize  : " << stackSize << "\n");
    LLVM_DEBUG(errs() << MI);
    LLVM_DEBUG(errs() << "Offset     : " << offset << "\n" << "<--------->\n");

}

Register P2RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
    return P2::SP;
}

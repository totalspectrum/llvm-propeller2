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
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

#define DEBUG_TYPE "p2-reg-info"

#define GET_REGINFO_TARGET_DESC
#include "P2GenRegisterInfo.inc"

P2RegisterInfo::P2RegisterInfo() : P2GenRegisterInfo(0) {}

const MCPhysReg* P2RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
    return CSR_SaveList;
}

const uint32_t *P2RegisterInfo::getCallPreservedMask(const MachineFunction &MF,CallingConv::ID CC) const {
    LLVM_DEBUG(errs() << CSR_RegMask[0] << "\n");
    return CSR_RegMask;
}

BitVector P2RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
    BitVector Reserved(getNumRegs());
    Reserved.set(P2::PTRA);
    Reserved.set(P2::PTRB);
    Reserved.set(P2::OUTA);
    Reserved.set(P2::OUTB);
    Reserved.set(P2::DIRA);
    Reserved.set(P2::DIRB);
    Reserved.set(P2::QX);
    Reserved.set(P2::QY);
    Reserved.set(P2::R15); // reserve R15 since it's the one used for returns
    return Reserved;
}

void P2RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj, unsigned FIOperandNum, RegScavenger *RS) const {

    // get a bunch of target info classes
    MachineInstr &MI = *II;
    DebugLoc dl = MI.getDebugLoc();
    MachineFunction &MF = *MI.getParent()->getParent();
    MachineFrameInfo &MFI = MF.getFrameInfo();
    //P2FunctionInfo *P2FI = MF.getInfo<P2FunctionInfo>();
    const P2TargetMachine &TM = (const P2TargetMachine &)MF.getTarget();
    const TargetInstrInfo &inst_info = *TM.getInstrInfo();
    const TargetFrameLowering *TFI = TM.getFrameLowering();

    LLVM_DEBUG(
        errs() << "\nFunction : " << MF.getFunction().getName() << "\n";
        errs() << "<--------->\n" << MI);

    int frame_idx = MI.getOperand(FIOperandNum).getIndex();
    uint64_t stack_size = MFI.getStackSize();
    int64_t fi_offset = MFI.getObjectOffset(frame_idx); // offset from the start of the frame (low address)
    int local_frame_size = MF.getFrameInfo().getLocalFrameSize();
    int64_t offset = stack_size-fi_offset;

    LLVM_DEBUG(errs() << "frame_idx : " << frame_idx << "\n"
                        << "stack size  : " << stack_size << "\n"
                        << "fi offset : " << fi_offset << "\n");
    LLVM_DEBUG(dbgs() << "LocalFrameSize : " << local_frame_size << "\n");

    LLVM_DEBUG(errs() << "Offset     : " << offset << "\n" << "<--------->\n");

    assert(offset >= 0 && "Invalid offset"); // offset should be positive or 0

    if (MI.getOpcode() == P2::FRMIDX) {
        MI.setDesc(inst_info.get(P2::MOVrr)); // change our psesudo instruction to a mov
        MI.getOperand(FIOperandNum).ChangeToRegister(P2::PTRA, false); // change the abstract frame index register to our real stack pointer register
        //MI.getOperand(0).ChangeToRegister(P2::PTRB, false); // I think we can always just use PTRB instead of the assigned reg

        Register dst_reg = MI.getOperand(0).getReg();
        II++; // skip forward by 1 instruction

        BuildMI(*MI.getParent(), II, dl, inst_info.get(P2::SUBri), dst_reg)
                                .addReg(dst_reg, RegState::Kill)
                                .addImm(offset);
    } else {
        // if we decide we need to scavange registers, we need to create an emergency stack slock in frame lowering,
        // then make sure to kill the register after it is used here. For now, we can just use PTRB as a second stack pointer
        // register for writing to this frame index

        // Register reg = RS->FindUnusedReg(&P2::P2GPRRegClass);
        // if (!reg) {
        //     LLVM_DEBUG(errs() << "No unused registers available, scavenging one\n");
        //     reg = RS->scavengeRegister(&P2::P2GPRRegClass, SPAdj);
        //     RS->setRegUsed(reg);
        // }

        // assert(reg && "Need to have a register for frame index elimination");

        // LLVM_DEBUG(errs() << "got reg to use " << reg << "\n");

        Register reg = P2::PTRB;

        BuildMI(*MI.getParent(), II, dl, inst_info.get(P2::MOVrr), reg)
                                .addReg(P2::PTRA); // save the SP to an unused register

        BuildMI(*MI.getParent(), II, dl, inst_info.get(P2::SUBri), reg)
                                .addReg(reg, RegState::Kill)
                                .addImm(offset); // adjust saved SP by frame index offset

        LLVM_DEBUG((*II).dump(); errs() << "... became ";);

        (*II).getOperand(FIOperandNum).ChangeToRegister(reg, false);

        LLVM_DEBUG((*II).dump());
    }
}

Register P2RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
    return P2::PTRA;
}

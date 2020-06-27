//===-- P2InstrInfo.cpp - P2 Instruction Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the P2 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "P2InstrInfo.h"

#include "P2TargetMachine.h"
#include "P2MachineFunctionInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "p2-inst-info"

#define GET_INSTRINFO_CTOR_DTOR
#include "P2GenInstrInfo.inc"

// Pin the vtable to this file.
void P2InstrInfo::anchor() {}

P2InstrInfo::P2InstrInfo() : P2GenInstrInfo(P2::ADJCALLSTACKUP, P2::ADJCALLSTACKDOWN), RI() {}

void P2InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        Register DestReg, int FrameIndex,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const {
    DebugLoc DL;
    if (MI != MBB.end()) {
        DL = MI->getDebugLoc();
    }

    MachineFunction &MF = *MBB.getParent();
    const MachineFrameInfo &MFI = MF.getFrameInfo();

    MachineMemOperand *MMO = MF.getMachineMemOperand(
        MachinePointerInfo::getFixedStack(MF, FrameIndex),
        MachineMemOperand::MOLoad, MFI.getObjectSize(FrameIndex),
        MFI.getObjectAlign(FrameIndex));

    unsigned Opcode = 0;
    if (TRI->isTypeLegalForClass(*RC, MVT::i32)) {
        Opcode = P2::RDLONGrr;
    } else {
        llvm_unreachable("Cannot load this register from a stack slot!");
    }

    BuildMI(MBB, MI, DL, get(Opcode), DestReg)
        .addFrameIndex(FrameIndex)
        .addMemOperand(MMO);

    LLVM_DEBUG(errs() << ">> load reg " << DestReg << " from stack " << FrameIndex << "\n");
}

void P2InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator MI,
                               const DebugLoc &DL, MCRegister DestReg,
                               MCRegister SrcReg, bool KillSrc) const {

    if (SrcReg == P2::QX) {
        BuildMI(MBB, MI, DL, get(P2::GETQX), DestReg);
    } else if (SrcReg == P2::QY) {
        BuildMI(MBB, MI, DL, get(P2::GETQY), DestReg);
    } else {
        BuildMI(MBB, MI, DL, get(P2::MOVrr), DestReg).addReg(SrcReg, getKillRegState(KillSrc));
    }
}

void P2InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        Register SrcReg, bool isKill,
                                        int FrameIndex,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const {
    MachineFunction &MF = *MBB.getParent();

    DebugLoc DL;
    if (MI != MBB.end()) {
        DL = MI->getDebugLoc();
    }

    const MachineFrameInfo &MFI = MF.getFrameInfo();

    MachineMemOperand *MMO = MF.getMachineMemOperand(
        MachinePointerInfo::getFixedStack(MF, FrameIndex),
        MachineMemOperand::MOStore, MFI.getObjectSize(FrameIndex),
        MFI.getObjectAlign(FrameIndex));

    unsigned Opcode = 0;
    if (TRI->isTypeLegalForClass(*RC, MVT::i32)) {
        Opcode = P2::WRLONGrr;
    } else {
        llvm_unreachable("Cannot store this register into a stack slot!");
    }

    BuildMI(MBB, MI, DL, get(Opcode))
        .addReg(SrcReg, getKillRegState(isKill))
        .addFrameIndex(FrameIndex)
        .addMemOperand(MMO);

    LLVM_DEBUG(errs() << ">> store reg " << SrcReg << " to stack " << FrameIndex << "\n");
}

void P2InstrInfo::adjustStackPtr(unsigned SP, int64_t amount, MachineBasicBlock &MBB, MachineBasicBlock::iterator I) const {
    DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();

    unsigned inst = P2::ADDri;

    LLVM_DEBUG(errs() << "adjust stack pointer by " << amount << "\n");

    if (amount < 0) {
        inst = P2::SUBri;
        amount = -amount;
    }

    if (isInt<32>(amount)) {
        if (!isInt<9>(amount)) {
            // if we need more than 9 bits to store amount, augment the next source immediate (which will be added below)
            BuildMI(MBB, I, DL, get(P2::AUGS)).addImm(amount>>9);
        }

        BuildMI(MBB, I, DL, get(inst), SP).addReg(SP).addImm(amount&0x1ff);
    } else {
        llvm_unreachable("Cannot adjust stack pointer by more than 32 bits (and adjusting by more than 20 bits never makes sense!)");
    }
}
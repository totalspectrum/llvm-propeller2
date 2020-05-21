//===-- P2FrameLowering.cpp - P2 Frame Information --------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the P2 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "P2FrameLowering.h"

#include "P2InstrInfo.h"
#include "P2TargetMachine.h"
#include "P2MachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"

/*
How the call stack will work:

The stack will grow up. regsiter sp always points to the TOP of the stack, which is the start of free stack space.
Selection will generate FRMIDX pseudo instructions that will be lowered in register info by be subtracting from the
current stack pointer (sp) by the frame index offset. The callee will not save any info. The data in the stack frame
will be organized as follows:

        free stack space
SP      ------------------
SP-4    local variables
...
        ------------------
        function arguments
        ------------------
        function return
        values
SP -    ------------------
size
        caller's stack frame

*/

#define DEBUG_TYPE "p2-frame-lower"

using namespace llvm;

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas,
// if it needs dynamic stack realignment, if frame pointer elimination is
// disabled, or if the frame address is taken.
bool P2FrameLowering::hasFP(const MachineFunction &MF) const {
    const MachineFrameInfo *MFI = &MF.getFrameInfo();
    const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();

    return MF.getTarget().Options.DisableFramePointerElim(MF) ||
            MFI->hasVarSizedObjects() || MFI->isFrameAddressTaken() ||
            TRI->needsStackRealignment(MF);
}

void P2FrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
}

void P2FrameLowering::emitEpilogue(MachineFunction &MF,
                                 MachineBasicBlock &MBB) const {
}

MachineBasicBlock::iterator P2FrameLowering::eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const {

    if (!hasReservedCallFrame(MF)) {
        int64_t Amount = I->getOperand(0).getImm();

        if (I->getOpcode() == P2::ADJCALLSTACKDOWN)
            Amount = -Amount;

        tm.getInstrInfo()->adjustStackPtr(P2::SP, Amount, MBB, I);
    }

    LLVM_DEBUG(errs() << "eliminate call frame pseudo\n");

    return MBB.erase(I);
}
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

        free stack space (callee stack space)
SP      ------------------
SP-4    local variables
...
        ------------------
        function arguments (vargs or > 4 registers)
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

    // LLVM_DEBUG(errs() << "hasFP = disable FP elim: " << MF.getTarget().Options.DisableFramePointerElim(MF) <<
    //             "; var sized objects: " << MFI->hasVarSizedObjects() <<
    //             "; frame address is taken: " << MFI->isFrameAddressTaken() <<
    //             "; needs stack realignment: " << TRI->needsStackRealignment(MF) << "\n");

    return MF.getTarget().Options.DisableFramePointerElim(MF) ||
            MFI->hasVarSizedObjects() || MFI->isFrameAddressTaken() ||
            TRI->needsStackRealignment(MF);
}

void P2FrameLowering::emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const {
    LLVM_DEBUG(dbgs() << "Emit Prologue: " << MF.getName() << "\n");

    MachineModuleInfo &MMI = MF.getMMI();
    const P2InstrInfo *TII = MF.getSubtarget<P2Subtarget>().getInstrInfo();
     MachineBasicBlock::iterator MBBI = MBB.begin();

    // Debug location must be unknown since the first debug location is used
    // to determine the end of the prologue.
    DebugLoc dl;
    const MachineFrameInfo &MFI = MF.getFrameInfo();
    const std::vector<CalleeSavedInfo> &CSI = MFI.getCalleeSavedInfo();

    if (MF.getFunction().isVarArg()) {
        // Add in the varargs area here first.
        llvm_unreachable("can't yet emit prologues for vararg functions");
    }

    if (!hasFP(MF)) {
        // no frame pointer, we can return early, right?
        LLVM_DEBUG(errs()<<"no frame pointer in function");
        return;
    }

    uint64_t StackSize = MFI.getStackSize();

    // No need to allocate space on the stack.
    if (StackSize == 0 && !MFI.adjustsStack()) {
        LLVM_DEBUG(errs() << "No need to allocate stack space");
        return;
    }

    TII->adjustStackPtr(P2::SP, StackSize, MBB, MBBI);

    if (CSI.size()) {
        LLVM_DEBUG(errs() << "have callee saved info\n");
        llvm_unreachable("can't yet handle callee saved info");
        // // Find the instruction past the last instruction that saves a callee-saved
        // // register to the stack.
        // for (unsigned i = 0; i < CSI.size(); ++i)
        //     ++MBBI;

        // // Iterate over list of callee-saved registers and emit .cfi_offset
        // // directives.
        // for (std::vector<CalleeSavedInfo>::const_iterator I = CSI.begin(), E = CSI.end(); I != E; ++I) {
        //     int64_t Offset = MFI->getObjectOffset(I->getFrameIdx());
        //     unsigned Reg = I->getReg();
        //     {
        //         // Reg is in CPURegs.
        //         unsigned CFIIndex = MMI.addFrameInst(MCCFIInstruction::createOffset(
        //             nullptr, MRI->getDwarfRegNum(Reg, 1), Offset));
        //         BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION)).addCFIIndex(CFIIndex);
        //     }
        // }
    }
}

void P2FrameLowering::emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {
    LLVM_DEBUG(dbgs() << "Emit Epilogue: " << MF.getName() << "\n");
    MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
    MachineFrameInfo *MFI = &MF.getFrameInfo();

    const P2InstrInfo *TII = MF.getSubtarget<P2Subtarget>().getInstrInfo();

    // if framepointer enabled, restore the stack pointer.
    if (hasFP(MF)) {
        // Find the first instruction that restores a callee-saved register.
        // MachineBasicBlock::iterator I = MBBI;

        // for (unsigned i = 0; i < MFI->getCalleeSavedInfo().size(); ++i) {
        //     --I;
        // }

        // // Insert instruction "move $sp, $fp" at this location.
        // BuildMI(MBB, I, dl, TII.get(ADDu), SP).addReg(FP).addReg(ZERO);
    }

    // Get the number of bytes from FrameInfo
    uint64_t StackSize = MFI->getStackSize();

    if (StackSize == 0) {
        LLVM_DEBUG(errs() << "No need to de-allocate stack space");
        return;
    }

    // Adjust stack.
    TII->adjustStackPtr(P2::SP, -StackSize, MBB, MBBI);
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
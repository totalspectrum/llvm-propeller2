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
#include "P2MachineFunctionInfo.h"
#include "P2Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas,
// if it needs dynamic stack realignment, if frame pointer elimination is
// disabled, or if the frame address is taken.
bool P2FrameLowering::hasFP(const MachineFunction &MF) const {
    // const MachineFrameInfo *MFI = MF.getFrameInfo();
    // const TargetRegisterInfo *TRI = STI.getRegisterInfo();

    // return MF.getTarget().Options.DisableFramePointerElim(MF) ||
    //     MFI->hasVarSizedObjects() || MFI->isFrameAddressTaken() ||
    //     TRI->needsStackRealignment(MF);
    return false;
}

//@emitPrologue {
void P2FrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
}
//}

//@emitEpilogue {
void P2FrameLowering::emitEpilogue(MachineFunction &MF,
                                 MachineBasicBlock &MBB) const {
}
//}
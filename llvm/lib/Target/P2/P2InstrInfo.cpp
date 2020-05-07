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

#define GET_INSTRINFO_CTOR_DTOR
#include "P2GenInstrInfo.inc"

// Pin the vtable to this file.
void P2InstrInfo::anchor() {}

//@P2InstrInfo {
P2InstrInfo::P2InstrInfo(const P2Subtarget &STI) : P2GenInstrInfo(), RI(), Subtarget(STI) {}

//@GetInstSizeInBytes {
/// Return the number of bytes of code the specified instruction may be.
unsigned P2InstrInfo::GetInstSizeInBytes(const MachineInstr &MI) const {
//@GetInstSizeInBytes - body
    switch (MI.getOpcode()) {
    default:
        return MI.getDesc().getSize();
    }
}
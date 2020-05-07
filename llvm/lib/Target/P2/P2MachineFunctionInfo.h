//===-- P2MachineFunctionInfo.h - Private data used for P2 ----*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the P2 specific subclass of MachineFunctionInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_P2_P2MACHINEFUNCTION_H
#define LLVM_LIB_TARGET_P2_P2MACHINEFUNCTION_H

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include <map>

namespace llvm {

class P2FunctionInfo : public MachineFunctionInfo {
public:
    //P2FunctionInfo(MachineFunction& MF) : MF(MF), VarArgsFrameIndex(0), MaxCallFrameSize(0) {}
    P2FunctionInfo(MachineFunction &MF) : VarArgsFrameIndex(0) {}

    ~P2FunctionInfo();

    int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
    void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

private:
    virtual void anchor();

    int VarArgsFrameIndex;
};

} // end of namespace llvm

#endif // P2_MACHINE_FUNCTION_INFO_H
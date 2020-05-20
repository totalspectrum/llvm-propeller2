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
    virtual void anchor();

    MachineFunction& MF;

    /// SRetReturnReg - Some subtargets require that sret lowering includes
    /// returning the value of the returned struct in a register. This field
    /// holds the virtual register into which the sret argument is passed.
    unsigned SRetReturnReg;

    /// VarArgsFrameIndex - FrameIndex for start of varargs area.
    int VarArgsFrameIndex;

    /// True if function has a byval argument.
    bool HasByvalArg;

    /// Size of incoming argument area.
    unsigned IncomingArgSize;

    mutable int DynAllocFI; // Frame index of dynamically allocated stack area.
    bool EmitNOAT;
    unsigned MaxCallFrameSize;

public:
    //P2FunctionInfo(MachineFunction& MF) : MF(MF), VarArgsFrameIndex(0), MaxCallFrameSize(0) {}
    P2FunctionInfo(MachineFunction &MF)
        : MF(MF),
        SRetReturnReg(0),
        VarArgsFrameIndex(0),
        EmitNOAT(false),
        MaxCallFrameSize(0)
        {}

    ~P2FunctionInfo();

    int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
    void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

    unsigned getSRetReturnReg() const { return SRetReturnReg; }
    void setSRetReturnReg(unsigned Reg) { SRetReturnReg = Reg; }

    bool hasByvalArg() const { return HasByvalArg; }
    void setFormalArgInfo(unsigned Size, bool HasByval) {
        IncomingArgSize = Size;
        HasByvalArg = HasByval;
    }

    unsigned getMaxCallFrameSize() const { return MaxCallFrameSize; }
    void setMaxCallFrameSize(unsigned S) { MaxCallFrameSize = S; }

    bool getEmitNOAT() const { return EmitNOAT; }
    void setEmitNOAT() { EmitNOAT = true; }

    unsigned getIncomingArgSize() const { return IncomingArgSize; }
};

} // end of namespace llvm

#endif // P2_MACHINE_FUNCTION_INFO_H
//===-- P2ISelLowering.cpp - P2 DAG Lowering Implementation -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that P2 uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//
#include "P2ISelLowering.h"

#include "P2MachineFunctionInfo.h"
#include "P2TargetMachine.h"
#include "P2TargetObjectFile.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "P2-lower"

const char *P2TargetLowering::getTargetNodeName(unsigned Opcode) const {

    switch (Opcode) {
        case P2ISD::RET: return "P2RET";
        default:
            return nullptr;
    }
    #undef NODE
}

//@P2TargetLowering {
P2TargetLowering::P2TargetLowering(const P2TargetMachine &TM) : TargetLowering(TM) {
    addRegisterClass(MVT::i32, &P2::P2GPRRegClass);

    //  computeRegisterProperties - Once all of the register classes are
    //  added, this allows us to compute derived properties we expose.
    computeRegisterProperties(TM.getRegisterInfo());
}

#include "P2GenCallingConv.inc"

SDValue P2TargetLowering::LowerFormalArguments(SDValue Chain,
                                            CallingConv::ID CallConv,
                                            bool IsVarArg,
                                            const SmallVectorImpl<ISD::InputArg> &Ins,
                                            const SDLoc &DL, SelectionDAG &DAG,
                                            SmallVectorImpl<SDValue> &InVals) const {

    return Chain;
}

SDValue P2TargetLowering::LowerReturn(SDValue Chain,
                                        CallingConv::ID CallConv, bool isVarArg,
                                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                                        const SmallVectorImpl<SDValue> &OutVals,
                                        const SDLoc &dl, SelectionDAG &DAG) const {
    // CCValAssign - represent the assignment of the return value to locations.
    SmallVector<CCValAssign, 16> RVLocs;

    // CCState - Info about the registers and stack slot.
    CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs, *DAG.getContext());

    // Analyze return values.
    auto CCFunction = RetCC_P2;
    CCInfo.AnalyzeReturn(Outs, CCFunction);

    // If this is the first return lowered for this function, add the regs to
    // the liveout set for the function.
    MachineFunction &MF = DAG.getMachineFunction();
    unsigned e = RVLocs.size();

    SDValue Flag;
    SmallVector<SDValue, 4> RetOps(1, Chain);
    // Copy the result values into the output registers.
    for (unsigned i = 0; i != e; ++i) {
        CCValAssign &VA = RVLocs[i];
        assert(VA.isRegLoc() && "Can only return in registers!");

        Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);

        // Guarantee that all emitted copies are stuck together with flags.
        Flag = Chain.getValue(1);
        RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
    }

    // Don't emit the ret/reti instruction when the naked attribute is present in
    // the function being compiled.
    if (MF.getFunction().getAttributes().hasAttribute(AttributeList::FunctionIndex, Attribute::Naked)) {
        return Chain;
    }

    unsigned RetOpc = P2ISD::RET;

    RetOps[0] = Chain; // Update chain.

    if (Flag.getNode()) {
        RetOps.push_back(Flag);
    }

    return DAG.getNode(RetOpc, dl, MVT::Other, RetOps);

}
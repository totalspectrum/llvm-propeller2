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

/*
it feels like there's a lot of redundant code here, so we should dig through what ever piece does and clean things up

How function calling works:

1. LowerCall is used to copy arguments to there destination registers/stack space
2. LowerFormalArguments is used to copy passed argument by the callee into the callee's stack frame
3. LowerReturn is used to copy the return value to r15 or the stack
4. LowerCallResult used to copy the return value out of r15 or the stack into the caller's space

*/

using namespace llvm;

#define DEBUG_TYPE "p2-isel-lower"

// addLiveIn - This helper function adds the specified physical register to the
// MachineFunction as a live in value.  It also creates a corresponding
// virtual register for it.
static unsigned addLiveIn(MachineFunction &MF, unsigned PReg, const TargetRegisterClass *RC) {
    unsigned VReg = MF.getRegInfo().createVirtualRegister(RC);
    MF.getRegInfo().addLiveIn(PReg, VReg);
    return VReg;
}

// // Passed first four i32 arguments in registers and others in stack.
// static bool analyzeCallArg(unsigned ValNo, MVT ValVT, CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags, CCState &State) {
//     static const MCPhysReg IntRegs[] = {P2::R0, P2::R1, P2::R2, P2::R3};

//     LLVM_DEBUG(errs() << "analyzing argument in call\n");

//     // Do not process byval args here.
//     if (ArgFlags.isByVal())
//         return true;

//     unsigned Reg;

//     // f32 and f64 are allocated in R0, R1 when either of the following
//     // is true: function is vararg, argument is 3rd or higher, there is previous
//     // argument which is not f32 or f64.
//     bool AllocateFloatsInIntReg = true;
//     unsigned OrigAlign = ArgFlags.getOrigAlign();
//     bool isI64 = (ValVT == MVT::i32 && OrigAlign == 8);

//     if (ValVT == MVT::i32 || (ValVT == MVT::f32 && AllocateFloatsInIntReg)) {
//         Reg = State.AllocateReg(IntRegs);
//         // If this is the first part of an i64 arg,
//         // the allocated register must be R0.
//         if (isI64 && (Reg == P2::R1))
//             Reg = State.AllocateReg(IntRegs);

//     } else {
//         llvm_unreachable("Can't handle this ValVT!");
//     }

//     if (!Reg) {
//         unsigned Offset = State.AllocateStack(ValVT.getSizeInBits() >> 3, OrigAlign);
//         LLVM_DEBUG(errs() << "allocating stack space for argument, offset = " << Offset << "\n");
//         State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, ValVT, LocInfo));
//     } else {
//         State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, ValVT, LocInfo));
//     }

//     return false;
// }

const char *P2TargetLowering::getTargetNodeName(unsigned Opcode) const {

    switch (Opcode) {
        case P2ISD::RET: return "P2RET";
        case P2ISD::CALL: return "P2CALL";
        case P2ISD::GAWRAPPER: return "P2GAWRAPPER";
        default:
            return nullptr;
    }
    #undef NODE
}

P2TargetLowering::P2TargetLowering(const P2TargetMachine &TM) : TargetLowering(TM) {
    addRegisterClass(MVT::i32, &P2::P2GPRRegClass);

    //  computeRegisterProperties - Once all of the register classes are
    //  added, this allows us to compute derived properties we expose.
    computeRegisterProperties(TM.getRegisterInfo());

    setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
}

SDValue P2TargetLowering::lowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const {
    auto DL = DAG.getDataLayout();

    const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
    int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();

    // Create the TargetGlobalAddress node, folding in the constant offset.
    SDValue Result = DAG.getTargetGlobalAddress(GV, SDLoc(Op), getPointerTy(DL), Offset);
    return DAG.getNode(P2ISD::GAWRAPPER, SDLoc(Op), getPointerTy(DL), Result);
}

#include "P2GenCallingConv.inc"

//===----------------------------------------------------------------------===//
//                  CALL Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue P2TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
    switch (Op.getOpcode()) {
        case ISD::GlobalAddress:
            return lowerGlobalAddress(Op, DAG);
    }
    return SDValue();
}

static const MCPhysReg O32IntRegs[] = {
    P2::R0, P2::R1, P2::R2, P2::R3
};

/// LowerCall - functions arguments are copied from virtual regs to
/// physical regs and/or the stack frame, CALLSEQ_START and CALLSEQ_END are emitted.
SDValue P2TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                              SmallVectorImpl<SDValue> &InVals) const {
    SelectionDAG &DAG                     = CLI.DAG;
    SDLoc DL                              = CLI.DL;
    SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
    SmallVectorImpl<SDValue> &OutVals     = CLI.OutVals;
    SmallVectorImpl<ISD::InputArg> &Ins   = CLI.Ins;
    SDValue Chain                         = CLI.Chain;
    SDValue Callee                        = CLI.Callee;
    CallingConv::ID CallConv              = CLI.CallConv;
    bool IsVarArg                         = CLI.IsVarArg;

    MachineFunction &MF = DAG.getMachineFunction();
    MachineFrameInfo *MFI = &MF.getFrameInfo();
    const TargetFrameLowering *TFL = MF.getSubtarget().getFrameLowering();
    P2FunctionInfo *FuncInfo = MF.getInfo<P2FunctionInfo>();
    P2FunctionInfo *P2FI = MF.getInfo<P2FunctionInfo>();

    // Analyze operands of the call, assigning locations to each operand.
    SmallVector<CCValAssign, 16> ArgLocs;
    CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs, *DAG.getContext());
    CCInfo.AnalyzeCallOperands(Outs, CC_P2);

    // Get a count of how many bytes are to be pushed on the stack.
    unsigned NextStackOffset = CCInfo.getNextStackOffset();

    LLVM_DEBUG(errs() << "Calling function. next stack offset is " << NextStackOffset << "\n");

    // Chain is the output chain of the last Load/Store or CopyToReg node.
    unsigned StackAlignment = TFL->getStackAlignment();
    NextStackOffset = alignTo(NextStackOffset, StackAlignment);

    // start the call sequence.
    Chain = DAG.getCALLSEQ_START(Chain, NextStackOffset, 0, DL);

    // get the current stack pointer value
    SDValue StackPtr = DAG.getCopyFromReg(Chain, DL, P2::SP, getPointerTy(DAG.getDataLayout()));

    // we have 4 args on registers
    std::deque<std::pair<unsigned, SDValue>> RegsToPass;
    SmallVector<SDValue, 8> MemOpChains;

    // iterate over the argument locations.
    // at each location, push a reg/value pair onto RegsToPass, promoting the type if needed.
    for (unsigned i = 0; i < ArgLocs.size(); i++) {
        SDValue Arg = OutVals[i];
        CCValAssign &VA = ArgLocs[i];
        MVT LocVT = VA.getLocVT();
        ISD::ArgFlagsTy Flags = Outs[i].Flags;

        // Promote the value if needed.
        switch (VA.getLocInfo()) {
            default: llvm_unreachable("Unknown loc info!");
            case CCValAssign::Full:
                break;
            case CCValAssign::SExt:
                Arg = DAG.getNode(ISD::SIGN_EXTEND, DL, LocVT, Arg);
                break;
            case CCValAssign::ZExt:
                Arg = DAG.getNode(ISD::ZERO_EXTEND, DL, LocVT, Arg);
                break;
            case CCValAssign::AExt:
                Arg = DAG.getNode(ISD::ANY_EXTEND, DL, LocVT, Arg);
                break;
        }

        // Arguments that can be passed on register must be kept at
        // RegsToPass vector
        if (VA.isRegLoc()) {
            RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
        } else {
            // Register can't get to this point...
            assert(VA.isMemLoc());

            SDValue PtrOff;
            int off = VA.getLocMemOffset()-4; // -4 because current SP is free

            LLVM_DEBUG(errs() << "offset is " << off << "\n");

            // if (off < 0) {
            PtrOff = DAG.getNode(ISD::SUB, DL, getPointerTy(DAG.getDataLayout()), StackPtr, DAG.getIntPtrConstant(-off, DL));
            // } else {
            //    PtrOff = DAG.getNode(ISD::ADD, DL, getPointerTy(DAG.getDataLayout()), StackPtr, DAG.getIntPtrConstant(off, DL));
            //}

            MemOpChains.push_back(DAG.getStore(Chain, DL, Arg, PtrOff, MachinePointerInfo()));
        }
    }

    // Transform all store nodes into one single node because all store
    // nodes are independent of each other.
    if (!MemOpChains.empty())
        Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, MemOpChains);

    // If the callee is a GlobalAddress/ExternalSymbol node (quite common, every
    // direct call is) turn it into a TargetGlobalAddress/TargetExternalSymbol
    // node so that legalize doesn't hack it.
    bool GlobalOrExternal = false, InternalLinkage = false;
    SDValue CalleeLo;
    EVT Ty = Callee.getValueType();

    if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
        Callee = DAG.getTargetGlobalAddress(G->getGlobal(), DL, getPointerTy(DAG.getDataLayout()), 0);
        GlobalOrExternal = true;
    }  else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
        const char *Sym = S->getSymbol();

        Callee = DAG.getTargetExternalSymbol(Sym, getPointerTy(DAG.getDataLayout()));

        GlobalOrExternal = true;
    }

    SmallVector<SDValue, 8> Ops(1, Chain);
    SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);

    // buit the list of CopyToReg operations.
    getOpndList(Ops, RegsToPass, false, GlobalOrExternal, InternalLinkage, CLI, Callee, Chain);

    // call the function
    Chain = DAG.getNode(P2ISD::CALL, DL, NodeTys, Ops);
    SDValue InFlag = Chain.getValue(1);

    // end teh call sequence
    Chain = DAG.getCALLSEQ_END(Chain, DAG.getIntPtrConstant(NextStackOffset, DL, true), DAG.getIntPtrConstant(0, DL, true), InFlag, DL);
    InFlag = Chain.getValue(1);

    // Handle result values, copying them out of physregs into vregs that we
    // return.
    return LowerCallResult(Chain, InFlag, CallConv, IsVarArg, Ins, DL, DAG, InVals, CLI.Callee.getNode(), CLI.RetTy);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue P2TargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                            CallingConv::ID CallConv, bool IsVarArg,
                                            const SmallVectorImpl<ISD::InputArg> &Ins,
                                            const SDLoc &dl, SelectionDAG &DAG,
                                            SmallVectorImpl<SDValue> &InVals,
                                            const SDNode *CallNode,
                                            const Type *RetTy) const {
    // Assign locations to each value returned by this call.
    SmallVector<CCValAssign, 16> RVLocs;
    CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs, *DAG.getContext());
    CCInfo.AnalyzeCallResult(Ins, RetCC_P2);

    SmallVector<std::pair<int, unsigned>, 4> ResultMemLocs;

    // Copy results out of physical registers.
    for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
        const CCValAssign &VA = RVLocs[i];
        if (VA.isRegLoc()) {
            SDValue RetValue;
            const TargetRegisterClass *RC = getRegClassFor(VA.getLocVT());

            // Transform the arguments stored on
            // physical registers into virtual ones
            DAG.getMachineFunction().getRegInfo().addLiveIn(VA.getLocReg());
            RetValue = DAG.getCopyFromReg(Chain, dl, VA.getLocReg(), VA.getValVT(), InFlag);
            InVals.push_back(RetValue);

            Chain = RetValue.getValue(1);
            InFlag = RetValue.getValue(2);
        } else {
            assert(VA.isMemLoc() && "Must be memory location.");
            llvm_unreachable("returning values via memory not yet supported!");
        }
    }

    // Transform all loads nodes into one single node because
    // all load nodes are independent of each other.
    // if (!MemOpChains.empty())
    //     Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);

    return Chain;

}

/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue P2TargetLowering::LowerFormalArguments(SDValue Chain,
                                                CallingConv::ID CallConv,
                                                bool IsVarArg,
                                                const SmallVectorImpl<ISD::InputArg> &Ins,
                                                const SDLoc &DL, SelectionDAG &DAG,
                                                SmallVectorImpl<SDValue> &InVals) const {
    MachineFunction &MF = DAG.getMachineFunction();
    MachineFrameInfo *MFI = &MF.getFrameInfo();
    P2FunctionInfo *P2FI = MF.getInfo<P2FunctionInfo>();

    SmallVector<SDValue, 12> MemOpChains;
    std::vector<SDValue> OutChains;

    // P2FI->setVarArgsFrameIndex(0);

    // Assign locations to all of the incoming arguments.
    SmallVector<CCValAssign, 16> ArgLocs;
    CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs, *DAG.getContext());
    CCInfo.AnalyzeFormalArguments(Ins, CC_P2);

    unsigned CurArgIdx = 0;

    // iterate over the argument locations and create copies to virtual registers
    for (unsigned i = 0; i < ArgLocs.size(); i++) {
        CCValAssign &VA = ArgLocs[i];
        EVT ValVT = VA.getValVT();
        ISD::ArgFlagsTy Flags = Ins[i].Flags;

        // Arguments stored on registers
        if (VA.isRegLoc()) {
            MVT RegVT = VA.getLocVT();
            unsigned ArgReg = VA.getLocReg();
            const TargetRegisterClass *RC = getRegClassFor(RegVT);

            // create a new virtual register for this frame and copy to it from
            // the current argument register
            unsigned Reg = addLiveIn(DAG.getMachineFunction(), ArgReg, RC);
            SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegVT);

            // If this is an 8 or 16-bit value, it has been passed promoted
            // to 32 bits.  Insert an assert[sz]ext to capture this, then
            // truncate to the right size.
            if (VA.getLocInfo() != CCValAssign::Full) {
                unsigned Opcode = 0;
                if (VA.getLocInfo() == CCValAssign::SExt) {
                    Opcode = ISD::AssertSext;
                } else if (VA.getLocInfo() == CCValAssign::ZExt) {
                    Opcode = ISD::AssertZext;
                }

                if (Opcode) {
                    ArgValue = DAG.getNode(Opcode, DL, RegVT, ArgValue, DAG.getValueType(ValVT));
                }

                ArgValue = DAG.getNode(ISD::TRUNCATE, DL, ValVT, ArgValue);
            }
            InVals.push_back(ArgValue);
        } else {
            // arguments stored in memory
            MVT LocVT = VA.getLocVT();

            // sanity check
            assert(VA.isMemLoc());

            // Load the argument to a virtual register
            unsigned ObjSize = VA.getLocVT().getStoreSize();
            assert((ObjSize <= 4) && "Unhandled argument--object size > stack slot size (4 bytes)");

            // Create the frame index object for this incoming parameter.
            int FI = MFI->CreateFixedObject(ObjSize, VA.getLocMemOffset(), true);

            // Create the SelectionDAG nodes corresponding to a load from this parameter
            SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
            SDValue ArgValue = DAG.getLoad(VA.getLocVT(), DL, Chain, FIN, MachinePointerInfo::getFixedStack(MF, FI));

            InVals.push_back(ArgValue);
            OutChains.push_back(ArgValue.getValue(1));
        }
    }

    // All stores are grouped in one node to allow the matching between
    // the size of Ins and InVals. This only happens when on varg functions
    if (!OutChains.empty()) {
        OutChains.push_back(Chain);
        Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, OutChains);
    }

    return Chain;
}

void P2TargetLowering::getOpndList(SmallVectorImpl<SDValue> &Ops,
            std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
            bool IsPICCall, bool GlobalOrExternal, bool InternalLinkage,
            CallLoweringInfo &CLI, SDValue Callee, SDValue Chain) const {

    Ops.push_back(Callee);

    // Build a sequence of copy-to-reg nodes chained together with token
    // chain and flag operands which copy the outgoing args into registers.
    // The InFlag in necessary since all emitted instructions must be
    // stuck together.
    SDValue InFlag;

    for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
        Chain = CLI.DAG.getCopyToReg(Chain, CLI.DL, RegsToPass[i].first, RegsToPass[i].second, InFlag);
        InFlag = Chain.getValue(1);
    }

    // Add argument registers to the end of the list so that they are
    // known live into the call.
    for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
        Ops.push_back(CLI.DAG.getRegister(RegsToPass[i].first, RegsToPass[i].second.getValueType()));

    if (InFlag.getNode())
        Ops.push_back(InFlag);
}

SDValue P2TargetLowering::LowerReturn(SDValue Chain,
                                        CallingConv::ID CallConv, bool IsVarArg,
                                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                                        const SmallVectorImpl<SDValue> &OutVals,
                                        const SDLoc &DL, SelectionDAG &DAG) const {

    // CCValAssign - represent the assignment of
    // the return value to a location
    SmallVector<CCValAssign, 16> RVLocs;
    MachineFunction &MF = DAG.getMachineFunction();

    // CCState - Info about the registers and stack slot.
    CCState CCInfo(CallConv, IsVarArg, MF, RVLocs, *DAG.getContext());

    // Analyze return values.
    CCInfo.AnalyzeReturn(Outs, RetCC_P2);

    SDValue Flag;
    SmallVector<SDValue, 4> RetOps(1, Chain);

    // Copy the result values into the output registers.
    for (unsigned i = 0; i != RVLocs.size(); ++i) {
        SDValue Val = OutVals[i];
        CCValAssign &VA = RVLocs[i];
        assert(VA.isRegLoc() && "Can only return in registers!");

        if (RVLocs[i].getValVT() != RVLocs[i].getLocVT())
            Val = DAG.getNode(ISD::BITCAST, DL, RVLocs[i].getLocVT(), Val);

        Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Val, Flag);

        // Guarantee that all emitted copies are stuck together with flags.
        Flag = Chain.getValue(1);
        RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
    }

    RetOps[0] = Chain;  // Update chain.

    // Add the flag if we have it.
    if (Flag.getNode())
        RetOps.push_back(Flag);

    // Return on P2 is always a "ret"
    return DAG.getNode(P2ISD::RET, DL, MVT::Other, RetOps);
}
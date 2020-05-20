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

#define DEBUG_TYPE "p2-lower"

// addLiveIn - This helper function adds the specified physical register to the
// MachineFunction as a live in value.  It also creates a corresponding
// virtual register for it.
static unsigned addLiveIn(MachineFunction &MF, unsigned PReg, const TargetRegisterClass *RC) {
    unsigned VReg = MF.getRegInfo().createVirtualRegister(RC);
    MF.getRegInfo().addLiveIn(PReg, VReg);
    return VReg;
}

// Passed first four i32 arguments in registers and others in stack.
static bool CC_P2_O32(unsigned ValNo, MVT ValVT, MVT LocVT, CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags, CCState &State) {
    static const MCPhysReg IntRegs[] = {P2::R0, P2::R1, P2::R3, P2::R4};

    // Do not process byval args here.
    if (ArgFlags.isByVal())
        return true;

    // Promote i8 and i16
    if (LocVT == MVT::i8 || LocVT == MVT::i16) {
        LocVT = MVT::i32;
        if (ArgFlags.isSExt())
            LocInfo = CCValAssign::SExt;
        else if (ArgFlags.isZExt())
            LocInfo = CCValAssign::ZExt;
        else
            LocInfo = CCValAssign::AExt;
    }

    unsigned Reg;

    // f32 and f64 are allocated in R0, R1 when either of the following
    // is true: function is vararg, argument is 3rd or higher, there is previous
    // argument which is not f32 or f64.
    bool AllocateFloatsInIntReg = true;
    unsigned OrigAlign = ArgFlags.getOrigAlign();
    bool isI64 = (ValVT == MVT::i32 && OrigAlign == 8);

    if (ValVT == MVT::i32 || (ValVT == MVT::f32 && AllocateFloatsInIntReg)) {
        Reg = State.AllocateReg(IntRegs);
        // If this is the first part of an i64 arg,
        // the allocated register must be R0.
        if (isI64 && (Reg == P2::R1))
            Reg = State.AllocateReg(IntRegs);

        LocVT = MVT::i32;
    } else if (ValVT == MVT::f64 && AllocateFloatsInIntReg) {
        // Allocate int register. If first available register is P2::R1, shadow it too.
        Reg = State.AllocateReg(IntRegs);
        if (Reg == P2::R1)
            Reg = State.AllocateReg(IntRegs);

        State.AllocateReg(IntRegs);
        LocVT = MVT::i32;
    } else
        llvm_unreachable("Cannot handle this ValVT.");

    if (!Reg) {
        unsigned Offset = State.AllocateStack(ValVT.getSizeInBits() >> 3, OrigAlign);
        State.addLoc(CCValAssign::getMem(ValNo, ValVT, Offset, LocVT, LocInfo));
    } else {
        State.addLoc(CCValAssign::getReg(ValNo, ValVT, Reg, LocVT, LocInfo));
    }

    return false;
}

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
//                  Call Calling Convention Implementation
//===----------------------------------------------------------------------===//

static const MCPhysReg O32IntRegs[] = {
    P2::R0, P2::R1, P2::R2, P2::R3
};

/// LowerCall - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
SDValue P2TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                              SmallVectorImpl<SDValue> &InVals) const {
    SelectionDAG &DAG                     = CLI.DAG;
    SDLoc DL                              = CLI.DL;
    SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
    SmallVectorImpl<SDValue> &OutVals     = CLI.OutVals;
    SmallVectorImpl<ISD::InputArg> &Ins   = CLI.Ins;
    SDValue Chain                         = CLI.Chain;
    SDValue Callee                        = CLI.Callee;
    bool &IsTailCall                      = CLI.IsTailCall;
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
    P2CC::SpecialCallingConvType SpecialCallingConv = getSpecialCallingConv(Callee);
    P2CC P2CCInfo(CallConv, CCInfo, SpecialCallingConv);

    P2CCInfo.analyzeCallOperands(Outs, IsVarArg, true, Callee.getNode(), CLI.getArgs());

    // Get a count of how many bytes are to be pushed on the stack.
    unsigned NextStackOffset = CCInfo.getNextStackOffset();

    // Chain is the output chain of the last Load/Store or CopyToReg node.
    // ByValChain is the output chain of the last Memcpy node created for copying
    // byval arguments to the stack.
    unsigned StackAlignment = TFL->getStackAlignment();
    NextStackOffset = alignTo(NextStackOffset, StackAlignment);
    //SDValue NextStackOffsetVal = DAG.getIntPtrConstant(NextStackOffset, DL, true);
    unsigned NextStackOffsetVal = CCInfo.getNextStackOffset();

    Chain = DAG.getCALLSEQ_START(Chain, NextStackOffsetVal, 0, DL);

    SDValue StackPtr = DAG.getCopyFromReg(Chain, DL, P2::SP, getPointerTy(DAG.getDataLayout()));

    // we have 4 args on registers
    std::deque<std::pair<unsigned, SDValue>> RegsToPass;
    SmallVector<SDValue, 8> MemOpChains;
    P2CC::byval_iterator ByValArg = P2CCInfo.byval_begin();

    // Walk the register/memloc assignments, inserting copies/loads.
    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
        SDValue Arg = OutVals[i];
        CCValAssign &VA = ArgLocs[i];
        MVT LocVT = VA.getLocVT();
        ISD::ArgFlagsTy Flags = Outs[i].Flags;

        if (Flags.isByVal()) {
            assert(Flags.getByValSize() && "ByVal args of size 0 should have been ignored by front-end.");
            assert(ByValArg != P2CCInfo.byval_end());
            assert(!IsTailCall && "Do not tail-call optimize if there is a byval argument.");
            passByValArg(Chain, DL, RegsToPass, MemOpChains, StackPtr, MFI, DAG, Arg, P2CCInfo, *ByValArg, Flags);
            ++ByValArg;
            continue;
        }

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
            continue;
        }

        // Register can't get to this point...
        assert(VA.isMemLoc());

        // emit ISD::STORE whichs stores the
        // parameter value to a stack Location
        MemOpChains.push_back(passArgOnStack(StackPtr, VA.getLocMemOffset(), Chain, Arg, DL, IsTailCall, DAG));
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

    getOpndList(Ops, RegsToPass, false, GlobalOrExternal, InternalLinkage, CLI, Callee, Chain);

    // if (IsTailCall)
    //     return DAG.getNode(P2ISD::TailCall, DL, MVT::Other, Ops);

    Chain = DAG.getNode(P2ISD::CALL, DL, NodeTys, Ops);
    SDValue InFlag = Chain.getValue(1);

    // Create the CALLSEQ_END node.
    Chain = DAG.getCALLSEQ_END(Chain, DAG.getIntPtrConstant(NextStackOffsetVal, DL, true), DAG.getIntPtrConstant(0, DL, true), InFlag, DL);
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

    P2CC P2CCInfo(CallConv, CCInfo);

    P2CCInfo.analyzeCallResult(Ins, true, CallNode, RetTy);

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
            //addLiveIn(DAG.getMachineFunction(), VA.getLocReg(), RC);
            RetValue = DAG.getCopyFromReg(Chain, dl, VA.getLocReg(), VA.getValVT(), InFlag);
            InVals.push_back(RetValue);

            Chain = RetValue.getValue(1);
            InFlag = RetValue.getValue(2);
        } else {
            assert(VA.isMemLoc() && "Must be memory location.");
            ResultMemLocs.push_back(std::make_pair(VA.getLocMemOffset(), InVals.size()));

            // Reserve space for this result.
            InVals.push_back(SDValue());
        }
    }

    // Copy results out of memory.
    SmallVector<SDValue, 4> MemOpChains;
    for (unsigned i = 0, e = ResultMemLocs.size(); i != e; ++i) {
        int Offset = ResultMemLocs[i].first;
        unsigned Index = ResultMemLocs[i].second;
        SDValue StackPtr = DAG.getRegister(P2::SP, MVT::i32);
        SDValue SpLoc = DAG.getNode(ISD::ADD, dl, MVT::i32, StackPtr, DAG.getConstant(Offset, dl, MVT::i32));
        SDValue Load = DAG.getLoad(MVT::i32, dl, Chain, SpLoc, MachinePointerInfo());
        InVals[Index] = Load;
        MemOpChains.push_back(Load.getValue(1));
    }

    // Transform all loads nodes into one single node because
    // all load nodes are independent of each other.
    if (!MemOpChains.empty())
        Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);

  return Chain;

}

/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue
P2TargetLowering::LowerFormalArguments(SDValue Chain,
                                         CallingConv::ID CallConv,
                                         bool IsVarArg,
                                         const SmallVectorImpl<ISD::InputArg> &Ins,
                                         const SDLoc &DL, SelectionDAG &DAG,
                                         SmallVectorImpl<SDValue> &InVals)
                                          const {
    MachineFunction &MF = DAG.getMachineFunction();
    MachineFrameInfo *MFI = &MF.getFrameInfo();
    P2FunctionInfo *P2FI = MF.getInfo<P2FunctionInfo>();

    P2FI->setVarArgsFrameIndex(0);

    // Assign locations to all of the incoming arguments.
    SmallVector<CCValAssign, 16> ArgLocs;
    CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs, *DAG.getContext());
    P2CC P2CCInfo(CallConv, CCInfo);

    Function::const_arg_iterator FuncArg = DAG.getMachineFunction().getFunction().arg_begin();
    bool UseSoftFloat = true;

    P2CCInfo.analyzeFormalArguments(Ins, UseSoftFloat, FuncArg);
    P2FI->setFormalArgInfo(CCInfo.getNextStackOffset(), P2CCInfo.hasByValArg());

    // Used with vargs to acumulate store chains.
    std::vector<SDValue> OutChains;

    unsigned CurArgIdx = 0;
    P2CC::byval_iterator ByValArg = P2CCInfo.byval_begin();

    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
        CCValAssign &VA = ArgLocs[i];

        if (Ins[i].isOrigArg()) {
            std::advance(FuncArg, Ins[i].getOrigArgIndex() - CurArgIdx);
            CurArgIdx = Ins[i].getOrigArgIndex();
        }

        EVT ValVT = VA.getValVT();
        ISD::ArgFlagsTy Flags = Ins[i].Flags;
        bool IsRegLoc = VA.isRegLoc();

        if (Flags.isByVal()) {
            assert(Flags.getByValSize() && "ByVal args of size 0 should have been ignored by front-end.");
            assert(ByValArg != P2CCInfo.byval_end());
            copyByValRegs(Chain, DL, OutChains, DAG, Flags, InVals, &*FuncArg, P2CCInfo, *ByValArg);
            ++ByValArg;
            continue;
        }

        // Arguments stored on registers
        if (IsRegLoc) {
            MVT RegVT = VA.getLocVT();
            unsigned ArgReg = VA.getLocReg();
            const TargetRegisterClass *RC = getRegClassFor(RegVT);

            // Transform the arguments stored on
            // physical registers into virtual ones
            unsigned Reg = addLiveIn(DAG.getMachineFunction(), ArgReg, RC);
            SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegVT);

            // If this is an 8 or 16-bit value, it has been passed promoted
            // to 32 bits.  Insert an assert[sz]ext to capture this, then
            // truncate to the right size.
            if (VA.getLocInfo() != CCValAssign::Full) {
                unsigned Opcode = 0;
                if (VA.getLocInfo() == CCValAssign::SExt)
                    Opcode = ISD::AssertSext;
                else if (VA.getLocInfo() == CCValAssign::ZExt)
                    Opcode = ISD::AssertZext;

                if (Opcode)
                    ArgValue = DAG.getNode(Opcode, DL, RegVT, ArgValue, DAG.getValueType(ValVT));
                ArgValue = DAG.getNode(ISD::TRUNCATE, DL, ValVT, ArgValue);
            }

            // Handle floating point arguments passed in integer registers.
            if ((RegVT == MVT::i32 && ValVT == MVT::f32) || (RegVT == MVT::i64 && ValVT == MVT::f64))
                ArgValue = DAG.getNode(ISD::BITCAST, DL, ValVT, ArgValue);

            InVals.push_back(ArgValue);
        } else { // VA.isRegLoc()
            MVT LocVT = VA.getLocVT();

            // sanity check
            assert(VA.isMemLoc());

            // The stack pointer offset is relative to the caller stack frame.
            int FI = MFI->CreateFixedObject(ValVT.getSizeInBits()/8, VA.getLocMemOffset(), true);

            // Create load nodes to retrieve arguments from the stack
            SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
            SDValue Load = DAG.getLoad(LocVT, DL, Chain, FIN, MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI));
            InVals.push_back(Load);
            OutChains.push_back(Load.getValue(1));
        }
    }

    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
        // The P2 ABIs for returning structs by value requires that we copy
        // the sret argument into $R15 for the return. Save the argument into
        // a virtual register so that we can access it from the return points.

        // TODO update this doc string once we know what the hell it means for propeller

        if (Ins[i].Flags.isSRet()) {
            unsigned Reg = P2FI->getSRetReturnReg();
            if (!Reg) {
                Reg = MF.getRegInfo().createVirtualRegister(getRegClassFor(MVT::i32));
                P2FI->setSRetReturnReg(Reg);
            }

            SDValue Copy = DAG.getCopyToReg(DAG.getEntryNode(), DL, Reg, InVals[i]);
            Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, Copy, Chain);
            break;
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

SDValue P2TargetLowering::passArgOnStack(SDValue StackPtr, unsigned Offset,
                                   SDValue Chain, SDValue Arg, const SDLoc &DL,
                                   bool IsTailCall, SelectionDAG &DAG) const {
    if (!IsTailCall) {
        SDValue PtrOff = DAG.getNode(ISD::ADD, DL, getPointerTy(DAG.getDataLayout()), StackPtr, DAG.getIntPtrConstant(Offset, DL));
        return DAG.getStore(Chain, DL, Arg, PtrOff, MachinePointerInfo());
    }

    MachineFrameInfo *MFI = &DAG.getMachineFunction().getFrameInfo();
    int FI = MFI->CreateFixedObject(Arg.getValueSizeInBits() / 8, Offset, false);
    SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
    return DAG.getStore(Chain, DL, Arg, FIN, MachinePointerInfo(),
                      /* Alignment = */ 0, MachineMemOperand::MOVolatile);
}

void P2TargetLowering::getOpndList(SmallVectorImpl<SDValue> &Ops,
            std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
            bool IsPICCall, bool GlobalOrExternal, bool InternalLinkage,
            CallLoweringInfo &CLI, SDValue Callee, SDValue Chain) const {
  // T9 should contain the address of the callee function if
  // -reloction-model=pic or it is an indirect call.
  // figure out what T9 is and if we need to make an equivalint

  // if (IsPICCall || !GlobalOrExternal) {
  //   unsigned T9Reg = P2::T9;
  //   RegsToPass.push_front(std::make_pair(T9Reg, Callee));
  // } else

    Ops.push_back(Callee);

    // Insert node "GP copy globalreg" before call to function.
    //
    // R_P2_CALL* operators (emitted when non-internal functions are called
    // in PIC mode) allow symbols to be resolved via lazy binding.
    // The lazy binding stub requires GP to point to the GOT.
    // if (IsPICCall && !InternalLinkage) {
    //   unsigned GPReg = P2::GP;
    //   EVT Ty = MVT::i32;
    //   RegsToPass.push_back(std::make_pair(GPReg, getGlobalReg(CLI.DAG, Ty)));
    // }

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

    // Add a register mask operand representing the call-preserved registers.
    // const TargetRegisterInfo *TRI = Subtarget.getRegisterInfo();
    // const uint32_t *Mask = TRI->getCallPreservedMask(CLI.DAG.getMachineFunction(), CLI.CallConv);
    // assert(Mask && "Missing call preserved mask for calling convention");
    // Ops.push_back(CLI.DAG.getRegisterMask(Mask));

    if (InFlag.getNode())
        Ops.push_back(InFlag);
}

P2TargetLowering::P2CC::SpecialCallingConvType P2TargetLowering::getSpecialCallingConv(SDValue Callee) const {
        P2CC::SpecialCallingConvType SpecialCallingConv = P2CC::NoSpecialCallingConv;

    return SpecialCallingConv;
}

void P2TargetLowering::P2CC::allocateRegs(ByValArgInfo &ByVal, unsigned ByValSize, unsigned Align) {
    unsigned RegSize = regSize(), NumIntArgRegs = numIntArgRegs();
    const ArrayRef<MCPhysReg> IntArgRegs = intArgRegs();
    assert(!(ByValSize % RegSize) && !(Align % RegSize) &&
            "Byval argument's size and alignment should be a multiple of RegSize.");

    ByVal.FirstIdx = CCInfo.getFirstUnallocated(IntArgRegs);

    // If Align > RegSize, the first arg register must be even.
    if ((Align > RegSize) && (ByVal.FirstIdx % 2)) {
        CCInfo.AllocateReg(IntArgRegs[ByVal.FirstIdx]);
        ++ByVal.FirstIdx;
    }

    // Mark the registers allocated.
    for (unsigned I = ByVal.FirstIdx; ByValSize && (I < NumIntArgRegs);
        ByValSize -= RegSize, ++I, ++ByVal.NumRegs)

    CCInfo.AllocateReg(IntArgRegs[I]);
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
    P2CC P2CCInfo(CallConv,CCInfo);

    // Analyze return values.
    P2CCInfo.analyzeReturn(Outs, true, MF.getFunction().getReturnType());

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

      // The P2 ABIs for returning structs by value requires that we copy
      // the sret argument into $r15 for the return. We saved the argument into
      // a virtual register in the entry block, so now we copy the value out
      // and into $r15.
    if (MF.getFunction().hasStructRetAttr()) {
        P2FunctionInfo *P2FI = MF.getInfo<P2FunctionInfo>();
        unsigned Reg = P2FI->getSRetReturnReg();

        if (!Reg)
            llvm_unreachable("sret virtual register not created in the entry block");

        SDValue Val = DAG.getCopyFromReg(Chain, DL, Reg, getPointerTy(DAG.getDataLayout()));
        unsigned R15 = P2::R15;

        Chain = DAG.getCopyToReg(Chain, DL, R15, Val, Flag);
        Flag = Chain.getValue(1);
        RetOps.push_back(DAG.getRegister(R15, getPointerTy(DAG.getDataLayout())));
    }

    RetOps[0] = Chain;  // Update chain.

    // Add the flag if we have it.
    if (Flag.getNode())
        RetOps.push_back(Flag);

    // Return on P2 is always a "ret"
    return DAG.getNode(P2ISD::RET, DL, MVT::Other, RetOps);
}

SDValue P2TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
    switch (Op.getOpcode()) {
        case ISD::GlobalAddress:
            return lowerGlobalAddress(Op, DAG);
    }
    return SDValue();
}

void P2TargetLowering::copyByValRegs(SDValue Chain, const SDLoc &DL, std::vector<SDValue> &OutChains,
              SelectionDAG &DAG, const ISD::ArgFlagsTy &Flags,
              SmallVectorImpl<SDValue> &InVals, const Argument *FuncArg,
              const P2CC &CC, const ByValArgInfo &ByVal) const {

    MachineFunction &MF = DAG.getMachineFunction();
    MachineFrameInfo *MFI = &MF.getFrameInfo();
    unsigned RegAreaSize = ByVal.NumRegs * CC.regSize();
    unsigned FrameObjSize = std::max(Flags.getByValSize(), RegAreaSize);
    int FrameObjOffset;

    const ArrayRef<MCPhysReg> ByValArgRegs = CC.intArgRegs();

    if (RegAreaSize)
        FrameObjOffset = (int)CC.reservedArgArea() - (int)((CC.numIntArgRegs() - ByVal.FirstIdx) * CC.regSize());
    else
        FrameObjOffset = ByVal.Address;

    // Create frame object.
    EVT PtrTy = getPointerTy(DAG.getDataLayout());
    int FI = MFI->CreateFixedObject(FrameObjSize, FrameObjOffset, true);
    SDValue FIN = DAG.getFrameIndex(FI, PtrTy);
    InVals.push_back(FIN);

    if (!ByVal.NumRegs)
        return;

    // Copy arg registers.
    MVT RegTy = MVT::getIntegerVT(CC.regSize() * 8);
    const TargetRegisterClass *RC = getRegClassFor(RegTy);

    for (unsigned I = 0; I < ByVal.NumRegs; ++I) {
        unsigned ArgReg = ByValArgRegs[ByVal.FirstIdx + I];
        unsigned VReg = addLiveIn(MF, ArgReg, RC);
        unsigned Offset = I * CC.regSize();
        SDValue StorePtr = DAG.getNode(ISD::ADD, DL, PtrTy, FIN, DAG.getConstant(Offset, DL, PtrTy));
        SDValue Store = DAG.getStore(Chain, DL, DAG.getRegister(VReg, RegTy), StorePtr, MachinePointerInfo(FuncArg, Offset));
        OutChains.push_back(Store);
    }
}

// Copy byVal arg to registers and stack.
void P2TargetLowering::passByValArg(SDValue Chain, const SDLoc &DL,
             std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
             SmallVectorImpl<SDValue> &MemOpChains, SDValue StackPtr,
             MachineFrameInfo *MFI, SelectionDAG &DAG, SDValue Arg,
             const P2CC &CC, const ByValArgInfo &ByVal,
             const ISD::ArgFlagsTy &Flags) const {

    unsigned ByValSizeInBytes = Flags.getByValSize();
    unsigned OffsetInBytes = 0; // From beginning of struct
    unsigned RegSizeInBytes = CC.regSize();
    unsigned Alignment = std::min((unsigned)Flags.getNonZeroByValAlign().value(), RegSizeInBytes);
    EVT PtrTy = getPointerTy(DAG.getDataLayout()),
        RegTy = MVT::getIntegerVT(RegSizeInBytes * 4);

    if (ByVal.NumRegs) {
        const ArrayRef<MCPhysReg> ArgRegs = CC.intArgRegs();
        bool LeftoverBytes = (ByVal.NumRegs * RegSizeInBytes > ByValSizeInBytes);
        unsigned I = 0;

        // Copy words to registers.
        for (; I < ByVal.NumRegs - LeftoverBytes; ++I, OffsetInBytes += RegSizeInBytes) {
            SDValue LoadPtr = DAG.getNode(ISD::ADD, DL, PtrTy, Arg, DAG.getConstant(OffsetInBytes, DL, PtrTy));
            SDValue LoadVal = DAG.getLoad(RegTy, DL, Chain, LoadPtr, MachinePointerInfo());
            MemOpChains.push_back(LoadVal.getValue(1));
            unsigned ArgReg = ArgRegs[ByVal.FirstIdx + I];
            RegsToPass.push_back(std::make_pair(ArgReg, LoadVal));
        }

        // Return if the struct has been fully copied.
        if (ByValSizeInBytes == OffsetInBytes)
            return;

        // Copy the remainder of the byval argument with sub-word loads and shifts.
        if (LeftoverBytes) {
            assert((ByValSizeInBytes > OffsetInBytes) && (ByValSizeInBytes < OffsetInBytes + RegSizeInBytes) &&
             "Size of the remainder should be smaller than RegSizeInBytes.");

            SDValue Val;

            for (unsigned LoadSizeInBytes = RegSizeInBytes / 2, TotalBytesLoaded = 0;
                    OffsetInBytes < ByValSizeInBytes; LoadSizeInBytes /= 2) {

                unsigned RemainingSizeInBytes = ByValSizeInBytes - OffsetInBytes;

                if (RemainingSizeInBytes < LoadSizeInBytes)
                    continue;

                // Load subword.
                SDValue LoadPtr = DAG.getNode(ISD::ADD, DL, PtrTy, Arg, DAG.getConstant(OffsetInBytes, DL, PtrTy));
                SDValue LoadVal = DAG.getExtLoad(ISD::ZEXTLOAD, DL, RegTy, Chain, LoadPtr, MachinePointerInfo(),
                    MVT::getIntegerVT(LoadSizeInBytes * 8), Alignment);

                MemOpChains.push_back(LoadVal.getValue(1));

                // Shift the loaded value.
                unsigned Shamt = (RegSizeInBytes - (TotalBytesLoaded + LoadSizeInBytes)) * 8;

                SDValue Shift = DAG.getNode(ISD::SHL, DL, RegTy, LoadVal, DAG.getConstant(Shamt, DL, MVT::i32));

                if (Val.getNode()) {
                    Val = DAG.getNode(ISD::OR, DL, RegTy, Val, Shift);
                } else {
                    Val = Shift;
                }

                OffsetInBytes += LoadSizeInBytes;
                TotalBytesLoaded += LoadSizeInBytes;
                Alignment = std::min(Alignment, LoadSizeInBytes);
            }

            unsigned ArgReg = ArgRegs[ByVal.FirstIdx + I];
            RegsToPass.push_back(std::make_pair(ArgReg, Val));
            return;
        }
    }

    // Copy remainder of byval arg to it with memcpy.
    unsigned MemCpySize = ByValSizeInBytes - OffsetInBytes;
    SDValue Src = DAG.getNode(ISD::ADD, DL, PtrTy, Arg, DAG.getConstant(OffsetInBytes, DL, PtrTy));
    SDValue Dst = DAG.getNode(ISD::ADD, DL, PtrTy, StackPtr, DAG.getIntPtrConstant(ByVal.Address, DL));
    Chain = DAG.getMemcpy(Chain, DL, Dst, Src,
                        DAG.getConstant(MemCpySize, DL, PtrTy),
                        Align(Alignment), /*isVolatile=*/false, /*AlwaysInline=*/false,
                        /*isTailCall=*/false,
                        MachinePointerInfo(), MachinePointerInfo());
    MemOpChains.push_back(Chain);
}

P2TargetLowering::P2CC::P2CC(CallingConv::ID CC, CCState &Info, P2CC::SpecialCallingConvType SpecialCallingConv_)
    : CCInfo(Info), CallConv(CC) {
    // Pre-allocate reserved argument area.
    CCInfo.AllocateStack(reservedArgArea(), 1);
}

void P2TargetLowering::P2CC::analyzeCallOperands(const SmallVectorImpl<ISD::OutputArg> &Args,
                                                bool IsVarArg, bool IsSoftFloat, const SDNode *CallNode,
                                                std::vector<ArgListEntry> &FuncArgs) {

    assert((CallConv != CallingConv::Fast || !IsVarArg) &&
            "CallingConv::Fast shouldn't be used for vararg functions.");

    unsigned NumOpnds = Args.size();
    llvm::CCAssignFn *FixedFn = fixedArgFn();
    llvm::CCAssignFn *VarFn = varArgFn();

    for (unsigned I = 0; I != NumOpnds; ++I) {
        MVT ArgVT = Args[I].VT;
        ISD::ArgFlagsTy ArgFlags = Args[I].Flags;
        bool R;

        if (ArgFlags.isByVal()) {
            handleByValArg(I, ArgVT, ArgVT, CCValAssign::Full, ArgFlags);
            continue;
        }

        if (IsVarArg && !Args[I].IsFixed)
            R = VarFn(I, ArgVT, ArgVT, CCValAssign::Full, ArgFlags, CCInfo);
        else {
            MVT RegVT = getRegVT(ArgVT, FuncArgs[Args[I].OrigArgIndex].Ty, CallNode, IsSoftFloat);
            R = FixedFn(I, ArgVT, RegVT, CCValAssign::Full, ArgFlags, CCInfo);
        }

        if (R) {
#ifndef NDEBUG
            dbgs() << "Call operand #" << I << " has unhandled type " << EVT(ArgVT).getEVTString();
#endif
            llvm_unreachable(nullptr);
        }
    }
}

void P2TargetLowering::P2CC::analyzeFormalArguments(const SmallVectorImpl<ISD::InputArg> &Args,
                                                    bool IsSoftFloat, Function::const_arg_iterator FuncArg) {

    unsigned NumArgs = Args.size();
    llvm::CCAssignFn *FixedFn = fixedArgFn();
    unsigned CurArgIdx = 0;

    for (unsigned I = 0; I != NumArgs; ++I) {
        MVT ArgVT = Args[I].VT;
        ISD::ArgFlagsTy ArgFlags = Args[I].Flags;
        if (Args[I].isOrigArg()) {
            std::advance(FuncArg, Args[I].getOrigArgIndex() - CurArgIdx);
            CurArgIdx = Args[I].getOrigArgIndex();
        }

        CurArgIdx = Args[I].OrigArgIndex;

        if (ArgFlags.isByVal()) {
            handleByValArg(I, ArgVT, ArgVT, CCValAssign::Full, ArgFlags);
            continue;
        }

        MVT RegVT = getRegVT(ArgVT, FuncArg->getType(), nullptr, IsSoftFloat);

        if (!FixedFn(I, ArgVT, RegVT, CCValAssign::Full, ArgFlags, CCInfo))
            continue;

#ifndef NDEBUG
        dbgs() << "Formal Arg #" << I << " has unhandled type " << EVT(ArgVT).getEVTString();
#endif
        llvm_unreachable(nullptr);
    }
}

template<typename Ty> void P2TargetLowering::P2CC::analyzeReturn(const SmallVectorImpl<Ty> &RetVals, bool IsSoftFloat,
              const SDNode *CallNode, const Type *RetTy) const {
    CCAssignFn *Fn;

    Fn = RetCC_P2;

    for (unsigned I = 0, E = RetVals.size(); I < E; ++I) {
        MVT VT = RetVals[I].VT;
        ISD::ArgFlagsTy Flags = RetVals[I].Flags;
        MVT RegVT = this->getRegVT(VT, RetTy, CallNode, IsSoftFloat);

        if (Fn(I, VT, RegVT, CCValAssign::Full, Flags, this->CCInfo)) {

            #ifndef NDEBUG
            dbgs() << "Call result #" << I << " has unhandled type "
                    << EVT(VT).getEVTString() << '\n';
            #endif
            llvm_unreachable(nullptr);
        }
    }
}

void P2TargetLowering::P2CC::analyzeCallResult(const SmallVectorImpl<ISD::InputArg> &Ins, bool IsSoftFloat,
                  const SDNode *CallNode, const Type *RetTy) const {
    analyzeReturn(Ins, IsSoftFloat, CallNode, RetTy);
}

void P2TargetLowering::P2CC::analyzeReturn(const SmallVectorImpl<ISD::OutputArg> &Outs, bool IsSoftFloat,
              const Type *RetTy) const {
    analyzeReturn(Outs, IsSoftFloat, nullptr, RetTy);
}

void P2TargetLowering::P2CC::handleByValArg(unsigned ValNo, MVT ValVT,
                                                MVT LocVT,
                                                CCValAssign::LocInfo LocInfo,
                                                ISD::ArgFlagsTy ArgFlags) {
    assert(ArgFlags.getByValSize() && "Byval argument's size shouldn't be 0.");

    struct ByValArgInfo ByVal;
    unsigned RegSize = regSize();
    unsigned ByValSize = alignTo(ArgFlags.getByValSize(), RegSize);
    unsigned Align = RegSize; //std::min(std::max(ArgFlags.getNonZeroByValAlign(), RegSize), RegSize * 2);

    if (useRegsForByval())
        allocateRegs(ByVal, ByValSize, Align);

    // Allocate space on caller's stack.
    ByVal.Address = CCInfo.AllocateStack(ByValSize - RegSize * ByVal.NumRegs, Align);
    CCInfo.addLoc(CCValAssign::getMem(ValNo, ValVT, ByVal.Address, LocVT, LocInfo));
    ByValArgs.push_back(ByVal);
}

unsigned P2TargetLowering::P2CC::numIntArgRegs() const {
    return array_lengthof(O32IntRegs);
}
const ArrayRef<MCPhysReg> P2TargetLowering::P2CC::intArgRegs() const {
    return makeArrayRef(O32IntRegs);
}

llvm::CCAssignFn *P2TargetLowering::P2CC::fixedArgFn() const {
    return CC_P2_O32;
}

llvm::CCAssignFn *P2TargetLowering::P2CC::varArgFn() const {
    return CC_P2_O32;
}

unsigned P2TargetLowering::P2CC::reservedArgArea() const {
    return (CallConv != CallingConv::Fast) ? 16 : 0;
}

MVT P2TargetLowering::P2CC::getRegVT(MVT VT, const Type *OrigTy,
                                         const SDNode *CallNode,
                                         bool IsSoftFloat) const {
    return VT;
}
//===-- P2ISelLowering.h - P2 DAG Lowering Interface --------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_P2_P2ISELLOWERING_H
#define LLVM_LIB_TARGET_P2_P2ISELLOWERING_H

#include "P2.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/TargetLowering.h"
#include <deque>

namespace llvm {

    namespace P2ISD {

    /// AVR Specific DAG Nodes
    enum NodeType {
        /// Start the numbering where the builtin ops leave off.
        FIRST_NUMBER = ISD::BUILTIN_OP_END,
        /// Return from subroutine.
        RET_FLAG,
        /// Return from ISR.
        RETI_FLAG,
        /// Represents an abstract call instruction,
        /// which includes a bunch of information.
        CALL,
        /// A wrapper node for TargetConstantPool,
        /// TargetExternalSymbol, and TargetGlobalAddress.
        WRAPPER,
        LSL,     ///< Logical shift left.
        LSR,     ///< Logical shift right.
        ASR,     ///< Arithmetic shift right.
        ROR,     ///< Bit rotate right.
        ROL,     ///< Bit rotate left.
        LSLLOOP, ///< A loop of single logical shift left instructions.
        LSRLOOP, ///< A loop of single logical shift right instructions.
        ROLLOOP, ///< A loop of single left bit rotate instructions.
        RORLOOP, ///< A loop of single right bit rotate instructions.
        ASRLOOP, ///< A loop of single arithmetic shift right instructions.
        /// AVR conditional branches. Operand 0 is the chain operand, operand 1
        /// is the block to branch if condition is true, operand 2 is the
        /// condition code, and operand 3 is the flag operand produced by a CMP
        /// or TEST instruction.
        BRCOND,
        /// Compare instruction.
        CMP,
        /// Compare with carry instruction.
        CMPC,
        /// Test for zero or minus instruction.
        TST,
        /// Operand 0 and operand 1 are selection variable, operand 2
        /// is condition code and operand 3 is flag operand.
        SELECT_CC
    };

} // end of namespace AVRISD


    //===--------------------------------------------------------------------===//
    // TargetLowering Implementation
    //===--------------------------------------------------------------------===//
    class P2FunctionInfo;
    class P2Subtarget;

    //@class P2TargetLowering
    class P2TargetLowering : public TargetLowering  {
    public:
        explicit P2TargetLowering(const P2TargetMachine &TM, const P2Subtarget &STI);

        /// getTargetNodeName - This method returns the name of a target specific
        //  DAG node.
        const char *getTargetNodeName(unsigned Opcode) const override;

    protected:
        const P2Subtarget &Subtarget;

    private:

        // Lower Operand specifics
        SDValue lowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;

        //- must be exist even without function all
        SDValue LowerFormalArguments(SDValue Chain,
                                       CallingConv::ID CallConv, bool IsVarArg,
                                       const SmallVectorImpl<ISD::InputArg> &Ins,
                                       const SDLoc &dl, SelectionDAG &DAG,
                                       SmallVectorImpl<SDValue> &InVals) const override;

        SDValue LowerReturn(SDValue Chain,
                            CallingConv::ID CallConv, bool IsVarArg,
                            const SmallVectorImpl<ISD::OutputArg> &Outs,
                            const SmallVectorImpl<SDValue> &OutVals,
                            const SDLoc &dl, SelectionDAG &DAG) const override;

    };
}

#endif // P2ISELLOWERING_H
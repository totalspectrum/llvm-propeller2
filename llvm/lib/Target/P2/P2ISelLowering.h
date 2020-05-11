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

        /// P2 Specific DAG Nodes
        enum NodeType {
            /// Start the numbering where the builtin ops leave off.
            FIRST_NUMBER = ISD::BUILTIN_OP_END,
            /// Return from subroutine.
            RET,
        };

    } // end of namespace P2ISD


    //===--------------------------------------------------------------------===//
    // TargetLowering Implementation
    //===--------------------------------------------------------------------===//
    class P2FunctionInfo;

    //@class P2TargetLowering
    class P2TargetLowering : public TargetLowering  {
    public:
        explicit P2TargetLowering(const P2TargetMachine &TM);

        /// getTargetNodeName - This method returns the name of a target specific
        //  DAG node.
        const char *getTargetNodeName(unsigned Opcode) const override;

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
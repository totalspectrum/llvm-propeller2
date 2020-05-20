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
            CALL,
            GAWRAPPER
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

        SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

    private:

        void getOpndList(SmallVectorImpl<SDValue> &Ops,
                std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
                bool IsPICCall, bool GlobalOrExternal, bool InternalLinkage,
                CallLoweringInfo &CLI, SDValue Callee, SDValue Chain) const;

         /// ByValArgInfo - Byval argument information.
        struct ByValArgInfo {
            unsigned FirstIdx; // Index of the first register used.
            unsigned NumRegs;  // Number of registers used for this argument.
            unsigned Address;  // Offset of the stack area used to pass this argument.

            ByValArgInfo() : FirstIdx(0), NumRegs(0), Address(0) {}
        };

        class P2CC {

        public:
            enum SpecialCallingConvType {
                NoSpecialCallingConv
            };

            P2CC(CallingConv::ID CallConv, CCState &Info, SpecialCallingConvType SpecialCallingConv = NoSpecialCallingConv);

            void analyzeCallOperands(const SmallVectorImpl<ISD::OutputArg> &Outs, bool IsVarArg, bool IsSoftFloat,
                                        const SDNode *CallNode,
                                        std::vector<ArgListEntry> &FuncArgs);

            void analyzeFormalArguments(const SmallVectorImpl<ISD::InputArg> &Ins, bool IsSoftFloat,
                                        Function::const_arg_iterator FuncArg);

            void analyzeCallResult(const SmallVectorImpl<ISD::InputArg> &Ins, bool IsSoftFloat,
                                    const SDNode *CallNode, const Type *RetTy) const;

            void analyzeReturn(const SmallVectorImpl<ISD::OutputArg> &Outs, bool IsSoftFloat, const Type *RetTy) const;

            const CCState &getCCInfo() const { return CCInfo; }

            /// hasByValArg - Returns true if function has byval arguments.
            bool hasByValArg() const { return !ByValArgs.empty(); }

            /// regSize - Size (in number of bits) of integer registers.
            unsigned regSize() const { return 4; }

            /// numIntArgRegs - Number of integer registers available for calls.
            unsigned numIntArgRegs() const;

            /// reservedArgArea - The size of the area the caller reserves for
            /// register arguments. This is 16 bytes (4 longs)
            unsigned reservedArgArea() const;

            /// Return pointer to array of integer argument registers.
            const ArrayRef<MCPhysReg> intArgRegs() const;

            typedef SmallVectorImpl<ByValArgInfo>::const_iterator byval_iterator;
            byval_iterator byval_begin() const { return ByValArgs.begin(); }
            byval_iterator byval_end() const { return ByValArgs.end(); }

            private:
                void handleByValArg(unsigned ValNo, MVT ValVT, MVT LocVT, CCValAssign::LocInfo LocInfo, ISD::ArgFlagsTy ArgFlags);

            /// useRegsForByval - Returns true if the calling convention allows the
            /// use of registers to pass byval arguments.
            bool useRegsForByval() const { return CallConv != CallingConv::Fast; }

            /// Return the function that analyzes fixed argument list functions.
            llvm::CCAssignFn *fixedArgFn() const;

            /// Return the function that analyzes variable argument list functions.
            llvm::CCAssignFn *varArgFn() const;

            void allocateRegs(ByValArgInfo &ByVal, unsigned ByValSize, unsigned Align);

            /// Return the type of the register which is used to pass an argument or
            /// return a value. This function returns f64 if the argument is an i64
            /// value which has been generated as a result of softening an f128 value.
            /// Otherwise, it just returns VT.
            MVT getRegVT(MVT VT, const Type *OrigTy, const SDNode *CallNode, bool IsSoftFloat) const;

            template<typename Ty> void analyzeReturn(const SmallVectorImpl<Ty> &RetVals, bool IsSoftFloat,
                                                        const SDNode *CallNode, const Type *RetTy) const;

            CCState &CCInfo;
            CallingConv::ID CallConv;
            SmallVector<ByValArgInfo, 2> ByValArgs;
        };

        P2CC::SpecialCallingConvType getSpecialCallingConv(SDValue Callee) const;

        SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                                CallingConv::ID CallConv, bool isVarArg,
                                const SmallVectorImpl<ISD::InputArg> &Ins,
                                const SDLoc &dl, SelectionDAG &DAG,
                                SmallVectorImpl<SDValue> &InVals,
                                const SDNode *CallNode, const Type *RetTy) const;

        /// isEligibleForTailCallOptimization - Check whether the call is eligible
        /// for tail call optimization.
        bool isEligibleForTailCallOptimization(const P2CC &P2CCInfo,
                                                unsigned NextStackOffset,
                                                const P2FunctionInfo& FI) const;

        /// passByValArg - Pass a byval argument in registers or on stack.
        void passByValArg(SDValue Chain, const SDLoc &DL,
                            std::deque< std::pair<unsigned, SDValue> > &RegsToPass,
                            SmallVectorImpl<SDValue> &MemOpChains, SDValue StackPtr,
                            MachineFrameInfo *MFI, SelectionDAG &DAG, SDValue Arg,
                            const P2CC &CC, const ByValArgInfo &ByVal,
                            const ISD::ArgFlagsTy &Flags) const;

        /// copyByValArg - Copy argument registers which were used to pass a byval
        /// argument to the stack. Create a stack frame object for the byval
        /// argument.
        void copyByValRegs(SDValue Chain, const SDLoc &DL,
                           std::vector<SDValue> &OutChains, SelectionDAG &DAG,
                           const ISD::ArgFlagsTy &Flags,
                           SmallVectorImpl<SDValue> &InVals,
                           const Argument *FuncArg,
                           const P2CC &CC, const ByValArgInfo &ByVal) const;

        SDValue LowerFormalArguments(SDValue Chain,
                           CallingConv::ID CallConv, bool IsVarArg,
                           const SmallVectorImpl<ISD::InputArg> &Ins,
                           const SDLoc &dl, SelectionDAG &DAG,
                           SmallVectorImpl<SDValue> &InVals) const override;

        SDValue passArgOnStack(SDValue StackPtr, unsigned Offset, SDValue Chain,
                           SDValue Arg, const SDLoc &DL, bool IsTailCall,
                           SelectionDAG &DAG) const;

        SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                          SmallVectorImpl<SDValue> &InVals) const override;


        // Lower Operand specifics
        SDValue lowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;

        SDValue LowerReturn(SDValue Chain,
                            CallingConv::ID CallConv, bool IsVarArg,
                            const SmallVectorImpl<ISD::OutputArg> &Outs,
                            const SmallVectorImpl<SDValue> &OutVals,
                            const SDLoc &dl, SelectionDAG &DAG) const override;

    };
}

#endif // P2ISELLOWERING_H
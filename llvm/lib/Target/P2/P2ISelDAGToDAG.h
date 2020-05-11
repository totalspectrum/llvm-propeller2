//===---- P2ISelDAGToDAG.h - A Dag to Dag Inst Selector for P2 --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the P2 target.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_P2_P2ISELDAGTODAG_H
#define LLVM_LIB_TARGET_P2_P2ISELDAGTODAG_H

#include "P2.h"
#include "P2TargetMachine.h"

#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// P2DAGToDAGISel - P2 specific code to select P2 machine
// instructions for SelectionDAG operations.
//===----------------------------------------------------------------------===//
namespace llvm {

    class StringRef;

    class P2DAGToDAGISel : public SelectionDAGISel {

    // Include the pieces autogenerated from the target description.
    #include "P2GenDAGISel.inc"

        /// getTargetMachine - Return a reference to the TargetMachine, casted
        /// to the target-specific type.
        const P2TargetMachine &getTargetMachine() {
            return static_cast<const P2TargetMachine &>(TM);
        }

        void Select(SDNode *N) override;
        //bool SelectAddr(SDNode *Parent, SDValue N, SDValue &Base, SDValue &Offset);

        // getImm - Return a target constant with the specified value.
        inline SDValue getImm(const SDNode *Node, unsigned Imm) {
            return CurDAG->getTargetConstant(Imm, SDLoc(Node), Node->getValueType(0));
        }

        //virtual void processFunctionAfterISel(MachineFunction &MF) = 0;

    public:
        explicit P2DAGToDAGISel(P2TargetMachine &TM, CodeGenOpt::Level OL)
            : SelectionDAGISel(TM, OL) {}

        // Pass Name
        StringRef getPassName() const override {
            return "P2 DAG->DAG Pattern Instruction Selection";
        }

        bool runOnMachineFunction(MachineFunction &MF) override;


    };

    FunctionPass *createP2ISelDag(P2TargetMachine &TM, CodeGenOpt::Level OptLevel);

}

#endif
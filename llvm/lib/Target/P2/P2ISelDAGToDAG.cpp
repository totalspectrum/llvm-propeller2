//===-- P2ISelDAGToDAG.cpp - A Dag to Dag Inst Selector for P2 --------===//
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

#include "P2ISelDAGToDAG.h"
#include "P2.h"

#include "P2MachineFunctionInfo.h"
#include "P2RegisterInfo.h"
#include "P2TargetMachine.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;

#define DEBUG_TYPE "p2-isel"

bool P2DAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
    return SelectionDAGISel::runOnMachineFunction(MF);
}

void P2DAGToDAGISel::selectMultiplication(SDNode *N) {
    SDLoc DL(N);
    MVT vt = N->getSimpleValueType(0);

    assert(vt == MVT::i32 && "unexpected value type");
    bool isSigned = N->getOpcode() == ISD::SMUL_LOHI;
    unsigned op = isSigned ? P2::QMULrr : P2::QMULrr; // FIXME: replace with signed multiplication node

    SDValue lhs = N->getOperand(0);
    SDValue rhs = N->getOperand(1);
    SDNode *mul = CurDAG->getMachineNode(op, DL, MVT::Glue, lhs, rhs);
    SDValue in_chain = CurDAG->getEntryNode();
    SDValue in_glue = SDValue(mul, 0);

    //LLVM_DEBUG(mul->dump());

    // Copy the low half of the result, if it is needed.
    if (N->hasAnyUseOfValue(0)) {
        SDValue res = CurDAG->getCopyFromReg(in_chain, DL, P2::QX, vt, in_glue);
        ReplaceUses(SDValue(N, 0), res);

        in_chain = res.getValue(1);
        in_glue = res.getValue(2);

        //LLVM_DEBUG(res.dump());
    }

    // Copy the high half of the result, if it is needed.
    if (N->hasAnyUseOfValue(1)) {
        SDValue res = CurDAG->getCopyFromReg(in_chain, DL, P2::QY, vt, in_glue);
        ReplaceUses(SDValue(N, 1), res);

        in_chain = res.getValue(1);
        in_glue = res.getValue(2);

        //LLVM_DEBUG(res.dump());
    }

    CurDAG->RemoveDeadNode(N);
}

void P2DAGToDAGISel::Select(SDNode *N) {
    //unsigned Opcode = N->getOpcode();

    // Dump information about the Node being selected
    LLVM_DEBUG(errs() << "<-------->\n");
    LLVM_DEBUG(errs() << "Selecting: "; N->dump(CurDAG); errs() << "\n");

    // this is already a machine op
    if (N->isMachineOpcode()) {
        LLVM_DEBUG(errs() << "== "; N->dump(CurDAG); errs() << "\n");
        N->setNodeId(-1);
        return;
    }

    /*
     * Instruction Selection not handled by the auto-generated
     * tablegen selection should be handled here.
     */
    //EVT NodeTy = Node->getValueType(0);
    unsigned Opcode = N->getOpcode();
    auto DL = CurDAG->getDataLayout();

    switch(Opcode) {
        default: break;

        case ISD::FrameIndex: {
            LLVM_DEBUG(errs() << "frame index node is being selected\n");
            FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(N);
            SDValue TFI = CurDAG->getTargetFrameIndex(FIN->getIndex(), getTargetLowering()->getPointerTy(DL));

            CurDAG->SelectNodeTo(N, P2::FRMIDX, getTargetLowering()->getPointerTy(DL), TFI);
            return;
        }

        // case ISD::SUBE: {
        //     SDValue InFlag = Node->getOperand(2);
        //     selectAddESubE(Cpu0::SUBu, InFlag, InFlag.getOperand(0), DL, Node);
        //     return true;
        // }

        // case ISD::ADDE: {
        //     SDValue InFlag = Node->getOperand(2);
        //     selectAddESubE(Cpu0::ADDu, InFlag, InFlag.getValue(0), DL, Node);
        //     return true;
        // }

        /// Mul with two results
        case ISD::SMUL_LOHI:
            llvm_unreachable("DAGToDAG: no signed multiplication implementation yet");
        case ISD::UMUL_LOHI: {
            selectMultiplication(N);
            return;
        }
    }

    // Select the default instruction
    SelectCode(N);
    LLVM_DEBUG(errs() << "Done selecting, chose "; N->dump());
}

FunctionPass *llvm::createP2ISelDag(P2TargetMachine &TM, CodeGenOpt::Level OptLevel) {
    return new P2DAGToDAGISel(TM, OptLevel);
}
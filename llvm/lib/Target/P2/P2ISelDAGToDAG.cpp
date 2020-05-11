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

#define DEBUG_TYPE "P2-isel"

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// P2DAGToDAGISel - P2 specific code to select P2 machine
// instructions for SelectionDAG operations.
//===----------------------------------------------------------------------===//

bool P2DAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
    return SelectionDAGISel::runOnMachineFunction(MF);
}

void P2DAGToDAGISel::Select(SDNode *N) {
    unsigned Opcode = N->getOpcode();

    // Dump information about the Node being selected
    LLVM_DEBUG(errs() << "Selecting: "; N->dump(CurDAG); errs() << "\n");

    // If we have a custom node, we already have selected!
    if (N->isMachineOpcode()) {
        LLVM_DEBUG(errs() << "== "; N->dump(CurDAG); errs() << "\n");
        N->setNodeId(-1);
        return;
    }

    if(FrameIndexSDNode *FIN = dyn_cast<FrameIndexSDNode>(N)) {
        // auto Addr = CurDAG->getTargetFrameIndex(FIN->getIndex(), TLI->getPointerTy(CurDAG->getDataLayout()));
        // SDLoc dl(Addr);
        // //LLVM_DEBUG(errs() << "have a frame index node, changing to move instruction, "; Addr.dump(); errs() << "\n");
        // SDNode *ADD = CurDAG->getMachineNode(P2::ADDri, dl, MVT::i32, Addr);
        // CurDAG->ReplaceAllUsesWith(Node, MOV);

        auto DL = CurDAG->getDataLayout();
        int FI = cast<FrameIndexSDNode>(N)->getIndex();
        SDValue TFI = CurDAG->getTargetFrameIndex(FI, getTargetLowering()->getPointerTy(DL));

        CurDAG->SelectNodeTo(N, P2::FRMIDX, getTargetLowering()->getPointerTy(DL), TFI,
                       CurDAG->getTargetConstant(0, SDLoc(N), MVT::i16));

        return;
    }

    switch(Opcode) {
        default:
            LLVM_DEBUG(errs() << "opcode " << Opcode << "\n");
        break;

    }

    LLVM_DEBUG(errs() << "running default select\n");
    // Select the default instruction
    SelectCode(N);
    LLVM_DEBUG(errs() << "Done selecting\n");
}

FunctionPass *llvm::createP2ISelDag(P2TargetMachine &TM, CodeGenOpt::Level OptLevel) {
    return new P2DAGToDAGISel(TM, OptLevel);
}
//===- P2ExpandPseudosPass - P2 expand pseudo loads -----------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass expands stores with large offsets into an appropriate sequence.
//===----------------------------------------------------------------------===//

#include "P2.h"
#include "P2InstrInfo.h"
#include "P2RegisterInfo.h"
#include "P2Subtarget.h"
#include "P2TargetMachine.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

#define DEBUG_TYPE "p2-expand-pseudos"

namespace {

    class P2ExpandPseudos : public MachineFunctionPass {
    public:
        static char ID;
        P2ExpandPseudos(P2TargetMachine &tm) : MachineFunctionPass(ID), TM(tm) {}

        bool runOnMachineFunction(MachineFunction &Fn) override;

        StringRef getPassName() const override { return "P2 Expand Pseudos"; }

    private:
        const P2InstrInfo *TII;
        const P2TargetMachine &TM;

        void expand_QSREM(MachineFunction &MF, MachineBasicBlock::iterator SII);
        void expand_QUREM(MachineFunction &MF, MachineBasicBlock::iterator SII);
        void expand_MOVri32(MachineFunction &MF, MachineBasicBlock::iterator SII);
    };

    char P2ExpandPseudos::ID = 0;

} // end anonymous namespace

void P2ExpandPseudos::expand_QSREM(MachineFunction &MF, MachineBasicBlock::iterator SII) {
    MachineInstr &SI = *SII;

    LLVM_DEBUG(errs()<<"== lower pseudo signed remainder\n");
    SI.dump();

    BuildMI(*SI.getParent(), SI, SI.getDebugLoc(), TII->get(P2::QDIV))
            .addReg(P2::QX, RegState::Define)
            .addReg(P2::QY, RegState::Define)
            .addReg(SI.getOperand(1).getReg())
            .addReg(SI.getOperand(2).getReg());
    BuildMI(*SI.getParent(), SI, SI.getDebugLoc(), TII->get(P2::GETQY), SI.getOperand(0).getReg())
            .addReg(P2::QY);

    SI.eraseFromParent();
}

void P2ExpandPseudos::expand_QUREM(MachineFunction &MF, MachineBasicBlock::iterator SII) {
    MachineInstr &SI = *SII;

    LLVM_DEBUG(errs()<<"== lower pseudo unsigned remainder\n");
    SI.dump();

    BuildMI(*SI.getParent(), SI, SI.getDebugLoc(), TII->get(P2::QDIV))
            .addReg(P2::QX, RegState::Define)
            .addReg(P2::QY, RegState::Define)
            .addReg(SI.getOperand(1).getReg())
            .addReg(SI.getOperand(2).getReg());
    BuildMI(*SI.getParent(), SI, SI.getDebugLoc(), TII->get(P2::GETQY), SI.getOperand(0).getReg())
            .addReg(P2::QY);

    SI.eraseFromParent();
}

// eventually we should have an operand in InstrInfo that will automatically convert any immediate to
// aug the top 23 bits, then mov the lower 9. TBD how to do that. we will still need something like this
// for global symbols where we don't know the value until linking, so we should always have an AUG instruction
void P2ExpandPseudos::expand_MOVri32(MachineFunction &MF, MachineBasicBlock::iterator SII) {
    MachineInstr &SI = *SII;

    LLVM_DEBUG(errs()<<"== lower pseudo mov i32imm\n");
    LLVM_DEBUG(errs() << "operand type = " << (unsigned)SI.getOperand(1).getType() << "\n");

    if (SI.getOperand(1).isGlobal()) {
        BuildMI(*SI.getParent(), SI, SI.getDebugLoc(), TII->get(P2::AUGS))
            .addImm(0); // we will encode the correct value into this later. if just printing assembly,
                        // the final optimization pass should remove this instruction
                        // as a result, the exact printing of this instruction won't be correct
        BuildMI(*SI.getParent(), SI, SI.getDebugLoc(), TII->get(P2::MOVri), SI.getOperand(0).getReg())
            .addGlobalAddress(SI.getOperand(1).getGlobal());

    } else {
        uint32_t imm = SI.getOperand(1).getImm();

        // expand into an AUGS for the top 23 bits of the immediate and MOVri for the lower 9 bits
        BuildMI(*SI.getParent(), SI, SI.getDebugLoc(), TII->get(P2::AUGS))
            .addImm(imm>>23);

        BuildMI(*SI.getParent(), SI, SI.getDebugLoc(), TII->get(P2::MOVri), SI.getOperand(0).getReg())
            .addImm(imm&0x1ff);
    }

    SI.eraseFromParent();
}

bool P2ExpandPseudos::runOnMachineFunction(MachineFunction &MF) {
    TII = TM.getInstrInfo();

    for (auto &MBB : MF) {
        MachineBasicBlock::iterator MBBI = MBB.begin(), E = MBB.end();
        while (MBBI != E) {
            MachineBasicBlock::iterator NMBBI = std::next(MBBI);
            switch (MBBI->getOpcode()) {
                case P2::QSREM:
                    expand_QSREM(MF, MBBI);
                    break;
                case P2::QUREM:
                    expand_QUREM(MF, MBBI);
                    break;
                case P2::MOVri32:
                    expand_MOVri32(MF, MBBI);
                    break;
                default:
                    break;
            }

            MBBI = NMBBI;
        }
    }

    LLVM_DEBUG(errs()<<"done with pseudo expansion\n");

    return true;
}

FunctionPass *llvm::createP2ExpandPseudosPass(P2TargetMachine &tm) {
    return new P2ExpandPseudos(tm);
}

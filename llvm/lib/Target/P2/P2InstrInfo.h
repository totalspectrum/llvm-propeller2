//===-- P2InstrInfo.h - P2 Instruction Information ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the P2 implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_P2_P2INSTRINFO_H
#define LLVM_LIB_TARGET_P2_P2INSTRINFO_H

#include "P2.h"
#include "P2RegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "P2GenInstrInfo.inc"

namespace llvm {

    class P2InstrInfo : public P2GenInstrInfo {
        const P2RegisterInfo RI;
        virtual void anchor();

    public:
        explicit P2InstrInfo();

        /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
        /// such, whenever a client has an instance of instruction info, it should
        /// always be able to get register info as well (through this method).
        const P2RegisterInfo &getRegisterInfo() const { return RI; };

        void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc) const override;

        void storeRegToStackSlot(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI, Register SrcReg,
                                bool isKill, int FrameIndex,
                                const TargetRegisterClass *RC,
                                const TargetRegisterInfo *TRI) const override;

        void loadRegFromStackSlot(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MI, Register DestReg,
                                    int FrameIndex, const TargetRegisterClass *RC,
                                    const TargetRegisterInfo *TRI) const override;

        void adjustStackPtr(unsigned SP, int64_t Amount, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator I) const;

        bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB, MachineBasicBlock *&FBB,
                            SmallVectorImpl<MachineOperand> &Cond, bool AllowModify) const override;

        unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                                const DebugLoc &DL, int *BytesAdded = nullptr) const override;

        unsigned removeBranch(MachineBasicBlock &MBB, int *BytesRemoved = nullptr) const override;
    };
}

#endif
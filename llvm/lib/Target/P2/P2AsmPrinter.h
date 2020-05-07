//===-- P2AsmPrinter.h - P2 LLVM Assembly Printer ----------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// P2 Assembly printer class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_P2_P2ASMPRINTER_H
#define LLVM_LIB_TARGET_P2_P2ASMPRINTER_H

#include "P2MachineFunctionInfo.h"
#include "P2MCInstLower.h"
#include "P2Subtarget.h"
#include "P2TargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
    class MCStreamer;
    class MachineInstr;
    class MachineBasicBlock;
    class Module;
    class raw_ostream;
    class StringRef;

    class LLVM_LIBRARY_VISIBILITY P2AsmPrinter : public AsmPrinter {

    private:

        // lowerOperand - Convert a MachineOperand into the equivalent MCOperand.
        bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp);

    public:

        const P2Subtarget *Subtarget;
        const P2FunctionInfo *P2FI;
        P2MCInstLower MCInstLowering;

        explicit P2AsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer)
                            : AsmPrinter(TM, std::move(Streamer)),
                            MCInstLowering(*this) {
            Subtarget = static_cast<P2TargetMachine &>(TM).getSubtargetImpl();
        }

        virtual StringRef getPassName() const override {
            return "P2 Assembly Printer";
        }

        virtual bool runOnMachineFunction(MachineFunction &MF) override;

        //- EmitInstruction() must exists or will have run time error.
        void emitInstruction(const MachineInstr *MI) override;
        //void printSavedRegsBitmask(raw_ostream &O);
        //void printHex32(unsigned int Value, raw_ostream &O);

        // void EmitFunctionEntryLabel() override;
        // void EmitFunctionBodyStart() override;
        // void EmitFunctionBodyEnd() override;
        // void EmitStartOfAsmFile(Module &M) override;
    };
}

#endif
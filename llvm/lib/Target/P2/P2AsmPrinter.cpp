//===-- P2AsmPrinter.cpp - P2 LLVM Assembly Printer -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format P2 assembly language.
//
//===----------------------------------------------------------------------===//

#include "P2AsmPrinter.h"

#include "MCTargetDesc/P2InstPrinter.h"
#include "P2.h"
#include "P2InstrInfo.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Twine.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

#define DEBUG_TYPE "P2-asm-printer"

bool P2AsmPrinter::runOnMachineFunction(MachineFunction &MF) {
    P2FI = MF.getInfo<P2FunctionInfo>();
    AsmPrinter::runOnMachineFunction(MF);
    return true;
}

void P2AsmPrinter::emitInstruction(const MachineInstr *MI) {
    //@EmitInstruction body {
    if (MI->isDebugValue()) {
        SmallString<128> Str;
        raw_svector_ostream OS(Str);

        //PrintDebugValueComment(MI, OS);
        return;
    }

    //@print out instruction:
    //  Print out both ordinary instruction and boudle instruction
    MachineBasicBlock::const_instr_iterator I = MI->getIterator();
    MachineBasicBlock::const_instr_iterator E = MI->getParent()->instr_end();

    do {
        if (I->isPseudo())
            llvm_unreachable("Pseudo opcode found in EmitInstruction()");

        MCInst TmpInst0;
        MCInstLowering.Lower(&*I, TmpInst0);
        OutStreamer->emitInstruction(TmpInst0, getSubtargetInfo());
    } while ((++I != E) && I->isInsideBundle()); // Delay slot check
}

// void P2AsmPrinter::PrintDebugValueComment(const MachineInstr *MI,
//                                            raw_ostream &OS) {
//     // TODO: implement
//      OS << "PrintDebugValueComment()";
// }

// Force static initialization.
extern "C" void LLVMInitializeP2AsmPrinter() {
    RegisterAsmPrinter<P2AsmPrinter> X(TheP2Target);
}
//===- P2InstPrinter.cpp - P2 MCInst to assembly syntax -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints an P2 MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "P2InstPrinter.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#include "P2GenAsmWriter.inc"


void P2InstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
    OS << StringRef(getRegisterName(RegNo)).lower();
}

void P2InstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &O) {
    printInstruction(MI, Address, O);
    printAnnotation(O, Annot);
}

static void printExpr(const MCExpr *Expr, const MCAsmInfo *MAI, raw_ostream &OS) {
    int Offset = 0;
    const MCSymbolRefExpr *SRE;

    if (const auto *CE = dyn_cast<MCConstantExpr>(Expr)) {
        OS << "0x";
        OS.write_hex(CE->getValue());
        return;
    }

    if (const auto *BE = dyn_cast<MCBinaryExpr>(Expr)) {
        SRE = dyn_cast<MCSymbolRefExpr>(BE->getLHS());
        const auto *CE = dyn_cast<MCConstantExpr>(BE->getRHS());
        assert(SRE && CE && "Binary expression must be sym+const.");
        Offset = CE->getValue();
    } else {
        SRE = dyn_cast<MCSymbolRefExpr>(Expr);
        assert(SRE && "Unexpected MCExpr type.");
    }
    assert(SRE->getKind() == MCSymbolRefExpr::VK_None);

    // Symbols are prefixed with '@'
    OS << '@';
    SRE->getSymbol().print(OS, MAI);

    if (Offset) {
        if (Offset > 0)
        OS << '+';
        OS << Offset;
    }
}

void P2InstPrinter::printOperand(const MCInst *MI, unsigned OpNum,
                                  raw_ostream &O) {
    const MCOperand &Op = MI->getOperand(OpNum);
    if (Op.isReg()) {
        printRegName(O, Op.getReg());
        return;
    }

    if (Op.isImm()) {
        O << Op.getImm();
        return;
    }

    assert(Op.isExpr() && "unknown operand kind in printOperand");
    printExpr(Op.getExpr(), &MAI, O);
}

// void P2InstPrinter::printMemOperandRI(const MCInst *MI, unsigned OpNum,
//                                        raw_ostream &O) {
//   const MCOperand &base = MI->getOperand(OpNum);
//   const MCOperand &offset = MI->getOperand(OpNum + 1);
//   assert(base.isReg() && "Base should be register.");
//   assert(offset.isImm() && "Offset should be immediate.");
//   printRegName(O, base.getReg());
//   O << "," << offset.getImm();
// }

// void P2InstPrinter::printPredicateOperand(const MCInst *MI, unsigned OpNum,
//                                            raw_ostream &O) {

//   const MCOperand &Op = MI->getOperand(OpNum);
//   assert(Op.isImm() && "Predicate operand is immediate.");
//   O << P2CondCodeToString((P2CC::CondCode)Op.getImm());
// }

// void P2InstPrinter::printBRCCPredicateOperand(const MCInst *MI, unsigned OpNum,
//                                                raw_ostream &O) {
//   const MCOperand &Op = MI->getOperand(OpNum);
//   assert(Op.isImm() && "Predicate operand is immediate.");
//   O << P2BRCondCodeToString((P2CC::BRCondCode)Op.getImm());
// }

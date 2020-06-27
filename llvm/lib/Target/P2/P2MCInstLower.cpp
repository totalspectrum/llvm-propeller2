//===-- P2MCInstLower.cpp - Convert P2 MachineInstr to MCInst ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower P2 MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "P2MCInstLower.h"

#include "P2InstrInfo.h"
#include "P2AsmPrinter.h"

#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "p2-mc-ilower"

P2MCInstLower::P2MCInstLower(P2AsmPrinter &asmprinter) : AsmPrinter(asmprinter) {}

void P2MCInstLower::Initialize(MCContext* C) {
    Ctx = C;
}

MCOperand P2MCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                              MachineOperandType MOTy,
                                              unsigned Offset) const {
    const MCSymbol *Symbol;

    switch (MOTy) {
        case MachineOperand::MO_GlobalAddress:
            Symbol = AsmPrinter.getSymbol(MO.getGlobal());
            Offset += MO.getOffset();
        break;

        case MachineOperand::MO_MachineBasicBlock:
            Symbol = MO.getMBB()->getSymbol();
        break;

        case MachineOperand::MO_BlockAddress:
            Symbol = AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress());
            Offset += MO.getOffset();
        break;

        case MachineOperand::MO_JumpTableIndex:
            Symbol = AsmPrinter.GetJTISymbol(MO.getIndex());
        break;

        case MachineOperand::MO_ExternalSymbol:
            Symbol = AsmPrinter.GetExternalSymbolSymbol(MO.getSymbolName());
        break;

        default:
            llvm_unreachable("<unknown operand type>");
    }

    const MCExpr *Expr = MCSymbolRefExpr::create(Symbol, *Ctx);

    if (Offset) {
        // Assume offset is never negative.
        assert(Offset > 0);
        Expr = MCBinaryExpr::createAdd(Expr, MCConstantExpr::create(Offset, *Ctx), *Ctx);
    }

    // if (TargetKind != Cpu0MCExpr::CEK_None)
    //     Expr = Cpu0MCExpr::create(TargetKind, Expr, *Ctx);

    return MCOperand::createExpr(Expr);
}

MCOperand P2MCInstLower::lowerOperand(const MachineOperand& MO, unsigned offset) const {
    MachineOperandType MOTy = MO.getType();

    switch (MOTy) {

        default:
            LLVM_DEBUG(errs() << "Operand type: " << (int)MOTy << "\n");
            llvm_unreachable("MCInstrLower: unknown operand type");
        case MachineOperand::MO_GlobalAddress:
        case MachineOperand::MO_JumpTableIndex:
        case MachineOperand::MO_BlockAddress:
        case MachineOperand::MO_MachineBasicBlock:
        case MachineOperand::MO_ExternalSymbol:
            return LowerSymbolOperand(MO, MOTy, offset);

        case MachineOperand::MO_Register:
            // Ignore all implicit register operands.
            if (MO.isImplicit()) break;
            return MCOperand::createReg(MO.getReg());

        case MachineOperand::MO_Immediate:
            return MCOperand::createImm(MO.getImm() + offset);

        case MachineOperand::MO_RegisterMask:
            break;
    }

    return MCOperand();
}

void P2MCInstLower::lowerInstruction(const MachineInstr &MI, MCInst &OutMI) const {
    OutMI.setOpcode(MI.getOpcode());
    OutMI.setFlags(MI.getDesc().TSFlags);

    for (unsigned i = 0, e = MI.getNumOperands(); i != e; ++i) {
        const MachineOperand &MO = MI.getOperand(i);
        MCOperand MCOp = lowerOperand(MO);

        if (MCOp.isValid())
            OutMI.addOperand(MCOp);
    }
}
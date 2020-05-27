//===-- P2MCCodeEmitter.cpp - Convert P2 Code to Machine Code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the P2MCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//

#include "P2MCCodeEmitter.h"

#include "MCTargetDesc/P2BaseInfo.h"
//#include "MCTargetDesc/P2FixupKinds.h"
//#include "MCTargetDesc/P2MCExpr.h"
#include "MCTargetDesc/P2MCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "mccodeemitter"

#define GET_INSTRMAP_INFO
#include "P2GenInstrInfo.inc"
#undef GET_INSTRMAP_INFO

MCCodeEmitter *llvm::createP2MCCodeEmitter(const MCInstrInfo &MCII, const MCRegisterInfo &MRI, MCContext &Ctx) {
    return new P2MCCodeEmitter(MCII, Ctx);
}

void P2MCCodeEmitter::emitByte(unsigned char C, raw_ostream &OS) const {
    OS << (char)C;
}

void P2MCCodeEmitter::emitInstruction(uint64_t Val, unsigned Size, raw_ostream &OS) const {
    // Output the instruction encoding in little endian byte order.
    for (unsigned i = 0; i < Size; ++i) {
        emitByte((Val >> i*8) & 0xff, OS);
    }
}

/// encodeInstruction - Emit the instruction.
/// Size the instruction (currently only 4 bytes)
void P2MCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {
    uint32_t bin = getBinaryCodeForInstr(MI, Fixups, STI);

    LLVM_DEBUG(MI.dump());

    // Check for unimplemented opcodes.
    unsigned op_code = MI.getOpcode();

    const MCInstrDesc &Desc = MCII.get(op_code);
    uint64_t TSFlags = Desc.TSFlags;
    uint64_t inst_type = (TSFlags >> 4) & 31; // 5 bits
    // inst type might not be necessary yet.

    LLVM_DEBUG(errs() << "emitting instruction binary: ");
    for (int i = 0; i < 32; i++) {
        LLVM_DEBUG(errs() << ((bin >> (31-i))&1));
    }

    LLVM_DEBUG(errs() << "\n");

    // Pseudo instructions don't get encoded and shouldn't be here
    // in the first place!
    // if ((TSFlags & P2::FormMask) == P2::Pseudo)
    //     llvm_unreachable("Pseudo opcode found in encodeInstruction()");

    // For now all instructions are 4 bytes
    int Size = 4; // FIXME: Have Desc.getSize() return the correct value!

    emitInstruction(bin, 4, OS);
}

unsigned P2MCCodeEmitter::getBranchTargetOpValue(const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {
    llvm_unreachable("getBranchTargetOpValue not implemented");
    return 0;
}

unsigned P2MCCodeEmitter::getJumpTargetOpValue(const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {
    llvm_unreachable("getJumpTargetOpValue not implemented");
    return 0;
}

unsigned P2MCCodeEmitter::encodeCallTarget(const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups, const MCSubtargetInfo &STI) const {
    llvm_unreachable("encodeCallTarget not implemented");
    return 0;
}

unsigned P2MCCodeEmitter::getExprOpValue(const MCExpr *Expr,SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {

    MCExpr::ExprKind Kind = Expr->getKind();

    if (Kind == MCExpr::Constant) {
        return cast<MCConstantExpr>(Expr)->getValue();
    }

    if (Kind == MCExpr::Binary) {
        unsigned Res = getExprOpValue(cast<MCBinaryExpr>(Expr)->getLHS(), Fixups, STI);
        Res += getExprOpValue(cast<MCBinaryExpr>(Expr)->getRHS(), Fixups, STI);
        return Res;
    }

    if (Kind == MCExpr::Target) {
        llvm_unreachable("no implementation for target expressions!");
        //const P2MCExpr *P2Expr = cast<P2MCExpr>(Expr);

        // P2::Fixups FixupKind = P2::Fixups(0);
        // switch (P2Expr->getKind()) {
        // default: llvm_unreachable("Unsupported fixup kind for target expression!");
        // } // switch
        // Fixups.push_back(MCFixup::create(0, Expr, MCFixupKind(FixupKind)));
        return 0;
    }
    return 0;
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned P2MCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
    if (MO.isReg()) {
        unsigned Reg = MO.getReg();
        unsigned RegNo = Ctx.getRegisterInfo()->getEncodingValue(Reg);
        LLVM_DEBUG(errs() << "-- register number is " << RegNo << " for reg " << Reg << "\n");
        return RegNo;
    } else if (MO.isImm()) {
        return static_cast<unsigned>(MO.getImm());
    }
    // MO must be an Expr.
    assert(MO.isExpr());
    return getExprOpValue(MO.getExpr(), Fixups, STI);
}

/// getMemEncoding - Return binary encoding of memory related operand.
/// If the offset operand requires relocation, record the relocation.
unsigned P2MCCodeEmitter::getMemEncoding(const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
    // Base register is encoded in bits 20-16, offset is encoded in bits 15-0.
    // TODO
    assert(MI.getOperand(OpNo).isReg());
    unsigned RegBits = getMachineOpValue(MI, MI.getOperand(OpNo), Fixups, STI) << 16;
    unsigned OffBits = getMachineOpValue(MI, MI.getOperand(OpNo+1), Fixups, STI);

    return (OffBits & 0xFFFF) | RegBits;
}

#include "P2GenMCCodeEmitter.inc"
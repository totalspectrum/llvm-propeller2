//===-- P2ELFObjectWriter.cpp - P2 ELF Writer -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===------------------------------------------------------------------===//

#include "MCTargetDesc/P2BaseInfo.h"
//#include "MCTargetDesc/P2FixupKinds.h"
#include "MCTargetDesc/P2MCTargetDesc.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include <list>

using namespace llvm;

namespace {
    class P2ELFObjectWriter : public MCELFObjectTargetWriter {
    public:
        P2ELFObjectWriter(uint8_t OSABI);

        ~P2ELFObjectWriter() override;

        unsigned getRelocType(MCContext &Ctx, const MCValue &Target, const MCFixup &Fixup, bool IsPCRel) const override;
        bool needsRelocateWithSymbol(const MCSymbol &Sym, unsigned Type) const override;
    };
}

P2ELFObjectWriter::P2ELFObjectWriter(uint8_t OSABI) : MCELFObjectTargetWriter(false, OSABI, ELF::EM_P2, /*HasRelocationAddend*/ false) {}

P2ELFObjectWriter::~P2ELFObjectWriter() {}

unsigned P2ELFObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target, const MCFixup &Fixup, bool IsPCRel) const {
    // determine the type of the relocation
    llvm_unreachable("getRelocType in P2ELFObjectWriter no implemented!");

    // unsigned Type = (unsigned)ELF::R_P2_NONE;
    // unsigned Kind = (unsigned)Fixup.getKind();

    // switch (Kind) {
    // default:
    //     llvm_unreachable("invalid fixup kind!");
    // case FK_Data_4:
    //     Type = ELF::R_P2_32;
    //     break;
    // case P2::fixup_P2_32:
    //     Type = ELF::R_P2_32;
    //     break;
    // case P2::fixup_P2_GPREL16:
    //     Type = ELF::R_P2_GPREL16;
    //     break;
    // case P2::fixup_P2_GOT:
    //     Type = ELF::R_P2_GOT16;
    //     break;
    // case P2::fixup_P2_HI16:
    //     Type = ELF::R_P2_HI16;
    //     break;
    // case P2::fixup_P2_LO16:
    //     Type = ELF::R_P2_LO16;
    //     break;
    // case P2::fixup_P2_GOT_HI16:
    //     Type = ELF::R_P2_GOT_HI16;
    //     break;
    // case P2::fixup_P2_GOT_LO16:
    //     Type = ELF::R_P2_GOT_LO16;
    //     break;
    // }

    // return Type;
}

bool P2ELFObjectWriter::needsRelocateWithSymbol(const MCSymbol &Sym, unsigned Type) const {
    // FIXME: This is extremelly conservative. This really needs to use a
    // whitelist with a clear explanation for why each realocation needs to
    // point to the symbol, not to the section.
    switch (Type) {
    default:
        return false;

    // case ELF::R_P2_GOT16:
    // // For P2 pic mode, I think it's OK to return true but I didn't confirm.
    // //  llvm_unreachable("Should have been handled already");
    //     return true;

    // // These relocations might be paired with another relocation. The pairing is
    // // done by the static linker by matching the symbol. Since we only see one
    // // relocation at a time, we have to force them to relocate with a symbol to
    // // avoid ending up with a pair where one points to a section and another
    // // points to a symbol.
    // case ELF::R_P2_HI16:
    // case ELF::R_P2_LO16:
    // // R_P2_32 should be a relocation record, I don't know why Mips set it to
    // // false.
    // case ELF::R_P2_32:
    //     return true;

    // case ELF::R_P2_GPREL16:
    //     return false;
    }
}

std::unique_ptr<MCObjectTargetWriter> llvm::createP2ELFObjectWriter(uint8_t OSABI) {
    return std::make_unique<P2ELFObjectWriter>(OSABI);
}
//===-- P2AsmBackend.cpp - P2 Asm Backend  ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the P2AsmBackend class.
//
//===----------------------------------------------------------------------===//
//

//#include "MCTargetDesc/P2FixupKinds.h"
#include "MCTargetDesc/P2AsmBackend.h"

#include "MCTargetDesc/P2MCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
static unsigned adjustFixupValue(const MCFixup &Fixup, uint64_t Value, MCContext *Ctx = nullptr) {

    unsigned Kind = Fixup.getKind();

    // Add/subtract and shift
    switch (Kind) {
        default:
            return 0;
    }

    return Value;
}
//@adjustFixupValue }

std::unique_ptr<MCObjectTargetWriter> P2AsmBackend::createObjectTargetWriter() const {
    return createP2ELFObjectWriter(MCELFObjectTargetWriter::getOSABI(OSType));
}

void P2AsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                        const MCValue &Target, MutableArrayRef<char> Data,
                        uint64_t Value, bool IsResolved,
                        const MCSubtargetInfo *STI) const {

    //MCFixupKind Kind = Fixup.getKind();
    Value = adjustFixupValue(Fixup, Value);

    if (!Value)
        return; // Doesn't change encoding.

    // // Where do we start in the object
    // unsigned Offset = Fixup.getOffset();
    // // Number of bytes we need to fixup
    // unsigned NumBytes = (getFixupKindInfo(Kind).TargetSize + 7) / 8;
    // // Used to point to big endian bytes
    // unsigned FullSize;

    // switch ((unsigned)Kind) {
    // default:
    //     FullSize = 4;
    //     break;
    // }

    // // Grab current value, if any, from bits.
    // uint64_t CurVal = 0;

    // for (unsigned i = 0; i != NumBytes; ++i) {
    //     unsigned Idx = IsLittle ? i : (FullSize - 1 - i);
    //     CurVal |= (uint64_t)((uint8_t)Data[Offset + Idx]) << (i*8);
    // }

    // uint64_t Mask = ((uint64_t)(-1) >>
    //                                     (64 - getFixupKindInfo(Kind).TargetSize));
    // CurVal |= Value & Mask;

    // // Write out the fixed up bytes back to the code/data bits.
    // for (unsigned i = 0; i != NumBytes; ++i) {
    //     unsigned Idx = IsLittle ? i : (FullSize - 1 - i);
    //     Data[Offset + Idx] = (uint8_t)((CurVal >> (i*8)) & 0xff);
    // }
}

const MCFixupKindInfo &P2AsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
    return MCAsmBackend::getFixupKindInfo(Kind);
}

bool P2AsmBackend::writeNopData(raw_ostream &OS, uint64_t Count) const {
    return true;
}

MCAsmBackend *llvm::createP2AsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const llvm::MCTargetOptions &TO) {
  return new P2AsmBackend(STI.getTargetTriple().getOS());
}

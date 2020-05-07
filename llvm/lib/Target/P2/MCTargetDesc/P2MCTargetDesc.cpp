//===-- P2MCTargetDesc.cpp - P2 Target Descriptions -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides P2 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "P2MCTargetDesc.h"

#include "P2InstPrinter.h"
#include "P2MCAsmInfo.h"

#include "llvm/MC/MachineLocation.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"


using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "P2GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "P2GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "P2GenRegisterInfo.inc"

static MCInstrInfo *createP2MCInstrInfo() {
    MCInstrInfo *X = new MCInstrInfo();
    InitP2MCInstrInfo(X); // defined in P2GenInstrInfo.inc
    return X;
}

static MCRegisterInfo *createP2MCRegisterInfo(const Triple &TT) {
    MCRegisterInfo *X = new MCRegisterInfo();
    InitP2MCRegisterInfo(X, 0); // defined in P2GenRegisterInfo.inc
    return X;
}

static MCSubtargetInfo *createP2MCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
    return createP2MCSubtargetInfoImpl(TT, CPU, FS);
    // createP2MCSubtargetInfoImpl defined in P2GenSubtargetInfo.inc
}

static MCInstPrinter *createP2MCInstPrinter(const Triple &T,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI) {
    return new P2InstPrinter(MAI, MII, MRI);
}

// namespace {
//     class P2MCInstrAnalysis : public MCInstrAnalysis {
//     public:
//         P2MCInstrAnalysis(const MCInstrInfo *Info) : MCInstrAnalysis(Info) {}
//     };
// }

// static MCInstrAnalysis *createP2MCInstrAnalysis(const MCInstrInfo *Info) {
//     return new P2MCInstrAnalysis(Info);
// }

//@2 {
extern "C" void LLVMInitializeP2TargetMC() {
    // Register the MC asm info.
    RegisterMCAsmInfo<P2MCAsmInfo> X(TheP2Target);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(TheP2Target, createP2MCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(TheP2Target, createP2MCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(TheP2Target, createP2MCSubtargetInfo);
    // Register the MC instruction analyzer.
    // TargetRegistry::RegisterMCInstrAnalysis(TheP2Target, createP2MCInstrAnalysis);
    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(TheP2Target, createP2MCInstPrinter);
}
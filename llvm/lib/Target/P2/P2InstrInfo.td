//===- P2InstrInfo.td - Target Description for P2 Target -*- tablegen -*-=//
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

/*
 * the encodings are currently random, should be updated to match the instruction set
 */

include "P2InstrFormats.td"

def P2retflag : SDNode<"P2ISD::RET_FLAG", SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

let isTerminator = 1, isReturn = 1, isBarrier = 1 in {
    def RET : P2Inst<0b1101011,
                (outs),
                (ins),
                "ret",
                [(P2retflag)]>;
}

let Constraints = "$src1 = $dst" in {
    def ADDrr       : P2Inst<0b0001000,  (outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "add $dst, $src2", [(set P2GPR:$dst, (add P2GPR:$src1, P2GPR:$src2))]>;
    def ADDri       : P2Inst<0b0001000,  (outs P2GPR:$dst), (ins P2GPR:$src1, i32imm:$src2), "add $dst, #$src2", [(set P2GPR:$dst, (add P2GPR:$src1, imm:$src2))]>;
}

def MOVrr       : P2Inst<0b0110000,  (outs P2GPR:$dst), (ins P2GPR:$src), "mov $dst, $src", [(set P2GPR:$dst, P2GPR:$src)]>;

def MOVri       : P2Inst<0b0110000,  (outs P2GPR:$dst), (ins i32imm:$src), "mov $dst, #$src", [(set P2GPR:$dst, imm:$src)]>;

def RDLONGrr    : P2Inst<0b1011000,  (outs P2GPR:$dst), (ins P2GPR:$src), "rdlong $dst, $src", [(set P2GPR:$dst, (load P2GPR:$src))]>;

def WRLONGrr    : P2Inst<0b1100011,  (outs), (ins P2GPR:$dst, P2GPR:$src), "wrlong $dst, $src", [(store P2GPR:$dst, P2GPR:$src)]>;
def WRLONGir    : P2Inst<0b1100011,  (outs), (ins i32imm:$dst, P2GPR:$src), "wrlong #$dst, $src", [(store imm:$dst, P2GPR:$src)]>;

/// No operation
def NOP         : P2Inst<0b0000000, (outs), (ins), "nop", []>;

/* Pseudo instructions for intermediate steps */
def FRMIDX : Pseudo<(outs P2GPR:$dst), (ins P2GPR:$src, i32imm:$src2), "frmidx $dst, $src, $src2", []>;

/* patterns */
//def : Pat<(set i32:$dst, (load i32:$src)), (MOVrr P2GPR:$dst, P2GPR:$src)>;
//def : Pat<(set P2GPR:$dst, (load P2GPR:$src)), (set P2GPR:$dst, P2GPR:$src)>;
//def : Pat<(store P2GPR:$dst, P2GPR:$src), (MOVrr P2GPR:$dst, P2GPR:$src)>;

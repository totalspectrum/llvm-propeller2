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

// TODO: create multiclasses to reduce the number of repetitions when definition immediate versions of instructions

include "P2InstrFormats.td"

/* Function Calling Instructions */

def P2RET : SDNode<"P2ISD::RET", SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

let isTerminator = 1, isReturn = 1, isBarrier = 1 in {
    def RET     : P2Inst<0b1101011, (outs), (ins), "ret", [(P2RET)]>;
}

/* Basic math and logic */

let Constraints = "$src1 = $dst" in {
    def ADDrr   : P2Inst<0b0001000,  (outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "add $dst, $src2", [(set P2GPR:$dst, (add P2GPR:$src1, P2GPR:$src2))]>;
    def ADDri   : P2Inst<0b0001000,  (outs P2GPR:$dst), (ins P2GPR:$src1, i32imm:$src2), "add $dst, #$src2", [(set P2GPR:$dst, (add P2GPR:$src1, imm:$src2))]>;

    def SUBrr   : P2Inst<0b0001100,  (outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "sub $dst, $src2", [(set P2GPR:$dst, (sub P2GPR:$src1, P2GPR:$src2))]>;
    def SUBri   : P2Inst<0b0001100,  (outs P2GPR:$dst), (ins P2GPR:$src1, i32imm:$src2), "sub $dst, #$src2", [(set P2GPR:$dst, (sub P2GPR:$src1, imm:$src2))]>;
}

/* More complex math and logic */
//def P2DIV       : SDNode<"P2ISD::DIV", SDTIntBinOp, [SDNPOutGlue]>;
//def P2REM       : SDNode<"P2ISD::REM", SDTIntBinOp, [SDNPOutGlue]>;

def GETQX       : P2Inst<0b1101011, (outs P2GPR:$dst), (ins P2Implicit:$qx), "getqx $dst", []>;
def GETQY       : P2Inst<0b1101011, (outs P2GPR:$dst), (ins P2Implicit:$qy), "getqy $dst", []>;

def QDIV        : P2Inst<0b1101000, (outs P2Implicit:$qx, P2Implicit:$qy), (ins P2GPR:$src1, P2GPR:$src2), "qdiv $src1, $src2", []>;

def QSREM       : Pseudo<(outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "srem $dst, $src1, $src2 PSEUDO", [(set P2GPR:$dst, (srem P2GPR:$src1, P2GPR:$src2))]>;
def QUREM       : Pseudo<(outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "Urem $dst, $src1, $src2 PSEUDO", [(set P2GPR:$dst, (srem P2GPR:$src1, P2GPR:$src2))]>;

//def : Pat<(srem i32:$src1, i32:$src2), (GETQY (QDIV i32:$src1, i32:$src2))>;
/* branching and conditionals */

// how branching will work:
// define branch instruction for various compare branches (equal, not equal, etc), they will probably be all subclasses of common branch with different effects stuck to it.
// define a pattern that matched a compare instruction output inputting to one of the branch instructions for various IR conditional branch patterns


def jmptarget    : Operand<OtherVT> {
    let EncoderMethod = "getJumpTargetOpValue";
    let OperandType = "OPERAND_PCREL";
}

def CMPrr       : P2Inst<0b0010000, (outs P2Implicit:$dst), (ins P2GPR:$src1, P2GPR:$src2), "cmp $src1, $src2\twcz", []>;
def CMPri       : P2Inst<0b0010000, (outs P2Implicit:$dst), (ins P2GPR:$src1, i32imm:$src2), "cmp $src1, #$src2\twcz", []>;
def CMPRrr      : P2Inst<0b0010100, (outs P2Implicit:$dst), (ins P2GPR:$src1, P2GPR:$src2), "cmpr $src1, $src2\twcz", []>;
def CMPRri      : P2Inst<0b0010100, (outs P2Implicit:$dst), (ins P2GPR:$src1, i32imm:$src2), "cmpr $src1, #$src2\twcz", []>;

def JMP         : P2BranchInst<(outs), (ins jmptarget:$target), always, [(br bb:$target)]>;
def JMPeq       : P2BranchInst<(outs), (ins P2Implicit:$cmp, jmptarget:$target), if_z, []>;
def JMPne       : P2BranchInst<(outs), (ins P2Implicit:$cmp, jmptarget:$target), if_nz, []>;
def JMPlt       : P2BranchInst<(outs), (ins P2Implicit:$cmp, jmptarget:$target), if_c, []>;
def JMPgt       : P2BranchInst<(outs), (ins P2Implicit:$cmp, jmptarget:$target), if_c, []>; // will use CMPR for greater than

// Patterns for various branch conditions
def : Pat<(brcc SETEQ, P2GPR:$lhs, P2GPR:$rhs, bb:$target), (JMPeq (CMPrr P2GPR:$lhs, P2GPR:$rhs), bb:$target)>;
def : Pat<(brcc SETEQ, P2GPR:$lhs, imm:$rhs, bb:$target), (JMPeq (CMPri P2GPR:$lhs, imm:$rhs), bb:$target)>;

def : Pat<(brcc SETNE, P2GPR:$lhs, P2GPR:$rhs, bb:$target), (JMPne (CMPrr P2GPR:$lhs, P2GPR:$rhs), bb:$target)>;
def : Pat<(brcc SETNE, P2GPR:$lhs, imm:$rhs, bb:$target), (JMPne (CMPri P2GPR:$lhs, imm:$rhs), bb:$target)>;

def : Pat<(brcc SETLT, P2GPR:$lhs, P2GPR:$rhs, bb:$target), (JMPgt (CMPrr P2GPR:$lhs, P2GPR:$rhs), bb:$target)>;
def : Pat<(brcc SETLT, P2GPR:$lhs, imm:$rhs, bb:$target), (JMPgt (CMPri P2GPR:$lhs, imm:$rhs), bb:$target)>;

def : Pat<(brcc SETGT, P2GPR:$lhs, P2GPR:$rhs, bb:$target), (JMPgt (CMPRrr P2GPR:$lhs, P2GPR:$rhs), bb:$target)>;
def : Pat<(brcc SETGT, P2GPR:$lhs, imm:$rhs, bb:$target), (JMPgt (CMPRri P2GPR:$lhs, imm:$rhs), bb:$target)>;

/* Moving data */
def MOVrr       : P2Inst<0b0110000,  (outs P2GPR:$dst), (ins P2GPR:$src), "mov $dst, $src", [(set P2GPR:$dst, P2GPR:$src)]>;
def MOVri       : P2Inst<0b0110000,  (outs P2GPR:$dst), (ins i32imm:$src), "mov $dst, #$src", [(set P2GPR:$dst, imm:$src)]>;

def RDLONGrr    : P2Inst<0b1011000,  (outs P2GPR:$dst), (ins P2GPR:$src), "rdlong $dst, $src", [(set P2GPR:$dst, (load P2GPR:$src))]>;

def WRLONGrr    : P2Inst<0b1100011,  (outs), (ins P2GPR:$dst, P2GPR:$src), "wrlong $dst, $src", [(store P2GPR:$dst, P2GPR:$src)]>;
def WRLONGir    : P2Inst<0b1100011,  (outs), (ins i32imm:$dst, P2GPR:$src), "wrlong #$dst, $src", [(store imm:$dst, P2GPR:$src)]>;

/* Misc instructions */
def NOP         : P2Inst<0b0000000, (outs), (ins), "nop", []>;

/* Pseudo instructions for intermediate steps */
def FRMIDX      : Pseudo<(outs P2GPR:$dst), (ins P2GPR:$src, i32imm:$src2), "frmidx $dst, $src, $src2 PSEUDO", []>;

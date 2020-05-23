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

/*--------------------------------*/
/* General type definitions       */
/*--------------------------------*/

def SDT_P2Call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_P2Mov : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>]>;
def SDT_P2CallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_P2CallSeqEnd : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

def jmptarget    : Operand<OtherVT> {
    let EncoderMethod = "getJumpTargetOpValue";
    let OperandType = "OPERAND_PCREL";
}

def calltarget : Operand<iPTR> {
    let EncoderMethod = "encodeCallTarget";
}

def NegateImm : SDNodeXForm<imm, [{return CurDAG->getConstant(-N->getSExtValue(), SDLoc(N), MVT::i32);}]>;

def NegSubInlineConst32 : ImmLeaf<i32, [{return Imm < 0;}], NegateImm>;

/*--------------------------------*/
/* custom nodes                   */
/*--------------------------------*/
// this is used for loading global addresses
def P2GAWrapper    : SDNode<"P2ISD::GAWRAPPER", SDT_P2Mov>;

def P2callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_P2CallSeqStart, [SDNPHasChain, SDNPOutGlue]>;
def P2callseq_end : SDNode<"ISD::CALLSEQ_END", SDT_P2CallSeqEnd, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def P2RET   : SDNode<"P2ISD::RET", SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def P2CALL  : SDNode<"P2ISD::CALL", SDT_P2Call, [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;

/*--------------------------------*/
/* basic math and logic           */
/*--------------------------------*/

let Constraints = "$src1 = $dst" in {
    def ADDrr   : P2Inst<0b0001000,  (outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "add $dst, $src2", [(set P2GPR:$dst, (add P2GPR:$src1, P2GPR:$src2))]>;
    def ADDri   : P2Inst<0b0001000,  (outs P2GPR:$dst), (ins P2GPR:$src1, i32imm:$src2), "add $dst, #$src2", [(set P2GPR:$dst, (add P2GPR:$src1, imm:$src2))]>;

    def SUBrr   : P2Inst<0b0001100,  (outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "sub $dst, $src2", [(set P2GPR:$dst, (sub P2GPR:$src1, P2GPR:$src2))]>;
    def SUBri   : P2Inst<0b0001100,  (outs P2GPR:$dst), (ins P2GPR:$src1, i32imm:$src2), "sub $dst, #$src2", [(set P2GPR:$dst, (sub P2GPR:$src1, imm:$src2))]>;
}

/*--------------------------------*/
/* More complicated math          */
/*--------------------------------*/

def GETQX       : P2Inst<0b1101011, (outs P2GPR:$dst), (ins P2Implicit:$qx), "getqx $dst", []>;
def GETQY       : P2Inst<0b1101011, (outs P2GPR:$dst), (ins P2Implicit:$qy), "getqy $dst", []>;

def QDIV        : P2Inst<0b1101000, (outs P2Implicit:$qx, P2Implicit:$qy), (ins P2GPR:$src1, P2GPR:$src2), "qdiv $src1, $src2", []>;

def QSREM       : Pseudo<(outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "srem $dst, $src1, $src2", [(set P2GPR:$dst, (srem P2GPR:$src1, P2GPR:$src2))]>;
def QUREM       : Pseudo<(outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "Urem $dst, $src1, $src2", [(set P2GPR:$dst, (srem P2GPR:$src1, P2GPR:$src2))]>;

/*--------------------------------*/
/* branching and conditionals     */
/*--------------------------------*/

// how branching will work:
// define branch instruction for various compare branches (equal, not equal, etc), they will probably be all subclasses of common branch with different effects stuck to it.
// define a pattern that matched a compare instruction output inputting to one of the branch instructions for various IR conditional branch patterns

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

/*--------------------------------*/
/* moving data                    */
/*--------------------------------*/
def MOVri       : P2Inst<0b0110000,  (outs P2GPR:$dst), (ins i32imm:$src), "mov $dst, #$src", [(set P2GPR:$dst, imm:$src)]>;
def MOVrr       : P2Inst<0b0110000,  (outs P2GPR:$dst), (ins P2GPR:$src), "mov $dst, $src", [(set P2GPR:$dst, P2GPR:$src)]>;

def RDLONGrr    : P2Inst<0b1011000,  (outs P2GPR:$dst), (ins P2GPR:$src), "rdlong $dst, $src", [(set P2GPR:$dst, (load P2GPR:$src))]>;

def WRLONGrr    : P2Inst<0b1100011,  (outs), (ins P2GPR:$dst, P2GPR:$src), "wrlong $dst, $src", [(store P2GPR:$dst, P2GPR:$src)]>;
def WRLONGir    : P2Inst<0b1100011,  (outs), (ins i32imm:$dst, P2GPR:$src), "wrlong #$dst, $src", [(store imm:$dst, P2GPR:$src)]>;

//def : Pat<(i32 imm:$i), (MOVri i32imm:$i)>;

/*--------------------------------*/
/* function calling and globals   */
/*--------------------------------*/

let isTerminator = 1, isReturn = 1, isBarrier = 1 in {
    def RET     : P2Inst<0b1101011, (outs), (ins), "ret", [(P2RET)]>;
}

let isCall = 1, Uses = [SP] in {
    def CALL    : P2Inst<0b1101101, (outs), (ins calltarget:$addr), "call #$addr", []>;
}

let Defs = [SP], Uses = [SP] in {
    def ADJCALLSTACKUP      : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2), "ADJCALLSTACKUP", [(P2callseq_start timm:$amt1, timm:$amt2)]>;
    def ADJCALLSTACKDOWN    : Pseudo<(outs), (ins i32imm:$amt, i32imm:$amt2), "ADJCALLSTACKDOWN", [(P2callseq_end timm:$amt, timm:$amt2)]>;
}

def : Pat<(P2GAWrapper tglobaladdr:$addr), (MOVri tglobaladdr:$addr)>;
def : Pat<(P2CALL (i32 tglobaladdr:$dst)), (CALL tglobaladdr:$dst)>;
def : Pat<(P2CALL (i32 texternalsym:$dst)), (CALL texternalsym:$dst)>;

/*--------------------------------*/
/* misc instructions              */
/*--------------------------------*/
def NOP         : P2Inst<0b0000000, (outs), (ins), "nop", []>;
def AUGS        : P2Inst<0b1111000, (outs), (ins i32imm:$n), "augs #$n", []>;
def AUGD        : P2Inst<0b1111100, (outs), (ins i32imm:$n), "augd #$n", []>;

def FRMIDX      : Pseudo<(outs P2GPR:$dst), (ins P2GPR:$src, i32imm:$src2), "frmidx $dst, $src, $src2", []>;

/*--------------------------------*/
/* Optimization patterns and such */
/*--------------------------------*/

// convert add -c to sub c
// def : Pat<(add i32:$src0, (i32 NegSubInlineConst32:$src1)),
//             (SUBri $src0, NegSubInlineConst32:$src1)>;

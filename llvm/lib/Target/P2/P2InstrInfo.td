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

//def NegateImm : SDNodeXForm<imm, [{return CurDAG->getConstant(-N->getSExtValue(), SDLoc(N), MVT::i32);}]>;
def NegateImm : SDNodeXForm<imm, [{return getImm(N, -N->getSExtValue());}]>;
def NegSubInlineConst32 : ImmLeaf<i32, [{return Imm < 0;}], NegateImm>;

/*--------------------------------*/
/* custom nodes                   */
/*--------------------------------*/
// this is used for loading global addresses
def P2GAWrapper    : SDNode<"P2ISD::GAWRAPPER", SDT_P2Mov>;

def P2callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_P2CallSeqStart, [SDNPHasChain, SDNPOutGlue]>;
def P2callseq_end : SDNode<"ISD::CALLSEQ_END", SDT_P2CallSeqEnd, [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def P2RDLUT  : SDNode<"P2ISD::RDLUT", SDTLoad, [SDNPHasChain, SDNPMayLoad, SDNPMemOperand]>;
def P2WRLUT  : SDNode<"P2ISD::WRLUT", SDTStore, [SDNPHasChain, SDNPMayStore, SDNPMemOperand]>;

def P2RET   : SDNode<"P2ISD::RET", SDTNone, [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def P2CALL  : SDNode<"P2ISD::CALL", SDT_P2Call, [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue, SDNPVariadic]>;

def rdlut : PatFrag<(ops node:$ptr), (P2RDLUT node:$ptr)> {
    let IsNonExtLoad = 1;
    let IsUnindexed = 1;
}

def wrlut : PatFrag<(ops node:$val, node:$ptr), (P2WRLUT node:$val, node:$ptr)> {
    let IsTruncStore = 0;
    let IsUnindexed = 1;
}

/*--------------------------------*/
/* basic math and logic           */
/*--------------------------------*/

let Constraints = "$src1 = $d" in {
    def ADDrr   : P2InstCZIDS<0b0001000,  0b000, always, (outs P2GPR:$d), (ins P2GPR:$src1, P2GPR:$s), "add $d, $s", [(set P2GPR:$d, (add P2GPR:$src1, P2GPR:$s))]>;
    def ADDri   : P2InstCZIDS<0b0001000,  0b001, always, (outs P2GPR:$d), (ins P2GPR:$src1, i32imm:$s), "add $d, #$s", [(set P2GPR:$d, (add P2GPR:$src1, imm:$s))]>;

    def SUBrr   : P2InstCZIDS<0b0001100,  0b000, always, (outs P2GPR:$d), (ins P2GPR:$src1, P2GPR:$s), "sub $d, $s", [(set P2GPR:$d, (sub P2GPR:$src1, P2GPR:$s))]>;
    def SUBri   : P2InstCZIDS<0b0001100,  0b001, always, (outs P2GPR:$d), (ins P2GPR:$src1, i32imm:$s), "sub $d, #$s", [(set P2GPR:$d, (sub P2GPR:$src1, imm:$s))]>;
}

/*--------------------------------*/
/* More complicated math          */
/*--------------------------------*/

def GETQX       : P2InstCZD<0b1101011, 0b000011000, 0b00, always, (outs P2GPR:$d), (ins P2Implicit:$qx), "getqx $d", []>;
def GETQY       : P2InstCZD<0b1101011, 0b000011001, 0b00, always, (outs P2GPR:$d), (ins P2Implicit:$qy), "getqy $d", []>;

def QDIV        : P2InstLIDS<0b11010001, 0b00, always, (outs P2Implicit:$qx, P2Implicit:$qy), (ins P2GPR:$src1, P2GPR:$src2), "qdiv $src1, $src2", []>;

def QSREM       : Pseudo<(outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "srem $dst, $src1, $src2", [(set P2GPR:$dst, (srem P2GPR:$src1, P2GPR:$src2))]>;
def QUREM       : Pseudo<(outs P2GPR:$dst), (ins P2GPR:$src1, P2GPR:$src2), "urem $dst, $src1, $src2", [(set P2GPR:$dst, (srem P2GPR:$src1, P2GPR:$src2))]>;

/*--------------------------------*/
/* branching and conditionals     */
/*--------------------------------*/

// how branching will work:
// define branch instruction for various compare branches (equal, not equal, etc), they will probably be all subclasses of common branch with different effects stuck to it.
// define a pattern that matched a compare instruction output inputting to one of the branch instructions for various IR conditional branch patterns

def CMPrr       : P2InstCZIDS<0b0010000, 0b110, always, (outs P2Implicit:$dst), (ins P2GPR:$d, P2GPR:$s), "cmp $d, $s\twcz", []>;
def CMPri       : P2InstCZIDS<0b0010000, 0b111, always, (outs P2Implicit:$dst), (ins P2GPR:$d, i32imm:$s), "cmp $d, #$s\twcz", []>;
def CMPRrr      : P2InstCZIDS<0b0010100, 0b110, always, (outs P2Implicit:$dst), (ins P2GPR:$d, P2GPR:$s), "cmpr $d, $s\twcz", []>;
def CMPRri      : P2InstCZIDS<0b0010100, 0b111, always, (outs P2Implicit:$dst), (ins P2GPR:$d, i32imm:$s), "cmpr $d, #$s\twcz", []>;

let DecoderMethod = "DecodeJumpInstruction" in {
    def JMP         : P2BranchInst<(outs), (ins jmptarget:$target), always, [(br bb:$target)]>;
    def JMPeq       : P2BranchInst<(outs), (ins jmptarget:$target, P2Implicit:$cmp), if_z, []>;
    def JMPne       : P2BranchInst<(outs), (ins jmptarget:$target, P2Implicit:$cmp), if_nz, []>;
    def JMPlt       : P2BranchInst<(outs), (ins jmptarget:$target, P2Implicit:$cmp), if_c, []>; // will use CMPR for greater than
    def JMPgte      : P2BranchInst<(outs), (ins jmptarget:$target, P2Implicit:$cmp), if_nc, []>; // will use CMPR for less than or equal
}

// Patterns for various branch conditions
def : Pat<(brcc SETEQ, P2GPR:$lhs, P2GPR:$rhs, bb:$target), (JMPeq bb:$target, (CMPrr P2GPR:$lhs, P2GPR:$rhs))>;
def : Pat<(brcc SETEQ, P2GPR:$lhs, imm:$rhs, bb:$target), (JMPeq bb:$target, (CMPri P2GPR:$lhs, imm:$rhs))>;

def : Pat<(brcc SETNE, P2GPR:$lhs, P2GPR:$rhs, bb:$target), (JMPne bb:$target, (CMPrr P2GPR:$lhs, P2GPR:$rhs))>;
def : Pat<(brcc SETNE, P2GPR:$lhs, imm:$rhs, bb:$target), (JMPne bb:$target, (CMPri P2GPR:$lhs, imm:$rhs))>;

def : Pat<(brcc SETLT, P2GPR:$lhs, P2GPR:$rhs, bb:$target), (JMPlt bb:$target, (CMPrr P2GPR:$lhs, P2GPR:$rhs))>;
def : Pat<(brcc SETLT, P2GPR:$lhs, imm:$rhs, bb:$target), (JMPlt bb:$target, (CMPri P2GPR:$lhs, imm:$rhs))>;

def : Pat<(brcc SETGT, P2GPR:$lhs, P2GPR:$rhs, bb:$target), (JMPlt bb:$target, (CMPRrr P2GPR:$lhs, P2GPR:$rhs))>;
def : Pat<(brcc SETGT, P2GPR:$lhs, imm:$rhs, bb:$target), (JMPlt bb:$target, (CMPRri P2GPR:$lhs, imm:$rhs))>;

/*--------------------------------*/
/* moving data                    */
/*--------------------------------*/

def MOVri       : P2InstCZIDS<0b0110000, 0b001, always, (outs P2GPR:$d), (ins i32imm:$s), "mov $d, #$s", [(set P2GPR:$d, imm:$s)]>;
def MOVrr       : P2InstCZIDS<0b0110000, 0b000, always, (outs P2GPR:$d), (ins P2GPR:$s), "mov $d, $s", [(set P2GPR:$d, P2GPR:$s)]>;

def RDLONGrr    : P2InstCZIDS<0b1011000, 0b000, always, (outs P2GPR:$d), (ins P2GPR:$s), "rdlong $d, $s", [(set P2GPR:$d, (load P2GPR:$s))]>;

def WRLONGrr    : P2InstLIDS<0b11000110, 0b00, always, (outs), (ins P2GPR:$d, P2GPR:$s), "wrlong $d, $s", [(store P2GPR:$d, P2GPR:$s)]>;
def WRLONGir    : P2InstLIDS<0b11000110, 0b10, always, (outs), (ins i32imm:$d, P2GPR:$s), "wrlong #$d, $s", [(store imm:$d, P2GPR:$s)]>;

/*
def RDLUTrr     : P2InstCZIDS<0b1010101, 0b000, always, (outs P2GPR:$d), (ins P2GPR:$s), "rdlut %d, %s", [(set P2GPR:$d, (rdlut P2GPR:$s))]>;
def RDLUTri     : P2InstCZIDS<0b1010101, 0b001, always, (outs P2GPR:$d), (ins i32imm:$s), "rdlut %d, #%s", [(set P2GPR:$d, (rdlut imm:$s))]>;

def WRLUTrr     : P2InstLIDS<0b11000011, 0b00, always, (outs), (ins P2GPR:$d, P2GPR:$s), "wrlut %d, %s", [(wrlut P2GPR:$d, P2GPR:$s)]>;
def WRLUTri     : P2InstLIDS<0b11000011, 0b01, always, (outs), (ins P2GPR:$d, i32imm:$s), "wrlut %d, #%s", [(wrlut P2GPR:$d, imm:$s)]>;
def WRLUTir     : P2InstLIDS<0b11000011, 0b10, always, (outs), (ins i32imm:$d, P2GPR:$s), "wrlut #%d, %s", [(wrlut imm:$d, P2GPR:$s)]>;
def WRLUTii     : P2InstLIDS<0b11000011, 0b11, always, (outs), (ins i32imm:$d, i32imm:$s), "wrlut #%d, #%s", [(wrlut imm:$d, imm:$s)]>;
*/

def MOVri32     : Pseudo<(outs P2GPR:$d), (ins i32imm:$s), "mov $d, ##s", []>;

/*--------------------------------*/
/* function calling and globals   */
/*--------------------------------*/

let isTerminator = 1, isReturn = 1, isBarrier = 1 in {
    def RET     : P2InstCZ<0b11010111, 0b000000000, 0b000101101, 0b00, always, (outs), (ins), "ret", [(P2RET)]>;
}

let isCall = 1, Uses = [SP] in {
    def CALL    : P2InstRA<0b1101101, 0b0, always, (outs), (ins calltarget:$a), "call #$a", []>;
}

let Defs = [SP], Uses = [SP] in {
    def ADJCALLSTACKUP      : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2), "ADJCALLSTACKUP", [(P2callseq_start timm:$amt1, timm:$amt2)]>;
    def ADJCALLSTACKDOWN    : Pseudo<(outs), (ins i32imm:$amt, i32imm:$amt2), "ADJCALLSTACKDOWN", [(P2callseq_end timm:$amt, timm:$amt2)]>;
}

def : Pat<(P2GAWrapper tglobaladdr:$addr), (MOVri32 tglobaladdr:$addr)>;
def : Pat<(P2CALL (i32 tglobaladdr:$dst)), (CALL tglobaladdr:$dst)>;
def : Pat<(P2CALL (i32 texternalsym:$dst)), (CALL texternalsym:$dst)>;

/*--------------------------------*/
/* I/O and hardware control       */
/*--------------------------------*/

let DecoderMethod = "DecodeIOInstruction" in {
    def DIRLr       : P2InstCZLD<0b1101011, 0b001000000, 0b000, always, (outs P2GPR:$dir), (ins P2GPR:$d), "dirl $d", []>;
    def DIRLi       : P2InstCZLD<0b1101011, 0b001000000, 0b001, always, (outs P2GPR:$dir), (ins i32imm:$d), "dirl #$d", []>;

    def DIRHr       : P2InstCZLD<0b1101011, 0b001000001, 0b000, always, (outs P2GPR:$dir), (ins P2GPR:$d), "dirh $d", []>;
    def DIRHi       : P2InstCZLD<0b1101011, 0b001000001, 0b001, always, (outs P2GPR:$dir), (ins i32imm:$d), "dirh #$d", []>;

    def OUTLr       : P2InstCZLD<0b1101011, 0b001001000, 0b000, always, (outs P2GPR:$dir), (ins P2GPR:$d), "outl $d", []>;
    def OUTLi       : P2InstCZLD<0b1101011, 0b001001000, 0b001, always, (outs P2GPR:$dir), (ins i32imm:$d), "outl #$d", []>;

    def OUTHr       : P2InstCZLD<0b1101011, 0b001001001, 0b000, always, (outs P2GPR:$dir), (ins P2GPR:$d), "outh $d", []>;
    def OUTHi       : P2InstCZLD<0b1101011, 0b001001001, 0b001, always, (outs P2GPR:$dir), (ins i32imm:$d), "outh #$d", []>;
}

def WAITXr      : P2InstLD<0b1101011, 0b000011111, 0b0, always, (outs), (ins P2GPR:$d), "waitx $d", []>;
def WAITXi      : P2InstLD<0b1101011, 0b000011111, 0b1, always, (outs), (ins i32imm:$d), "waitx #$d", []>;

def COGINITir   : P2InstCLIDS<0b1100111, 0b010, always, (outs), (ins i32imm:$d, P2GPR:$s), "coginit #$d, $s", []>;

/*--------------------------------*/
/* misc instructions              */
/*--------------------------------*/
def NOP         : P2Inst<_ret_, 0, (outs), (ins), "nop", []>;
def AUGS        : P2InstN<0b11110, always, (outs), (ins i32imm:$n), "augs #$n", []>;
def AUGD        : P2InstN<0b11111, always, (outs), (ins i32imm:$n), "augd #$n", []>;

def FRMIDX      : Pseudo<(outs P2GPR:$dst), (ins P2GPR:$src, i32imm:$src2), "frmidx $dst, $src, $src2", []>;

/*--------------------------------*/
/* Optimization patterns and such */
/*--------------------------------*/

// convert add -c to sub c
def : Pat<(add i32:$d, (i32 NegSubInlineConst32:$s)), (SUBri $d, NegSubInlineConst32:$s)>;

//===-- P2InstrFormats.td - P2 Instruction Formats -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// instruction formats in P2
//
// Each instruction breaks down as one of the following:
//   EEEE | OOOOOOO | CZI | DDDDDDDDD | SSSSSSSSS || most math and logic commands. Single operand instructions have specific S values and I = 0. only S can be immediate
//   EEEE | OOOOOOO | CZL | DDDDDDDDD | SSSSSSSSS || single operand where D can be immediate, and can write both C and Z
//   EEEE | OOOOOOO | CLI | DDDDDDDDD | SSSSSSSSS || (notice that L can move!) instructions where both D and S can be immediates
//   EEEE | OOOOOON | NNI | DDDDDDDDD | SSSSSSSSS || special instructions that sets word or nibbles defined by N
//   EEEE | OOOOOOO | RAA | AAAAAAAAA | AAAAAAAAA || call/jump instructions that modify the entire PC (for hub exec modes)
//   EEEE | OOOOOnn | nnn | nnnnnnnnn | nnnnnnnnn || AUGS/AUGD
//
// E = Effects. conditions stuck on the instruction to determine if it should be executed based on status word flags
// O = op code. typically 7 bits, sometimes fewer and the extra bits are used as operands
// C = write C. Will write C status flag if 1
// Z = write Z. Will write Z status flag if 1
// I = S is immediate value (and not register) if 1
// L = D is immediate value (and not register) if 1
// N = nib or short for special instructions that operate on them
// R = relative for hub exec call/jumps
// n = immediate to be glued to the next #D or #S immediate to turn it into a 32 bit immediate value.
//===----------------------------------------------------------------------===//

// effect string/encoding pairing
class P2Effect<string str, bits<4> eff> {
    field bits<4> effect = eff;
    field string effect_str = str;
}

// keep this in sync with the Instruction printer
def _ret_           : P2Effect<"_ret_", 0b0000>;
def if_nc_and_nz    : P2Effect<"if_nc_and_nz", 0b0001>;
def if_nc_and_z     : P2Effect<"if_nc_and_z", 0b0010>;
def if_nc           : P2Effect<"if_nc", 0b0011>;
def if_c_and_nz     : P2Effect<"if_c_and_nz", 0b0100>;
def if_nz           : P2Effect<"if_nz", 0b0101>;
def if_c_ne_z       : P2Effect<"if_c_ne_z", 0b0110>;
def if_nc_or_nz     : P2Effect<"if_nc_or_nz", 0b0111>;
def if_c_and_z      : P2Effect<"if_c_and_z", 0b1000>;
def if_c_eq_z       : P2Effect<"if_c_eq_z", 0b1001>;
def if_z            : P2Effect<"if_z", 0b1010>;
def if_nc_or_z      : P2Effect<"if_nc_or_z", 0b1011>;
def if_c            : P2Effect<"if_c", 0b1100>;
def if_c_or_nz      : P2Effect<"if_c_or_nz", 0b1101>;
def if_c_or_z       : P2Effect<"if_c_or_z", 0b1110>;
def always          : P2Effect<"", 0b1111>;

// generic instructions class
class P2GenInst<bits<7> op, P2Effect eff, dag outs, dag ins, string asmstr, list<dag> pattern>: Instruction {
    // Inst and Size: for tablegen(... -gen-emitter) and
    // tablegen(... -gen-disassembler) in CMakeLists.txt
    field bits<32> Inst;

    let Namespace = "P2";
    let Size = 4;

    bits<4> effect = 0;
    bits<7> opcode = 0;
    bits<3> flags = 0; // C, Z, and I flags (for write c, write z, and immediate value instruction flags)
    bits<9> rd = 0; // desitination register address
    bits<9> rs = 0; // source register address

    let effect = eff.effect;
    let opcode = op;

    // Top 4 bits are the 'effect' field
    let Inst{31-28} = effect;
    // next 7 are opcode
    let Inst{27-21} = opcode;
    // then C, Z, I (or other values)
    let Inst{20-18} = flags;
    // then 9 bit destination
    let Inst{17-9} = rd; // tbd how to encode these from the operands list.
    // then 9 bit source
    let Inst{8-0} = rs;

    let OutOperandList = outs;
    let InOperandList  = ins;

    let AsmString   = asmstr;
    let Pattern     = pattern;

    // TSFlags layout should be kept in sync with P2InstrInfo.h.
    let TSFlags{3-0}   = effect; // put the effect bits into the TSFlags field for use by the instruction printer

    let DecoderNamespace = "P2";
}

// typical instruction that should always execute
class P2Inst<bits<7> op, dag outs, dag ins, string asmstr, list<dag> pattern> :
    P2GenInst<op, always, outs, ins, asmstr, pattern> {}

// branch instruction that should execute based on the effects
// ins must have a "$target" operand
class P2BranchInst<dag outs, dag ins, P2Effect eff, list<dag> pattern> :
    P2GenInst<0b1101100, eff, outs, ins, "jmp #$target", pattern> {

    let isBarrier = 1;
    let isBranch = 1;
    let isTerminator = 1;
}

class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern> : P2Inst<0, outs, ins, !strconcat(asmstr, " PSEUDO"), pattern> {
    let Pattern = pattern;
    let isPseudo = 1;
    let isCodeGenOnly = 1;
}

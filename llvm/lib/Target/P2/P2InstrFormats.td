//===-- P2InstrFormats.td - P2 Instruction Formats -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Describe P2 instructions format
//
//  CPU INSTRUCTION FORMATS
//
//  opcode  - operation code.
//  d       - dst reg
//  s/#     - src reg or immediate
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// instruction class in P2 : <|opcode|rd|rs|>
//
// define this to match the propeller instruction format. this doesn't inlcude
// the few 3 operand instructions (that take an extra imm value), but we'll
// get there
//===----------------------------------------------------------------------===//

class P2Inst<bits<7> op, dag outs, dag ins, string asmstr, list<dag> pattern>: Instruction {
  // Inst and Size: for tablegen(... -gen-emitter) and
  // tablegen(... -gen-disassembler) in CMakeLists.txt
  field bits<32> Inst;

  let Namespace = "P2";
  let Size = 4;

  bits<4> Effect = 0xf; // no effect i.e. always execute is 0xf
  bits<7> Opcode = 0;
  bits<3> CZI = 0; // C, Z, and I flags (for write c, write z, and immediate value instruction flags)
  bits<9> rd = 0; // desitination register address
  bits<9> rs = 0; // source register address

  let Opcode = op;

  // Top 4 bits are the 'effect' field
  let Inst{31-28} = Effect;
  let Inst{27-21} = Opcode;
  let Inst{20-18} = CZI;
  let Inst{17-9} = rd;
  let Inst{8-0} = rs;

  let OutOperandList = outs;
  let InOperandList  = ins;

  let AsmString   = asmstr;
  let Pattern     = pattern;

  // TSFlags layout should be kept in sync with P2InstrInfo.h.
  // let TSFlags{3-0}   = FormBits; // idk what this is yet

  let DecoderNamespace = "P2";

}
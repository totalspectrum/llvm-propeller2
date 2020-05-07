//===-- P2RegisterInfo.td - P2 Register defs -----------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the P2 register file
//
//  propeller doesn't have a real register file, other than it's 16 special
//  registers at the end of COG ram. We'll extend that to include 16 general
//  purpose registers before that, called r0-r15
//
//
//===----------------------------------------------------------------------===//

// each register is a 9 bit address
class P2Reg<bits<9> Enc, string n> : Register<n> {
  // For tablegen(... -gen-emitter)  in CMakeLists.txt
  let HWEncoding{8-0} = Enc;

  let Namespace = "P2";
}

// P2 cog Registers
class P2GPRReg<bits<9> Enc, string n> : P2Reg<Enc, n>;

//===----------------------------------------------------------------------===//
//@Registers
//===----------------------------------------------------------------------===//
// The register string, such as "9" or "gp" will show on "llvm-objdump -d"
//@ All registers definition
let Namespace = "P2" in {
  //@ General Purpose Registers
  // this will be modified as we adjust how the backend will work since each of the 512 "registers" can store either instructions
  // or data
  def R0 : P2GPRReg<0,  "r0">, DwarfRegNum<[0]>;
  def R1   : P2GPRReg<1,  "r1">,    DwarfRegNum<[1]>;
  def R2   : P2GPRReg<2,  "r2">,    DwarfRegNum<[2]>;
  def R3   : P2GPRReg<3,  "r3">,    DwarfRegNum<[3]>;
  def R4   : P2GPRReg<4,  "r4">,    DwarfRegNum<[4]>;
  def R5   : P2GPRReg<5,  "r5">,    DwarfRegNum<[5]>;
  def R6   : P2GPRReg<6,  "r6">,   DwarfRegNum<[6]>;
  def R7   : P2GPRReg<7,  "r7">,    DwarfRegNum<[7]>;
  def R8   : P2GPRReg<8,  "r8">,    DwarfRegNum<[8]>;
  def R9   : P2GPRReg<9,  "r9">,    DwarfRegNum<[9]>;
  def R10   : P2GPRReg<10, "r10">,   DwarfRegNum<[10]>;
  def R11   : P2GPRReg<11, "r11">,   DwarfRegNum<[11]>;
  def R12   : P2GPRReg<12, "r12">,   DwarfRegNum<[12]>;
  def R13   : P2GPRReg<13, "r13">,   DwarfRegNum<[13]>;
  def R14   : P2GPRReg<14, "r14">,   DwarfRegNum<[14]>;
  def R15   : P2GPRReg<15, "r15">,   DwarfRegNum<[15]>;
}

//===----------------------------------------------------------------------===//
//@Register Classes
//===----------------------------------------------------------------------===//

def CPURegs : RegisterClass<"P2", [i32], 32, (add
  // general purpose registers
  R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15
)>;

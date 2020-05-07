//===-- P2CallingConv.td - Calling Conventions for P2 --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for P2 architecture.
//===----------------------------------------------------------------------===//

def RetCC_AVR : CallingConv
<[
  // i8 is returned in R24.
  CCIfType<[i8], CCAssignToReg<[R24]>>,

  // i16 are returned in R25:R24, R23:R22, R21:R20 and R19:R18.
  CCIfType<[i16], CCAssignToReg<[R25R24, R23R22, R21R20, R19R18]>>
]>;
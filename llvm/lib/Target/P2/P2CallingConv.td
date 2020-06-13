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

def CC_P2 : CallingConv<[
    // Promote i8/i16 arguments to i32
    CCIfType<[i8, i16], CCPromoteToType<i32>>,
    // The first 4 arguments are passed in registers
    CCIfType<[i32], CCAssignToReg<[R0, R1, R2, R3]>>,
    // Fall-back, and use the stack, stack is 4 bytes wide and byte aligned.
    CCIfType<[i32], CCAssignToStack<4, 1>>,
    // use this for byvals
    CCIfByVal<CCPassByVal<4, 1>>
]>;

// Calling convention for variadic functions.
def CC_P2_Vararg : CallingConv <[
    // i32 are always passed through the stack with an alignment of 4.
    CCAssignToStack<4, 1>
]>;

def RetCC_P2 : CallingConv <[
    // i32 are returned in R15
    CCIfType<[i32], CCAssignToReg<[R15]>>,

    CCIfByVal<CCAssignToStack<4, 1>>
]>;

	.text
	.file	"test5.cpp"
	.globl	_Z3sumiiiii             ' -- Begin function _Z3sumiiiii
	.type	_Z3sumiiiii,@function
_Z3sumiiiii:                            ' @_Z3sumiiiii
' %bb.0:                                ' %entry
			mov r4, sp
			sub r4, #20
			rdlong r5, r4
			mov r5, sp
			sub r5, #16
			wrlong r0, r5
			mov r0, sp
			sub r0, #12
			wrlong r1, r0
			mov r1, sp
			sub r1, #8
			wrlong r2, r1
			mov r2, sp
			sub r2, #4
			wrlong r3, r2
			rdlong r15, r5
			rdlong r0, r0
			add r15, r0
			rdlong r0, r1
			add r15, r0
			rdlong r0, r2
			add r15, r0
			rdlong r0, r4
			add r15, r0
			ret
.Lfunc_end0:
	.size	_Z3sumiiiii, .Lfunc_end0-_Z3sumiiiii
                                        ' -- End function
	.globl	main                    ' -- Begin function main
	.type	main,@function
main:                                   ' @main
' %bb.0:                                ' %entry
			mov r0, sp
			sub r0, #8
			wrlong #0, r0
			add sp, #20
			mov r0, sp
			add r0, #16
			wrlong #5, r0
			mov r0, #1
			mov r1, #2
			mov r2, #3
			mov r3, #4
			call #_Z3sumiiiii
			sub sp, #20
			mov r0, sp
			sub r0, #4
			wrlong r15, r0
			rdlong r15, r0
			ret
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
                                        ' -- End function
	.ident	"clang version 11.0.0 (https://github.com/ne75/llvm-propeller2.git 18a7aff8960a14bc35064548075716e6fe921e92)"
	.section	".note.GNU-stack","",@progbits

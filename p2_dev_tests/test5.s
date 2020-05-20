	.text
	.file	"test5.cpp"
	.globl	_Z3sumii                ' -- Begin function _Z3sumii
	.type	_Z3sumii,@function
_Z3sumii:                               ' @_Z3sumii
' %bb.0:                                ' %entry
			mov r2, sp
			add r2, #4
			wrlong r0, r2
			mov r0, sp
			add r0, #0
			wrlong r1, r0
			rdlong r15, r2
			rdlong r0, r0
			add r15, r0
			ret
.Lfunc_end0:
	.size	_Z3sumii, .Lfunc_end0-_Z3sumii
                                        ' -- End function
	.globl	main                    ' -- Begin function main
	.type	main,@function
main:                                   ' @main
' %bb.0:                                ' %entry
			mov r0, sp
			add r0, #4
			wrlong #0, r0
			sub sp, #16
			mov r0, #2
			mov r1, #4
			call #_Z3sumii
			add sp, #16
			mov r0, sp
			add r0, #0
			wrlong r15, r0
			rdlong r15, r0
			ret
.Lfunc_end1:
	.size	main, .Lfunc_end1-main
                                        ' -- End function
	.type	c,@object               ' @c
	.data
	.globl	c
	.p2align	2
c:
c$local:
	.long	10                      ' 0xa
	.size	c, 4

	.ident	"clang version 11.0.0 (https://github.com/ne75/llvm-propeller2.git 14f814c5cfab3ec8fa7fe17a214cdb51c04ca235)"
	.section	".note.GNU-stack","",@progbits

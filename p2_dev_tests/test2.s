	.text
	.file	"test2.cpp"
	.globl	main                    '' -- Begin function main
	.type	main,@function
main:                                   '' @main
'' %bb.0:                               '' %entry
	mov r0, sp
	add r0, #12
	wrlong #5, r0
	mov r1, sp
	add r1, #16
	wrlong #2, r1
	rdlong r0, r0
	rdlong r1, r1
	add r0, r1
	mov r1, sp
	add r1, #20
	wrlong r0, r1
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        '' -- End function
	.ident	"clang version 11.0.0 (https://github.com/ne75/llvm-propeller2.git 1e3fd7a4ad4ef8a5cbde982352465ec3c3eb6559)"
	.section	".note.GNU-stack","",@progbits

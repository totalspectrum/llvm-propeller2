	.text
	.file	"test1.cpp"
	.globl	main                    ' -- Begin function main
	.type	main,@function
main:                                   ' @main
' %bb.0:                                ' %entry
	mov r0, sp
	add r0, #12
	wrlong #0, r0
	mov r0, sp
	add r0, #8
	wrlong #5, r0
	mov r1, sp
	add r1, #4
	wrlong #2, r1
	rdlong r0, r0
	rdlong r1, r1
	add r0, r1
	mov r1, sp
	add r1, #0
	wrlong r0, r1
	rdlong r15, r1
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        ' -- End function
	.ident	"clang version 11.0.0 (https://github.com/ne75/llvm-propeller2.git d1c3cdbf8cc823d3a4eaf073edc830b45da44b64)"
	.section	".note.GNU-stack","",@progbits

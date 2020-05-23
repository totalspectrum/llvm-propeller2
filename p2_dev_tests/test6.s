	.text
	.file	"test6.cpp"
	.globl	main                    '' -- Begin function main
	.type	main,@function
main:                                   '' @main
'' %bb.0:                               '' %entry
			add sp, #4
			mov r0, sp
			sub r0, #4
			wrlong #0, r0
.LBB0_1:                                '' %while.body
                                        '' =>This Inner Loop Header: Depth=1
	''APP
			dirh #56
	''NO_APP
	''APP
			augd #156250
	''NO_APP
	''APP
			waitx #0
	''NO_APP
	''APP
			dirl #56
	''NO_APP
	''APP
			augd #156250
	''NO_APP
	''APP
			waitx #0
	''NO_APP
			jmp #.LBB0_1
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        '' -- End function
	.type	x,@object               '' @x
	.section	.bss,"aw",@nobits
	.globl	x
	.p2align	2
x:
x$local:
	.long	0                       '' 0x0
	.size	x, 4

	.ident	"clang version 11.0.0 (https://github.com/ne75/llvm-propeller2.git e9f9d355c78c353223bb80946791140aee671499)"
	.section	".note.GNU-stack","",@progbits

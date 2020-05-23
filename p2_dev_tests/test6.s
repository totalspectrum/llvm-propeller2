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
	''APP
			dirh #56
	''NO_APP
			mov r15, #0
			sub sp, #4
			ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        '' -- End function
	.ident	"clang version 11.0.0 (https://github.com/ne75/llvm-propeller2.git 57c5b98e73e5b371cb637ff361238791675de5eb)"
	.section	".note.GNU-stack","",@progbits

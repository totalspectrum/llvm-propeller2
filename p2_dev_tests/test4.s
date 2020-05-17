	.text
	.file	"test4.cpp"
	.globl	_Z4testv                ' -- Begin function _Z4testv
	.type	_Z4testv,@function
_Z4testv:                               ' @_Z4testv
' %bb.0:                                ' %entry
			mov r0, sp
			add r0, #8
			wrlong #0, r0
			mov r0, sp
			add r0, #4
			wrlong #3, r0
			mov r0, sp
			add r0, #0
			wrlong #0, r0
.LBB0_1:                                ' %for.cond
                                        ' =>This Inner Loop Header: Depth=1
			mov r0, sp
			add r0, #0
			rdlong r0, r0
			cmpr r0, #24	wcz
	if_c	jmp #.LBB0_6
' %bb.2:                                ' %for.body
                                        '   in Loop: Header=BB0_1 Depth=1
			mov r0, sp
			add r0, #0
			rdlong r0, r0
			mov r1, sp
			add r1, #4
			rdlong r1, r1
			qdiv r0, r1
			getqy r0
			cmp r0, #0	wcz
	if_z	jmp #.LBB0_4
' %bb.3:                                ' %if.then
                                        '   in Loop: Header=BB0_1 Depth=1
			mov r0, sp
			add r0, #0
			rdlong r0, r0
			mov r1, sp
			add r1, #4
			rdlong r1, r1
			sub r0, r1
			mov r1, sp
			add r1, #8
			wrlong r0, r1
.LBB0_4:                                ' %if.end
                                        '   in Loop: Header=BB0_1 Depth=1
' %bb.5:                                ' %for.inc
                                        '   in Loop: Header=BB0_1 Depth=1
			mov r0, sp
			add r0, #0
			rdlong r1, r0
			add r1, #1
			wrlong r1, r0
			jmp #.LBB0_1
.LBB0_6:                                ' %for.end
			ret
.Lfunc_end0:
	.size	_Z4testv, .Lfunc_end0-_Z4testv
                                        ' -- End function
	.ident	"clang version 11.0.0 (https://github.com/ne75/llvm-propeller2.git dca2f02eafd03e587263c53711cd9c9e2e8f9001)"
	.section	".note.GNU-stack","",@progbits

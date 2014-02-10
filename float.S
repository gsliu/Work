	.file	"float.c"
	.comm	f1,4,4
	.comm	f2,4,4
	.comm	f3,4,4
	.text
.globl main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$0x3f8e353f, %eax
	movl	%eax, f1(%rip)
	movl	$0x400e353f, %eax
	movl	%eax, f2(%rip)
	movss	f1(%rip), %xmm1
	movss	f2(%rip), %xmm0
	addss	%xmm1, %xmm0
	movss	%xmm0, f3(%rip)
	movss	f3(%rip), %xmm1
	movss	.LC2(%rip), %xmm0
	mulss	%xmm1, %xmm0
	movss	%xmm0, f3(%rip)
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.section	.rodata
	.align 4
.LC2:
	.long	1082130432
	.ident	"GCC: (GNU) 4.4.7 20120313 (Red Hat 4.4.7-4)"
	.section	.note.GNU-stack,"",@progbits

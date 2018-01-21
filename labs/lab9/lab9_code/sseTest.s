	.file	"sseTest.c"
	.intel_syntax noprefix
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC7:
	.string	"|%g %g| * |%g %g| = |%g %g|\n"
.LC8:
	.string	"|%g %g|   |%g %g|   |%g %g|\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4,,15
	.globl	main
	.type	main, @function
main:
.LFB697:
	.cfi_startproc
	sub	rsp, 88
	.cfi_def_cfa_offset 96
	xorpd	xmm6, xmm6
	movsd	xmm0, QWORD PTR .LC0[rip]
	mov	esi, OFFSET FLAT:.LC7
	movsd	xmm7, QWORD PTR .LC1[rip]
	mov	edi, 1
	movsd	QWORD PTR [rsp+16], xmm0
	mov	eax, 6
	movsd	QWORD PTR [rsp+24], xmm7
	movsd	QWORD PTR [rsp+48], xmm6
	movsd	QWORD PTR [rsp+56], xmm6
	movsd	QWORD PTR [rsp+64], xmm6
	movsd	QWORD PTR [rsp+72], xmm6
	movsd	xmm1, QWORD PTR .LC2[rip]
	movsd	xmm7, QWORD PTR .LC3[rip]
	movupd	xmm2, XMMWORD PTR [rsp+16]
	movapd	xmm5, XMMWORD PTR .LC5[rip]
	mulpd	xmm5, xmm2
	movsd	QWORD PTR [rsp+32], xmm1
	movupd	xmm4, XMMWORD PTR [rsp+48]
	movsd	QWORD PTR [rsp+40], xmm7
	movupd	xmm3, XMMWORD PTR [rsp+64]
	addpd	xmm4, xmm5
	xorpd	xmm5, xmm5
	movsd	QWORD PTR [rsp+8], xmm6
	mulpd	xmm2, xmm5
	addpd	xmm3, xmm2
	movupd	xmm2, XMMWORD PTR [rsp+32]
	mulpd	xmm5, xmm2
	mulpd	xmm2, XMMWORD PTR .LC6[rip]
	addpd	xmm4, xmm5
	addpd	xmm3, xmm2
	movupd	XMMWORD PTR [rsp+48], xmm4
	movapd	xmm2, xmm1
	movupd	XMMWORD PTR [rsp+64], xmm3
	movapd	xmm3, xmm6
	movsd	xmm5, QWORD PTR [rsp+64]
	movsd	xmm4, QWORD PTR [rsp+48]
	call	__printf_chk
	movsd	xmm6, QWORD PTR [rsp+8]
	mov	esi, OFFSET FLAT:.LC8
	movsd	xmm5, QWORD PTR [rsp+72]
	mov	edi, 1
	movsd	xmm4, QWORD PTR [rsp+56]
	mov	eax, 6
	movsd	xmm1, QWORD PTR [rsp+40]
	movsd	xmm0, QWORD PTR [rsp+24]
	movapd	xmm2, xmm6
	movsd	xmm3, QWORD PTR .LC1[rip]
	call	__printf_chk
	xor	eax, eax
	add	rsp, 88
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE697:
	.size	main, .-main
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC0:
	.long	0
	.long	1072693248
	.align 8
.LC1:
	.long	0
	.long	1073741824
	.align 8
.LC2:
	.long	0
	.long	1074266112
	.align 8
.LC3:
	.long	0
	.long	1074790400
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC5:
	.long	0
	.long	1074266112
	.long	0
	.long	1074266112
	.align 16
.LC6:
	.long	0
	.long	1073741824
	.long	0
	.long	1073741824
	.ident	"GCC: (Ubuntu 4.8.4-2ubuntu1~14.04) 4.8.4"
	.section	.note.GNU-stack,"",@progbits

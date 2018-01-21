	.file	1 "lab3_ex3.c"
	.section .mdebug.eabi32
	.previous
	.section .gcc_compiled_long32
	.previous
	.gnu_attribute 4, 1
	.text
	.align	2
	.globl	main
	.set	nomips16
	.ent	main
	.type	main, @function
main:
	.frame	$sp,0,$31		# vars= 0, regs= 0/0, args= 0, gp= 0
	.mask	0x00000000,0
	.fmask	0x00000000,0
	lui	$6,%hi(source)
	lw	$3,%lo(source)($6)
	beq	$3,$0,$L2
	lui	$7,%hi(dest)
	addiu	$7,$7,%lo(dest)
	addiu	$6,$6,%lo(source)
	move	$2,$0
$L3:
	addu	$5,$7,$2
	addu	$4,$6,$2
	sw	$3,0($5)
	lw	$3,4($4)
	addiu	$2,$2,4
	bne	$3,$0,$L3
$L2:
	move	$2,$0
	j	$31
	.end	main
	.size	main, .-main
	.globl	source
	.data
	.align	2
	.type	source, @object
	.size	source, 28
source:
	.word	3
	.word	1
	.word	4
	.word	1
	.word	5
	.word	9
	.word	0

	.comm	dest,40,4
	.ident	"GCC: (GNU) 4.4.1"

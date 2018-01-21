# Just like cache.s, but does something interesting with the index calculation.
# See pseudocode below:
#
# PSEUDOCODE:
#    for (k = 0; k < repcount; k++) {		// repeat repcount times
#      /* Step through the selected array segment with the given step size. */
#      for (index = 0; index < arraysize; index += stepsize) {
#	 int offset = random number in range [0, stepsize)
#        array[index+offset] = 0;
#      }
#    }

	.data
array:	.space	2048		# max array size specified in BYTES (DO NOT CHANGE)
	
	.text
main:	li	$a0, 256	# array size in BYTES (power of 2 < array size)
	li	$a1, 8		# step size  (power of 2 > 0)
	li	$a2, 2		# rep count
	li	$a3, 1		# DO NOT CHANGE
	
	jal	accessWords	# lw/sw
	#jal	accessBytes	# lb/sb
	
	li	$v0,10		# exit
	syscall
	
	
# SUMMARY OF REGISTER USE:
#  $a0 = array size in bytes
#  $a1 = step size
#  $a2 = number of times to repeat
#  $a3 = 0 (W) / 1 (RW)
#  $s0 = moving array ptr
#  $s1 = array limit (ptr)
	
accessWords:
	la	$s0, array		# ptr to array
	addu	$s1, $s0, $a0		# hardcode array limit (ptr)
	sll	$t1, $a1, 2		# multiply stepsize by 4 because WORDS
wordLoop:
	# backup $a0, $a1, $v0 for random int syscall
	move 	$s6, $a0
	move 	$s7, $a1
	move 	$s5, $v0
	# prep for random int syscall
	addiu 	$a0, $0, 100 		# seed for random number generator, you may ignore this
	addiu 	$a1, $a1, 0 		# included for clarity; $a1 is already step size, our upper bound
	addiu 	$v0, $0, 42 		# syscall 42 is random int range
	syscall
	sll 	$a0, $a0, 2		# multiply random offset by 4 because WORDS
	addu 	$s4, $s0, $a0		# add randomly generated offset to moving array ptr
	sw	$0,  0($s4)		# array[(index+offset)/4] = 0
	# undo backups for random int syscall
	move 	$a0, $s6
	move 	$a1, $s7
	move 	$v0, $s5
	
wordCheck:
	addu	$s0, $s0, $t1		# increment ptr
	blt	$s0,$s1,wordLoop	# inner loop done?
	
	addi	$a2, $a2, -1
	bgtz	$a2, accessWords	# outer loop done?
	jr	$ra
	
	
accessBytes:
	la	$s0, array		# ptr to array
	addu	$s1, $s0, $a0		# hardcode array limit (ptr)
byteLoop:
	beq	$a3, $0,  byteZero

	lbu	$t0, 0($s0)		# array[index]++
	addi	$t0, $t0, 1
	sb	$t0, 0($s0)
	j	byteCheck
	
byteZero:	
	sb	$0,  0($s0)		# array[index] = 0
	
byteCheck:
	addu	$s0, $s0, $a1		# increment ptr
	blt	$s0,$s1,byteLoop	# inner loop done?
	
	addi	$a2, $a2, -1
	bgtz	$a2, accessBytes	# outer loop done?
	jr	$ra

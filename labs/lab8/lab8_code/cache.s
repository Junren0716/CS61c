# This program accesses an array in ways that provide data about the cache parameters.
# Coupled with the Data Cache Simulator and Memory Reference Visualization to help students
#    understand how the cache parameters affect cache performance.
#
# PSEUDOCODE:
#    for (k = 0; k < repcount; k++) {		// repeat repcount times
#      /* Step through the selected array segment with the given step size. */
#      for (index = 0; index < arraysize; index += stepsize) {
#        if(option==0)
#          array[index] = 0;			// Option 0: One cache access - write
#        else
#          array[index] = array[index] + 1;	// Option 1: Two cache accesses - read AND write
#      }
#    }

	.data
array:	.space	2048		# max array size specified in BYTES (DO NOT CHANGE)
	
	.text
##################################################################################################
# You MAY change the code below this section
main:	li	$a0, 256		# array size in BYTES (power of 2 < array size)
	li	$a1, 2		# step size  (power of 2 > 0)
	li	$a2, 1		# rep count  (int > 0)
	li	$a3, 1		# 0 - option 0, 1 - option 1
# You MAY change the code above this section
##################################################################################################

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
	beq	$a3, $0,  wordZero

	lw	$t0, 0($s0)		# array[index/4]++
	addi	$t0, $t0, 1
	sw	$t0, 0($s0)
	j	wordCheck
	
wordZero:	
	sw	$0,  0($s0)		# array[index/4] = 0
	
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

# Test function over here:
	addiu $a0, $0, 0xABC		# Load base addr
	addiu $a1, $0, 10		# Load length
	jal myFunc			# Call function
	li $v0, 0xABCDE

myFunc:	addiu $t0, $0, 0		# Init counter
startLoop:	beq $t0, $a1, endLoop		# Check if end condition reached
	addu $t1, $a0, $t0		# Calculate offset address
	lb $t2, 0($t1)		# Load value
	
	lbu $t3, -3($s2)
	addiu $t2, $t2, 1		# Increment by one
	or $a0, $a1, $a3
	li $t0, 3
	slt $a2, $t1, $t0
	sltu $a2, $t1, $t0
	sll $t3, $t2, 31
	
random:	ori $t3, $t2, 0x123
	lui $t3, 532
	sb $t2, 0($t1)
	sw $t2, -32768($t1)
	lw $t3, 32767($t1)
	blt $t3, $t2, myFunc
	
	addiu $t1, $t1, 1		# Increment counter
	j startLoop
endLoop:	jr $ra
	bne $t3, $a0, myFunc
	
	

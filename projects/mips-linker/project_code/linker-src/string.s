# CS 61C Summer 2015 Project 2-2 
# string.s

#==============================================================================
#                              Project 2-2 Part 1
#                               String README
#==============================================================================
# In this file you will be implementing some utilities for manipulating strings.
# The functions you need to implement are:
#  - strlen()
#  - strncpy()
#  - copy_of_str()
# Test cases are in linker-tests/test_string.s
#==============================================================================

.data
newline:	.asciiz "\n"
tab:	.asciiz "\t"

.text
#------------------------------------------------------------------------------
# function strlen()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string input
#
# Returns: the length of the string
#------------------------------------------------------------------------------

# Equivalent C code:
# 
# int sum = 0;		// (sum = $t0)
# char* x = "hi";	// (x = $a0)
# while (x != NULL) {
# 	sum += 1;
# 	x++;
# }
# return sum;
# 
strlen:
	# YOUR CODE HERE
	add $t0, $0, $0
	lb $t1, 0($a0)		# $a0 is the pointer, $t0 is the dereferenced value
	beq $t1, $0, rtn_sum
next_char:
	addi $t0, $t0, 1
	addi $a0, $a0, 1
	lb $t1, 0($a0)
	bne $t1, $0, next_char
rtn_sum:
	add $v0, $t0, $0
	jr $ra

#------------------------------------------------------------------------------
# function strncpy()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = pointer to destination array
#  $a1 = source string
#  $a2 = number of characters to copy
#
# Returns: the destination array
#------------------------------------------------------------------------------

# Equivalent C code:
# 
# int i = 0;
# for (; i < n; i++) {
# 	*b = *a;
# 	a++;
# 	b++;
# }
# return b;
# 
strncpy:
	# YOUR CODE HERE
	add $t0, $0, $0
	add $t3, $a1, $0
	add $t4, $a0, $0
	slt $t1, $t0, $a2
	beq $t1, $0, final
n_times:
	lb $t5, 0($t3)
	sb $t5, 0($t4)
	addi $t3, $t3, 1
	addi $t4, $t4, 1
	addi $t0, $t0, 1
	slt $t1, $t0, $a2
	bne $t1, $0, n_times
final:
	move $v0, $a0
	jr $ra

#------------------------------------------------------------------------------
# function copy_of_str()
#------------------------------------------------------------------------------
# Creates a copy of a string. You will need to use sbrk (syscall 9) to allocate
# space for the string. strlen() and strncpy() will be helpful for this function.
# In MARS, to malloc memory use the sbrk syscall (syscall 9). See help for details.
#
# Arguments:
#   $a0 = string to copy
#
# Returns: pointer to the copy of the string
#------------------------------------------------------------------------------

# Equivalent C code:
# 
# char* b = malloc(strlen(a) + 1);
# b = strncpy(b, a, strlen(a))
# return b;
# 
copy_of_str:
	# YOUR CODE HERE
	# To call others functions, save $a0 and $ra value to stack
	addiu $sp, $sp, -12
	sw $ra, 0($sp)
	sw $a0, 4($sp)
	sw $s0, 8($sp)		# Need to store the value of the previous call to copy_of_str
	
	# Preserve the value of the string to be copied before calling strlen
	add $s0, $a0, $0	# S-regs preserved along function calls
	jal strlen
	
	add $a1, $s0, $0	# Set string to be copied as argument for strncpy
	add $a2, $v0, $0	# Set the length of the string for call to strncpy
	
	# Allocate memory for the empty new string that will be returned
	add $a0, $v0, $0	# Tells syscall how many bytes needs to be allocated on heap
	addiu $v0, $0, 9	# Tells syscall to allocate memory, then set address to $v0
	syscall			# More info: http://courses.missouristate.edu/KenVollmar/Mars/Help/SyscallHelp.html
	add $a0, $v0, $0
	jal strncpy
	
	# Return all allocated memory to the stack
	lw $s0, 8($sp)		# Failing basic test because this value was not saved properly
	lw $a0, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 12
	jr $ra
	

###############################################################################
#                 DO NOT MODIFY ANYTHING BELOW THIS POINT                       
###############################################################################

#------------------------------------------------------------------------------
# function streq() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string 1
#  $a1 = string 2
#
# Returns: 0 if string 1 and string 2 are equal, -1 if they are not equal
#------------------------------------------------------------------------------
streq:
	beq $a0, $0, streq_false	# Begin streq()
	beq $a1, $0, streq_false
streq_loop:
	lb $t0, 0($a0)
	lb $t1, 0($a1)
	addiu $a0, $a0, 1
	addiu $a1, $a1, 1
	bne $t0, $t1, streq_false
	beq $t0, $0, streq_true
	j streq_loop
streq_true:
	li $v0, 0
	jr $ra
streq_false:
	li $v0, -1
	jr $ra			# End streq()

#------------------------------------------------------------------------------
# function dec_to_str() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Convert a number to its unsigned decimal integer string representation, eg.
# 35 => "35", 1024 => "1024". 
#
# Arguments:
#  $a0 = int to write
#  $a1 = character buffer to write into
#
# Returns: the number of digits written
#------------------------------------------------------------------------------
dec_to_str:
	li $t0, 10			# Begin dec_to_str()
	li $v0, 0
dec_to_str_largest_divisor:
	div $a0, $t0
	mflo $t1		# Quotient
	beq $t1, $0, dec_to_str_next
	mul $t0, $t0, 10
	j dec_to_str_largest_divisor
dec_to_str_next:
	mfhi $t2		# Remainder
dec_to_str_write:
	div $t0, $t0, 10	# Largest divisible amount
	div $t2, $t0
	mflo $t3		# extract digit to write
	addiu $t3, $t3, 48	# convert num -> ASCII
	sb $t3, 0($a1)
	addiu $a1, $a1, 1
	addiu $v0, $v0, 1
	mfhi $t2		# setup for next round
	bne $t2, $0, dec_to_str_write
	jr $ra			# End dec_to_str()

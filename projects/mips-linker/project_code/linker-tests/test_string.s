# CS 61C Summer 2015 Project 2-2 
# linker-tests/test_string.s

#==============================================================================
#                              string.s Test Cases
#==============================================================================

.include "../linker-src/string.s"
.include "test_core.s"

#-------------------------------------------
# Test Data - Feel free to add your own
#-------------------------------------------
.data
test_str1:		.asciiz "a9bw enijn webb"
test_str2:		.asciiz "a9bw en"
test_str3:		.asciiz ""

.globl main
.text
#-------------------------------------------
# Test driver
#-------------------------------------------
main:
	print_str(test_header_name)

	print_newline()
	jal test_strlen
	
	print_newline()
	jal test_strncpy
	
	print_newline()
	jal test_copy_of_str
	
	li $v0, 10
	syscall

#-------------------------------------------
# Tests strlen()
#-------------------------------------------
test_strlen:
	addiu $sp, $sp, -4
	sw $ra, 0($sp)
	print_str(test_strlen_name)
	
	la $a0, test_str1
	jal strlen
	check_uint_equals($v0, 15)
	
	la $a0, test_str3
	jal strlen
	check_uint_equals($v0, 0)
	
	la $a0, test_str2
	jal strlen
	check_uint_equals($v0, 7)
	
	lw $ra, 0($sp)
	addiu $sp, $sp, 4
	jr $ra

#-------------------------------------------
# Tests strncpy()
#-------------------------------------------
test_strncpy:
	addiu $sp, $sp, -4
	sw $ra, 0($sp)
	print_str(test_strncpy_name)
	
	la $a0, test_buffer
	la $a1, test_str1
	li $a2, 16		# note: len + 1
	jal strncpy
	check_str_equals($v0, test_str1)
	
	la $a0, test_buffer
	la $a1, test_str2
	li $a2, 8		# note: len + 1
	jal strncpy
	check_str_equals($v0, test_str2)
	
	lw $ra, 0($sp)
	addiu $sp, $sp, 4
	jr $ra

#-------------------------------------------
# Tests copy_of_str()
#-------------------------------------------
test_copy_of_str:
	addiu $sp, $sp, -12
	sw $s0, 8($sp)
	sw $s1, 4($sp)
	sw $ra, 0($sp)
	print_str(test_copy_of_str_name)
	
	la $a0, test_str1
	jal copy_of_str
	move $s0, $v0
	
	la $a0, test_str2
	jal copy_of_str
	move $s1, $v0
	
	check_str_equals($s0, test_str1)
	check_str_equals($s1, test_str2)
	
	lw $s0, 8($sp)
	lw $s1, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 4
	jr $ra

.data
test_header_name:	.asciiz "Running util tests:\n"

test_strlen_name:	.asciiz "Testing strlen():\n"
test_strncpy_name:	.asciiz "Testing strncpy():\n"
test_copy_of_str_name:	.asciiz "Testing copy_of_str():\n"

# 20 byte buffer x 5 = 100 byte buffer
test_buffer:	.word 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF
		.word 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF
		.word 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF
		.word 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF
		.word 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF
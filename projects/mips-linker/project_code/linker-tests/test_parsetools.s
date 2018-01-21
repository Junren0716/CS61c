# CS 61C Summer 2015 Project 2-2 
# linker-tests/test_parsetools.s

#==============================================================================
#                            parsetools.s Test Cases
#==============================================================================

.include "../linker-src/parsetools.s"
.include "test_core.s"
.include "../linker-src/string.s"

#-------------------------------------------
# Test Data - Feel free to add your own
#-------------------------------------------
.data
hex_str1:	.asciiz "abcdef12\n"
hex_str2:	.asciiz "00034532\n"

.globl main
.text
#-------------------------------------------
# Test driver
#-------------------------------------------
main:
	print_str(test_header_name)
	
	print_newline()
	jal test_hex_to_str
	
	li $v0, 10
	syscall
	
#-------------------------------------------
# Tests hex_to_str() from parsetools.s
#-------------------------------------------
test_hex_to_str:
	addiu $sp, $sp, -4
	sw $ra, 0($sp)
	print_str(test_hex_to_str_name)
	
	li $a0, 2882400018
	la $a1, test_buffer
	jal hex_to_str
	la $a0, test_buffer
	check_str_equals($a0, hex_str1)
	
	li $a0, 214322
	la $a1, test_buffer
	jal hex_to_str
	la $a0, test_buffer
	check_str_equals($a0, hex_str2)
	
	lw $ra, 0($sp)
	addiu $sp, $sp, 4
	jr $ra

.data
test_header_name:	.asciiz "Running test parsetools:\n"

test_hex_to_str_name:	.asciiz "Testing hex_to_str():\n"

test_buffer:	.space 10
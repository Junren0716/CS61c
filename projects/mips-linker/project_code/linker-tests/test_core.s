# CS 61C Summer 2015 Project 2-2 
# linker-tests/test_core.s
#
# Contains core testing utilities

.data
nl_separator:	.asciiz "\n"	# to avoid naming conflicts

tc_actual_str:	.asciiz "actual: "
tc_expected_str:	.asciiz " expected: "
tc_expected_str2:	.asciiz "\nexpected: "
tc_testpass_str:	.asciiz " ...test passes\n"
tc_testfail_str:	.asciiz " ...test FAILS\n"

#==================================
# Macros - EASY TO INTRODUCE BUGS WITH
#  Be careful when using these
#==================================

#-------------------------------------------
# Prints a newline 
# CLOBBERS: $a0 and $v0
#-------------------------------------------
.macro print_newline()
	la $a0, nl_separator
	li $v0, 4
	syscall
.end_macro

#-------------------------------------------
# Prints the string
#   %test_reg = label containing address of string
# CLOBBERS: $a0 and $v0
#-------------------------------------------
.macro print_str(%str_label)
	la $a0, %str_label
	li $v0, 4
	syscall
.end_macro

#-------------------------------------------
# Checks whether the actual result matches the expected result
# Treats arguments as signed ints
#   %actual_reg = register containing actual result
#   %expected_val = expected value of register
# CLOBBERS: $a0, $v0, and $t0
#-------------------------------------------
.macro check_int_equals(%actual_reg, %expected_val)
	move $t0, %actual_reg
	print_str(tc_actual_str)
	move $a0, $t0
	li $v0, 1
	syscall
	print_str(tc_expected_str)
	li $a0, %expected_val
	li $v0, 1
	syscall
	bne $t0, $a0, notequal
	print_str(tc_testpass_str)
	j end
notequal:
	print_str(tc_testfail_str)
end:	
.end_macro

#-------------------------------------------
# Checks whether the actual result matches the expected result
# Treats arguments as unsigned ints
#   %actual_reg = register containing actual result
#   %expected_val = expected value of register
# CLOBBERS: $a0, $v0, and $t0
#-------------------------------------------
.macro check_uint_equals(%actual_reg, %expected_val)
	move $t0, %actual_reg
	print_str(tc_actual_str)
	move $a0, $t0
	li $v0, 36
	syscall
	print_str(tc_expected_str)
	li $a0, %expected_val
	li $v0, 36
	syscall
	bne $t0, $a0, notequal
	print_str(tc_testpass_str)
	j end
notequal:
	print_str(tc_testfail_str)
end:
.end_macro

#-------------------------------------------
# Checks whether the actual result matches the expected result
# Arguments should be strings
#   %actual_reg = register containing actual result
#   %expected_val = label to expected string
# CLOBBERS: $a0, $v0, and $t0
#-------------------------------------------
.macro check_str_equals(%actual_reg, %expected_val)
	move $t0, %actual_reg
	print_str(tc_actual_str)
	move $a0, $t0
	li $v0, 4
	syscall
	print_str(tc_expected_str2)
	print_str(%expected_val)
	print_newline()
	move $a0, $t0
	la $a1, %expected_val
	jal streq
	bne $v0, $0, notequal
	print_str(tc_testpass_str)
	j end
notequal:
	print_str(tc_testfail_str)
end:
	print_newline()
.end_macro

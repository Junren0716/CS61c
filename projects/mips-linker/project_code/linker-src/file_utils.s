# CS 61C Summer 2015 Project 2-2 
# file_utils.s
#
# Utilities for opening and closing files

.data
cannot_open_files: .asciiz "Error opening files. Exiting program.\n"

.text
#------------------------------------------------------------------------------
# function open_files()
#------------------------------------------------------------------------------
# Given an array of filenames, opens each file. If a file cannot be opened, exits
# the program.
#
# Arguments:
#  $a0 = an array filename strings
#  $a1 = length of the array
#
# Returns: an array of file pointers corresponding to the filenames
#------------------------------------------------------------------------------
open_files:
	addiu $sp, $sp, -12			# Begin open_files()
	sw $s0, 8($sp)
	sw $s1, 4($sp)
	sw $ra, 0($sp)
	move $s0, $a0		# $s0 = file array
	move $s1, $a1		# $s1 = number of items
	# Alloc space for array:
	sll $a0, $a1, 2		# $a0 = # bytes needed for array
	li $v0, 9
	syscall			
	move $v1, $v0		# $v1 = array of file ptrs
	# Setup flags
	li $a1, 0			# flag = read only
	li $t0, 0			# $t0 = counter
open_files_begin:
	beq $t0, $s1, open_files_done
	sll $t1, $t0, 2		# offset in bytes
	addu $t2, $s0, $t1		# compute current filename loc
	lw $a0, 0($t2)
	li $v0, 13
	syscall			# open input file
	blt $v0, 0, open_files_err
	addu $t3, $v1, $t1		# compute current file ptr loc
	sw $v0, 0($t3)		# store file descriptor in array
	addiu $t0, $t0, 1
	j open_files_begin
open_files_done:
	move $v0, $v1		# $v0 = file descriptors
	lw $s0, 8($sp)
	lw $s1, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 12
	jr $ra
open_files_err:
	# Error file input failed
	move $a0, $v1		# $a0 = file descriptor array
	move $a1, $t0		# $a1 = number of files currently open
	jal close_files
	la $a0, cannot_open_files
	li $v0, 4
	syscall
	li $a0, 1
	li $v0, 17			# exit program
	syscall				# End open_files()

#------------------------------------------------------------------------------
# function close_files()
#------------------------------------------------------------------------------
# Given an array of file pointers, closes all files
#
# Arguments:
#  $a0 = an array of file pointers
#  $a1 = length of the array
#
# Returns: none
#------------------------------------------------------------------------------
close_files:
	move $t0, $a0			# Begin open_files()
	sll $a1, $a1, 2
	li $t1, 0			# $t1 = counter
cf_begin:	slt $t2, $t1, $a1		# $a1 = loop end condition (in bytes)
	beq $t2, $0, cf_end
	addu $t3, $t0, $t1
	lw $a0, 0($t3)		# load file descriptor
	li $v0, 16
	syscall			# close file
	addiu $t1, $t1, 4
	j cf_begin
cf_end:	jr $ra				# End close_files()
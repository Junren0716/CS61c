# CS 61C Summer 2015 Project 2-2 
# linker.s

#==============================================================================
#                              Project 2-2 Part 4
#                               Linker README
#==============================================================================
# In this file you will be implementing the write_machine_code() function, 
# which will copy machine code from an object file into the final executable.
# 
# Please read the README in linker_utils.s before proceeding. The function you
# are implementing performs the actual write operation, copying the .text
# section from an object file to the final executable. 
#
# When you complete this function, your linker should be able to link object
# files!
#==============================================================================

.include "linker_utils.s"
.include "file_utils.s"

.data
base_addr:		.word 0x00400000
hex_buffer:		.space 10

.globl main
.text
#------------------------------------------------------------------------------
# function write_machine_code()
#------------------------------------------------------------------------------
# Write the contents of the .text section of the input file to the output file.
# Fill in the blanks labeled YOUR_INSTRUCTIONS_HERE according to the descriptions
# above. Each blank may require one or more MIPS instructions, but no blank 
# requires a lot of instructions (the maximum we used for one blank is 6).
# There are seven such blanks in total.
#
# Note: You may need to store additional registers onto the stack. You should
# change stack poineter appropriately.
#
# Arguments:
#  $a0 = The output file pointer
#  $a1 = The input file pointer
#  $a2 = The symbol table, which you can pass into relocate_inst() if needed
#  $a3 = The relocation table, which you can pass into relocate_inst() if needed
#
# Returns: 0 on success and -1 on fail. 
#------------------------------------------------------------------------------
write_machine_code:
	# You may need to save additional items onto the stack. Feel free to
	# change this part.
	addiu $sp, $sp, -32
	sw $s0, 20($sp)
	sw $s1, 16($sp)
	sw $s2, 12($sp)
	sw $s3, 8($sp)
	sw $s4, 4($sp)
	sw $s5, 24($sp)
	sw $s6, 28($sp)
	sw $ra, 0($sp)
	# We'll save the arguments since we are making function calls.
	move $s0, $a0			# $s0 = output file ptr
	move $s1, $a1			# $s1 = input file ptr
	move $s2, $a2			# $s2 = symbol table
	move $s3, $a3			# $s3 = relocation table
	# We find the start of the .text section by reading each line and 
	# checking to see if we find ".text".
write_machine_code_find_text:	
	move $a0, $s1
	jal readline
	blt $v0, $0, write_machine_code_error	# Invalid line reached
	move $a0, $v1
	la $a1, textLabel
	jal streq
	bne $v0, $0, write_machine_code_find_text
	# Now, we are at the start of the text section (ie. next time we call
	# readline(), we will read in the first instruction). Fill in the
	# blanks below with the correct instruction(s).
	
	# 1. Initialize the byte offset to zero. We will need this for any instructions
	# that require relocation:
	# YOUR_INSTRUCTIONS_HERE
	addiu $s5, $0, 0	# Save the PC value (initially 0) into $s5 ($s0 - $s4 already used)

write_machine_code_next_inst:
	# 2. Call readline() while passing in the correct arguments:
	# YOUR_INSTRUCTIONS_HERE
	add $a0, $s1, $0	# Same procedure as the given code above (just go to next line)
	jal readline

	# Check whether readline() returned an error.
	blt $v0, $0, write_machine_code_error
	# Store whether end-of-file was reached into $s4. We will check this later.
	move $s4, $v0			
	# We check whether there are any characters besides the NULL terminator below.
	lbu $t0, 0($v1)
	beq $t0, $0, write_machine_code_done
	
	# 3. Looks like there is another instruction. Call parse_int() with base=16
	# to convert the instruction into a number, and store it into a register:
	# YOUR_INSTRUCTIONS_HERE
	add $a0, $v1, $0	# Number to parse is the instruction
	addiu $a1, $0, 16	# Base 16
	jal parse_int
	addiu $s6, $v0, 0	# Store the value to $s6, use s regs to save value across calls

	# 4. Check if the instruction needs relocation. If it does not, branch to
	# the label write_machine_code_to_file:
	# YOUR_INSTRUCTIONS_HERE
	add $a0, $v0, $0
	jal inst_needs_relocation
	add $t0, $v0, $0
	beq $t0, $0, write_machine_code_to_file
	
	# 5. Here we handle relocation. Call relocate_inst() with the appropriate
	# arguments, and store the relocated instruction in the appropriate register:
	# YOUR_INSTRUCTIONS_HERE
	add $a0, $s6, $0	# Instruction that needs relocating
	add $a1, $s5, $0	# Grab the byte offset
	add $a2, $s2, $0	# Gets the original value
	add $a3, $s3, $0	# Picks up the original value
	jal relocate_inst
	add $s6, $v0, $0

write_machine_code_to_file:
	# 6. Write the instruction into a string buffer via hex_to_str():
	# YOUR_INSTRUCTIONS_HERE 
	add $a0, $s6, $0
	la $a1 hex_buffer	# hex_buffer can be more than 16 bits, so load it as address
	jal hex_to_str
	add $s6, $v0, $0

	# 7. Increment the byte offset by the appropriate amount:
	# YOUR_INSTRUCTIONS_HERE
	addiu $s5, $s5, 4

	# Here, we use the write to file syscall. WE specify the output file as $a0.
	move $a0, $s0
	# Set $a1 = the buffer that we will write.
	la $a1, hex_buffer
	# Set $a2 = number of bytes to write. 8 digits + newline = 9 bytes
	li $a2, 9			
	li $v0, 15
	syscall
	# Finally, we check whether end-of-file was reached. If not, keep going.
	bne $s4, $0, write_machine_code_next_inst
write_machine_code_done:
	li $v0, 0
	j write_machine_code_end
write_machine_code_error:
	li $v0, -1
write_machine_code_end:
	# Don't forget to change this part if you saved more items onto the stack!
	lw $s6, 28($sp)
	lw $s5, 24($sp)
	lw $s0, 20($sp)
	lw $s1, 16($sp)
	lw $s2, 12($sp)
	lw $s3, 8($sp)
	lw $s4, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 32
	jr $ra

###############################################################################
#                 DO NOT MODIFY ANYTHING BELOW THIS POINT                       
###############################################################################

#------------------------------------------------------------------------------
# function build_tables()
#------------------------------------------------------------------------------
# Build tables
# $a0 = input file arr, $a1 = input arr len, $a2 = global offset
# $v0 = symbol table, $v1 = reloc_data
#------------------------------------------------------------------------------
build_tables:
	addiu $sp, $sp, -32		# Begin build_tables()
	sw $s0, 28($sp)
	sw $s1, 24($sp)
	sw $s2, 20($sp)
	sw $s3, 16($sp)
	sw $s4, 12($sp)
	sw $s5, 8($sp)
	sw $s6, 4($sp)
	sw $ra, 0($sp)
	# Store arguments:
	move $s0, $a0	# $s0 = input file array
	move $s1, $a1	# $s1 = number of files
	move $s2, $a2	# $s2 = base offset
	# Open files:
	jal open_files
	move $s3, $v0	# $s3 = file descriptors
	# Alloc space for reloc_data
	sll $a0, $s1, 3	# 8 * (# files)
	li $v0, 9
	syscall
	move $s4, $v0	# $s4 = array of reloc_data
	# Loop through and build table:
	li $s5, 0		# $s5 = counter
	li $s6, 0		# $s6 = symbol table (empty)
build_tables_loop:
	beq $s5, $s1, build_tables_end
	
	sll $t0, $s5, 2	
	addu $t5, $s3, $t0	# $a0 = current file handle
	lw $a0, 0($t5)
	move $a1, $s6	# $a1 = current symobl table
	sll $t1, $t0, 1	
	addu $a2, $s4, $t1	# $a2 = current entry in reloc_data
	move $a3, $s2	# $a3 = current global offset
	jal fill_data
	blt $v0, $0, build_tables_error
	move $s6, $v1	# update symbol table
	sll $t1, $s5, 3	
	addu $t2, $s4, $t1	# current entry in reloc_data
	lw $t3, 0($t2)
	addu $s2, $s2, $t3	# update offset
	addiu $s5, $s5, 1	# increment count
	j build_tables_loop
build_tables_error:
	move $a0, $s3
	move $a1, $s1
	jal close_files
	la $a0, error_parsing
	li $v0, 4
	syscall
	li $a0, 1
	li $v0, 17		# exit on error
	syscall
build_tables_end:
	move $a0, $s3
	move $a1, $s1
	jal close_files
	# Set return values
	move $v0, $s6	# set symbol table
	move $v1, $s4	# set reloc_data
	# Stack stuff
	lw $s0, 28($sp)
	lw $s1, 24($sp)
	lw $s2, 20($sp)
	lw $s3, 16($sp)
	lw $s4, 12($sp)
	lw $s5, 8($sp)
	lw $s6, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 32
	jr $ra			# End build_tables()


#------------------------------------------------------------------------------
# function main()
#------------------------------------------------------------------------------
# argc = num args, argv = char**
#------------------------------------------------------------------------------
main:	bge $a0, 2, m_arg_ok
	# Error not enough arguments
	la $a0, error_args
	li $v0, 4
	syscall
	li $a0, 1	
	li $v0, 17
	syscall			# exit with error
m_arg_ok:	
	addiu $s0, $a0, -1		# $s0 = num inputs
	move $s1, $a1		# $s1 = array of filenames
	sll $t0, $s0, 2
	addu $t1, $s1, $t0		
	lw $s2, 0($t1)		# $s2 = output filename
	
	# Build symbol/reloc table:
	move $a0, $s1
	move $a1, $s0
	la $t0, base_addr
	lw $a2, 0($t0)
	jal build_tables
	move $s3, $v0		# $s3 = symbol table
	move $s4, $v1		# $s4 = reloc_data
	
	# Open input & output files for writing:
	move $a0, $s1
	move $a1, $s0
	jal open_files
	move $s1, $v0		# $s1 = array of file ptrs
	move $a0, $s2
	li $a1, 1
	li $v0, 13
	syscall			# open output file
	blt $v0, 0, m_fopen_error
	move $s2, $v0		# $s2 = output file descriptor

	# Begin writing:
	li $s5, 0
m_write_loop:
	beq $s5, $s0, m_write_done
	move $a0, $s2		# $a0 = output file
	sll $t0, $s5, 2		# sizeof(file_ptr) = 4
	addu $t1, $s1, $t0
	lw $a1, 0($t1)		# $a1 = input file
	move $a2, $s3		# $a2 = symbol table
	sll $t0, $s5, 3		# sizeof(reloc_data) = 8
	addu $t2, $s4, $t0
	lw $a3, 4($t2)		# $a3 = reloc table
	jal write_machine_code
	blt $v0, $0, m_write_error
	addiu $s5, $s5, 1
	j m_write_loop
m_write_done:
	move $a0, $s1
	move $a1, $s0
	jal close_files
	move $a0, $s2
	li $v0, 16
	syscall
	la $a0, link_success
	li $v0, 4
	syscall
	li $v0, 10
	syscall			# exit without errors
m_write_error:
	move $a0, $s1
	move $a1, $s0
	jal close_files
	move $a0, $s2
	li $v0, 16
	syscall
	la $a0, error_write
	li $v0, 4
	syscall
	li $a0, 1
	li $v0, 17
	syscall			# exit with errors
m_fopen_error:
	move $a0, $s1
	move $a1, $s0
	jal close_files
	la $a0, error_fopen
	li $v0, 4
	syscall
	li $a0, 1
	li $v0, 17
	syscall			# exit with errors

.data
error_args:		.asciiz "Error not enough arguments. Exiting.\n"
error_parsing:	.asciiz "Error parsing files. Exiting.\n"
error_fopen:	.asciiz "Error opening output file. Exiting.\n"
error_write:	.asciiz "Error during write. Exiting.\n"
link_success:	.asciiz "Operation completed successfully.\n"
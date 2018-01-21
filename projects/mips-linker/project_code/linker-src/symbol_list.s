# CS 61C Summer 2015 Project 2-2 
# symbol_list.s

#==============================================================================
#                              Project 2-2 Part 2
#                               SymbolList README
#==============================================================================
# In this file you will be implementing a linked list-based data structure for
# storing symbols. Its purpose is analogous to SymbolTable from Proj2-1, but to
# disinguish it we will call it SymbolList.
#
# Each node in the SymbolList contains a (addr, name) pair. An empty SymbolList
# is simply a pointer to NULL. As (addr, name) pairs are added to a SymbolList,
# new list nodes are dynamically allocated and appended to the front of the list. 
# However, as there is no free() in MARS, you do not need to write a free_list()
# function. You may use the functions in string.s during your implementation.
#
# You do not need to perform error checking on duplicate addr or name entries
# in SymbolList, nor do you need to do alignment checking. Don't worry about
# provided addresses being too big.
#
# If SymbolList were to be written in C, each list node would be declared as 
# follows:
#   typedef struct symbollist { 
#       int addr;
#       char* name;
#       struct symbollist* next; 
#   } SymbolList;
#
# You need to write addr_for_symbol() and add_to_list().
# You will find test cases in linker-tests/test_symbol_list.s
#==============================================================================

.include "string.s"

.text

#------------------------------------------------------------------------------
# function addr_for_symbol()
#------------------------------------------------------------------------------
# Iterates through the SymbolList and searches for an entry with the given name.
# If an entry is found, return that addr. Otherwise return -1.
#
# Arguments:
#  $a0 = pointer to a SymbolList (NULL indicates empty list)
#  $a1 = name to look for
#
# Returns:  address of symbol if found or -1 if not found
#------------------------------------------------------------------------------

# Equivalent C code:
# 
# while (*list != NULL) {
# 	if (list -> name == nme) {
# 		return list -> addr;
# 	}
#	list++;
# }
# return -1;
# 

addr_for_symbol:
	# YOUR CODE HERE
	# Make space on the stack to store the default values
	addiu $sp, $sp, -16
	sw $ra, 0($sp)
	sw $s0, 4($sp)
	sw $s1, 8($sp)
	sw $s2, 12($sp)
	add $s0, $0, $a0	# Allows us to keep original values in method calls
	add $s1, $0, $a1

add_sym_loop:
	# Follow procedure of symbol_for_addr to search SymbolList for the symbol
	# Check to see if the symbol is found in the list node
	beq $s0, $0, failure
	lw $a0, 4($s0)
	jal streq		# Checks if strings are the same (in string.s)
	add $a1, $s1, $0	# Streq modifies values of a1 and a0, so must reset them manually (debugged!)
	beq $v0, $0, success
	lw $s0, 8($s0)		# Increment to next element
	j add_sym_loop		# Check the rest till list is null
	
success:
	# If symbol is found, return the appropriate address
	lw $v0, 0($s0)
	j final_steps
	
failure:
	addiu $v0, $0, -1
		
final_steps:
	# Finally, free up all space allocated on the stack
	lw $s2, 12($sp)
	lw $s1, 8($sp)
	lw $s0, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 16
	jr $ra
	
#------------------------------------------------------------------------------
# function add_to_list()
#------------------------------------------------------------------------------
# Adds a (name, addr) pair to the FRONT of the list. You should call new_node()
# to create a new node. You do not need to perform error checking on duplicate 
# addr or name entries in SymbolList, nor do you need to do alignment checking.
#
# As with Project 2-1, you WILL need to create a copy of the string that was
# passed in. copy_of_str() from Part 1 will be useful. After new entry has been
# added to the list, return the new list.
#
# Arguments:
#   $a0 = ptr to list (may be NULL)
#   $a1 = address of symbol (integer)
#   $a2 = pointer to name of symbol (string)
#
# Returns: the new list
#------------------------------------------------------------------------------

# Equivalent C code:
# 
# SymbolList x = {$a1, copy_of_str($a2), $a0};
# return x;
# 

add_to_list:	
	# YOUR CODE HERE
	# First, account for necessary space on the stack
	addiu $sp, $sp, -16
	sw $ra, 0($sp)
	sw $s0, 4($sp)
	sw $s1, 8($sp)
	sw $s2, 12($sp)
	add $s0, $a0, $0	# Save value for later use (since $a0 used by copy_of_str)
	add $s1, $a1, $0	# Save value for later use
	
	
create_new_node:
	# Make a new node and add in all the appropriate attributes (front of the list)
	jal new_node
	add $s2, $v0, $0	# The new node is stored into reg $s2
	add $a0, $a2, $0
	jal copy_of_str
	add $s3, $v0, $0	# Reg $s3 stores the copy of the string
	sw $s1, 0($s2)		# Address of the new node
	sw $s3, 4($s2)		# Name of the new node
	sw $s0, 8($s2)		# Next node of the new node (NULL)
	add $v0, $s2, $0
	
end_goal:
	# Finally, free up all space on the stack
	lw $s2, 12($sp)
	lw $s1, 8($sp)
	lw $s0, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 16
	jr $ra

###############################################################################
#                 DO NOT MODIFY ANYTHING BELOW THIS POINT                       
###############################################################################

#------------------------------------------------------------------------------
# function symbol_for_addr()
#------------------------------------------------------------------------------
# Iterates through the SymbolList and searches for an entry with the given addr.
# If an entry is found, return a pointer to the name. Otherwise return NULL.
#
# Arguments:
#  $a0 = pointer to a SymbolList (NULL indicates empty list)
#  $a1 = addr to look for
#
# Returns: a pointer to the name if found or NULL if not found
#------------------------------------------------------------------------------
symbol_for_addr:
	beq $a0, $0, symbol_not_found	# Begin symbol_for_addr
	lw $t0, 0($a0)
	beq $t0, $a1, symbol_found
	lw $a0, 8($a0)
	j symbol_for_addr
symbol_found:
	lw $v0, 4($a0)
	jr $ra
symbol_not_found:
	li $v0, 0
	jr $ra			# End addr_for_symbol

#------------------------------------------------------------------------------
# function print_list() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = pointer to a SymbolList (NULL indicates empty list)
#  $a1 = print function
#  $a2 = file pointer
#------------------------------------------------------------------------------
print_list:
	addiu $sp, $sp, -16		# Begin print_list
	sw $s0, 12($sp)
	sw $s1, 8($sp)
	sw $s2, 4($sp)
	sw $ra, 0($sp)
	move $s0, $a0
	move $s1, $a1
	move $s2, $a2
print_list_loop:
	beq $s0, $0, print_list_end
	lw $a0, 0($s0)
	lw $a1, 4($s0)
	move $a2, $s2
	jalr $s1
	lw $s0, 8($s0)
	j print_list_loop
print_list_end:
	lw $s0, 12($sp)
	lw $s1, 8($sp)
	lw $s2, 4($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 16
	jr $ra			# End print_list	

#------------------------------------------------------------------------------
# function print_symbol() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Prints one symbol to standard output.
#
# Arguments:
#  $a0 = addr of symbol
#  $a1 = name of symbol
#  $a2 = file pointer (this argument is actually ignored)
#
# Returns: none
#------------------------------------------------------------------------------
print_symbol:
	li $v0, 36			# Begin print_symbol()
	syscall
	la $a0, tab
	li $v0, 4
	syscall
	move $a0, $a1
	syscall
	la $a0, newline
	syscall
	jr $ra			# End print_symbol()

#------------------------------------------------------------------------------
# function write_symbol() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Writes one symbol to the file specified
#
# Arguments:
#  $a0 = addr of symbol
#  $a1 = name of symbol
#  $a2 = file pointer
#
# Returns: none
#------------------------------------------------------------------------------
write_symbol:		
	addiu $sp, $sp, -20		# Begin write_symbol()
	sw $s0, 16($sp)
	sw $s1, 12($sp)
	sw $a1, 8($sp)
	sw $a2, 4($sp)
	sw $ra, 0($sp)
	
	la $s0, temp_buf
	
	move $a1, $s0
	jal dec_to_str	# write int
	move $s1, $v0	
	
	addu $a0, $s0, $s1
	addiu $s1, $s1, 1
	la $a1, tab
	li $a2, 1
	jal strncpy		# write tab
	
	lw $a0, 8($sp)
	jal strlen
	
	addu $a0, $s0, $s1
	addu $s1, $s1, $v0
	lw $a1, 8($sp)
	move $a2, $v0
	jal strncpy		# write string
	
	addu $a0, $s0, $s1
	addiu $s1, $s1, 1
	la $a1, newline
	li $a2, 1
	jal strncpy		# write newline
	
	lw $a0, 4($sp)	# file ptr
	move $a1, $s0	# buffer to write
	move $a2, $s1	# num chars to write
	li $v0, 15
	syscall		# write to file
	
	lw $s0, 16($sp)
	lw $s1, 12($sp)
	lw $ra, 0($sp)
	addiu $sp, $sp, 20
	jr $ra			# End write_symbol()
	
#------------------------------------------------------------------------------
# function new_node() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Creates a new uninitialized SymbolList node.
# Arguments: none
# Returns: pointer to a SymbolList node
#------------------------------------------------------------------------------
new_node:	
	li $a0, 12			# Begin new_node()
	li $v0, 9
	syscall
	jr $ra			# End new_node()
	
.data 
temp_buf:	.space 1024

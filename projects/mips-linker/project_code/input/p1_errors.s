# Things to catch

		addu $v0, $a0, $a1, $t0, $t1			# Extra arguments
label:
		or $a2, $a3, $t0
		jr $t1
3hello:											# Invalid labels
		slt $t2, $t2, $s0,						
		sltu $s1, $s2, $s3 sll $sp, $ra, 3		# Multiple instructions
		lui $t3, 123
label:	lb $t0, 0($s0)							# Multiple definition of label
		jal label
l1: l2: addiu $t3, $t1, 5						# Multiple labels on one line
		ori $t3, $t2, 0xABC
		
# Things to ignore
		addiu $t3, $99, 3							# invalid register
		ori $t1, $t0, 0xFFFFFFFF				# invalid immediate
		bne $t0, $t1, not_found					# nonexistant label
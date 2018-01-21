add $t0, $s0, $zero
add $t1, $s1, $zero
add $t2, $t0, $t1
add $t3, $t1, $t2
add $t4, $t2, $t3
add $t5, $t3, $t4
add $t6, $t4, $t5
add $t7, $t5, $t6
addi 	$a0, $t7, 0
li   	$v0, 1		# you will be asked about what the purpose of this line for syscall 
syscall			
li      $v0, 10		
syscall

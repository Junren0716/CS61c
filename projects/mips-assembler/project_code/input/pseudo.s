		li $v0, 10
label1:	li $a1, -6000
		
label2:
		li $a2, 80000
		li $a3, 0xB0BACAFE
		blt $t3, $a0, label1
		blt $sp, $v0, label2
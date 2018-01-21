label1:

label2:		beq $t0, $t1, label2 
			bne $t0, $t1, label1
label3:		j label2
			bne $t0, $t1, label3
			
label4:		jal label1

			beq $t0, $t1, label2
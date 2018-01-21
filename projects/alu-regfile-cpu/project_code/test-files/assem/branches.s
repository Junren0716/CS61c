        beq $t0 $t0 start       #0
badness:
        lui $t0 0xFF            #1
        beq $t0 $t0 badness     #2
label1:
        lui $t0 3               #3
        bne $t0 $t3 label2      #4

start:  lui $t0 1               #5
        beq $t0 $t1 badness     #6
        lui $t0 2               #7
        bne $t0 $t1 label1      #8

label2: lui $t0 4               #9
end:    ori $t2 $t0 0           #10
        beq $t2 $t0 end         #11
        ori $t2 $t0 2           #12

#0, 5, 6, 7, 8, 3, 4, 9, 10, 11, 10, 11...

 lui $t0, 0x3333          #3c083333
 ori $t0, $t0, 0x4444     #35084444
 lui $t1, 0x3333          #3c093333
 ori $t1, $t1, 0x4444     #35294444
self: beq $t0, $t1, self  #1109ffff
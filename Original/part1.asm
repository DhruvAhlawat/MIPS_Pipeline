addi $s0, $0, 1
loop:
	beq $s0, $0, exit
	addi $s0, $s0, -1
	j loop
exit:
	addi $t0, $0, 1




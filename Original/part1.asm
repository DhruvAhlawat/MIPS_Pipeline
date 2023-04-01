addi $s0, $0, 0
loop:
	beq $s0, $0, exit
	addi $s0, $s0, -1
	addi $s1, $0, 5
	addi $s2, $0, 10
	# j loop
	
exit:
	addi $t0, $0, 1




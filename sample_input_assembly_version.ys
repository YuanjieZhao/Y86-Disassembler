.pos 0x100
sum_function:
	irmovq start, %rdx
	xorq   %rax, %rax
L2:
	mrmovq (%rdx), %rbx
	addq   %rbx, %rax
	irmovq $8, %rbx
	addq   %rbx, %rdx
	irmovq end, %rbx
	subq   %rdx, %rbx
	jne   L2
	ret

.align 8
start:  .quad 0x123456789ABC
	.quad 0x1111111111111
	.quad 0xFF
	.quad 0x2468A
	.quad 0xBA
	.quad 0x10101010F0F0F0F0
end:   .quad 0
	

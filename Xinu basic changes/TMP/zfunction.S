.globl zfunction

zfunction:
	push %ebp
	movl %esp, %ebp
	movl 8(%ebp), %eax 
	and $0xFFC01FFF, %eax
	shl $0x4, %eax
	pop %ebp
	ret
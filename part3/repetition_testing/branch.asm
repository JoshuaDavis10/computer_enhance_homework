global ConditionalNOP

section .text

ConditionalNOP:
	xor rax, rax
.loop:
	mov r10, [rdi + rax]
	inc rax
	test r10, 1
	jnz .skip
	nop
.skip:
	cmp rax, rsi
	jb .loop
	ret

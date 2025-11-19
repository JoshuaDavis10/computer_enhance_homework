global READ_1GB_ASM

section .text

; NOTE(josh); written for x64 linux ABI, expects rdi, rsi, rdx as first 3 parameters

READ_1GB_ASM:
	align 64
	xor rax, rax
.loop:
	mov rcx, rdi
	mov r8, rax
	and r8, rdx ; address_mask
	add rcx, r8 ; add offset (i.e. bytes read so far) to base pointer

; read 128 bytes
	vmovdqu ymm0, [rcx]
	vmovdqu ymm1, [rcx+32]
	vmovdqu ymm2, [rcx+64]
	vmovdqu ymm3, [rcx+96]
	add rax, 128 ; read 128 bytes
	cmp rax, rsi ; check if we've read rsi (bytes_to_read) bytes
	jb .loop
	ret

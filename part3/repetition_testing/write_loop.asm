global MOVAllBytesASM
global NOPAllBytesASM
global CMPAllBytesASM
global DECAllBytesASM

global NOP3x1AllBytes
global NOP1x3AllBytes
global NOP1x9AllBytes

section .text

; NOTE(josh): written for x64 linux ABI, expects rsi and rdi to be first 2 parameters

MOVAllBytesASM:
	xor rax, rax
.loop:
	mov[rsi + rax], al
	inc rax
	cmp rax, rdi
	jb .loop
	ret

NOPAllBytesASM:
	xor rax, rax
.loop: 
	db 0x0f, 0x1f, 0x00 ; NOTE(josh): byte sequence for 3-byte NOP (no-op)
	inc rax
	cmp rax, rdi
	jb .loop
	ret

CMPAllBytesASM:
	xor rax, rax
.loop:
	inc rax
	cmp rax, rdi
	jb .loop
	ret

DECAllBytesASM:
.loop:
	dec rdi
	jnz .loop
	ret
	
NOP3x1AllBytes:
	xor rax, rax
.loop: 
	db 0x0f, 0x1f, 0x00 ; NOTE(josh): byte sequence for 3-byte NOP (no-op)
	inc rax
	cmp rax, rdi
	jb .loop
	ret

NOP1x3AllBytes:
	xor rax, rax
.loop: 
	nop
	nop
	nop
	inc rax
	cmp rax, rdi
	jb .loop
	ret

NOP1x9AllBytes:
	xor rax, rax
.loop: 
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	inc rax
	cmp rax, rdi
	jb .loop
	ret


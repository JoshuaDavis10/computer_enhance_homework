global write_x1
global write_x2
global write_x3
global write_x4

section .text

; NOTE(josh): written for x64 linux ABI, expects rdi and rsi to be first 2 parameters

write_x1:
	align 64
	mov rax, 0xDEADBEEF
.loop:
	mov [rdi], rax 
	sub rsi, 1	  
	jnle .loop
	ret

write_x2:
	align 64
	mov rax, 0xDEADBEEF
.loop:
	mov [rdi], rax 
	mov [rdi], rax
	sub rsi, 2	   
	jnle .loop
	ret

write_x3:
	align 64
	mov rax, 0xDEADBEEF
.loop:
	mov [rdi], rax 
	mov [rdi], rax
	mov [rdi], rax
	sub rsi, 3  
	jnle .loop
	ret

write_x4:
	align 64
	mov rax, 0xDEADBEEF
.loop:
	mov [rdi], rax 
	mov [rdi], rax
	mov [rdi], rax
	mov [rdi], rax
	sub rsi, 4
	jnle .loop
	ret

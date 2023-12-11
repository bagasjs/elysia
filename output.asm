section .text
global _start
main:
    push rbp
    mov  rbp, rsp
    pop  rbp
    ret_start:
    call main
    mov rax, 60
    mov rdi, 0
    syscall
    ret

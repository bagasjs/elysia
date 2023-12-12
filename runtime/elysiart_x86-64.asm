section .text
global _start, write, exit

extern main

write:
    mov rax, 1
    syscall
    ret

exit:
    mov rax, 60
    syscall
    ret

_start:
    call main
    mov rdi, rax
    call exit
    ret


section .text

global exit_ok
global exit_err

exit_ok:
    mov rax, 1
    mov rdi, 1
    mov rsi, ok_msg
    mov rdx, length_of_ok_msg

    mov rax, 60
    mov rdi, 0
    syscall

exit_err:
    mov rax, 1
    mov rdi, 1
    mov rsi, err_msg
    mov rdx, length_of_err_msg

    mov rax, 60
    mov rdi, 1
    syscall

section .data
    err_msg db "ERROR", 0
    length_of_err_msg equ $ - err_msg
    ok_msg db "SUCCESS", 0
    length_of_ok_msg equ $ - ok_msg

section .text
global main
extern printf
main:
    push    rbp
    mov     rbp, rsp
    sub     rsp, 16
    mov     edi, .hello_world   ; Load the address of the string
    mov     eax, 0              ; Format specifier for "%s"
    call    printf
    mov     eax, 0
    leave
    ret

section .data
.hello_world db "Hello, World", 0  ; Null-terminate the string

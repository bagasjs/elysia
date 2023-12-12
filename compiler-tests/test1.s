section .text
global _start
extern exit_ok
_start:
    call exit_ok
    

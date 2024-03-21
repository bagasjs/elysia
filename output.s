.text
.globl test
test:
	pushq %rbp
	movq %rsp, %rbp
	movl $1, %eax
	leave
	ret
.type test, @function
.size test, .-test
/* end function test */

.text
.globl main
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $69, %eax
	leave
	ret
.type main, @function
.size main, .-main
/* end function main */

.section .note.GNU-stack,"",@progbits

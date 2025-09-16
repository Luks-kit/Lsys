    .section .data
v_0:
    .long 0
    .section .text
    .global _start
_start:
    movl v_0(%rip), %eax
    movl %eax, %ebx
    movl $1, %eax
    movl %eax, %ecx
    movl %ebx, %eax
    movl %ecx, %ebx
    addl %ebx, %eax
    movl %eax, v_0(%rip)
    movl $60, %eax
    xorl %edi, %edi
    syscall

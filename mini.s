    section .data
v_0:
    .long 0
v_1:
    .long 0
    .section .text
    .global _start
_start:
    movl $1, %eax
    movl %eax, v_0(%rip)
    movl v_0(%rip), %eax
    movl %eax, %ebx
    movl v_1(%rip), %eax
    movl %eax, %ecx
    movl %ebx, %eax
    movl %ecx, %ebx
    addl %ebx, %eax
    movl %eax, v_0(%rip)
    movl v_0(%rip), %eax
    movl %eax, %ebx
    movl $1, %eax
    movl %eax, %ecx
    movl %ebx, %eax
    movl %ecx, %ebx
    addl %ebx, %eax
    movl %eax, v_0(%rip)
    movl v_1(%rip), %eax
    movl %eax, %ebx
    movl $2, %eax
    movl %eax, %ecx
    movl %ebx, %eax
    movl %ecx, %ebx
    imull %ebx, %eax
    movl %eax, v_1(%rip)
    movl v_1(%rip), %eax
    movl %eax, %ebx
    movl $1, %eax
    movl %eax, %ecx
    movl %ebx, %eax
    movl %ecx, %ebx
    subl %ebx, %eax
    movl %eax, v_1(%rip)
    movl $60, %eax
    xorl %edi, %edi
    syscall

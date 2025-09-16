; start.s - minimal x86_64 Linux entry point
; Assemble with:   as -o start.o start.s
; Link with:       ld -o myprog start.o main.o ...

.global _start
.extern main

.section .text
_start:
    ; argc = [rsp]
    mov rdi, [rsp]           ; first argument: argc
    lea rsi, [rsp+8]         ; second argument: argv (pointer to argv[0])
    mov rdx, rsi
.next_arg:
    cmp qword [rdx], 0       ; scan until NULL terminator
    add rdx, 8
    jne .next_arg
    add rdx, 8               ; now rdx = envp (just past argv NULL)

    ; Call C main(argc, argv, envp)
    call main

    ; main returns int -> exit with that code
    mov rdi, rax             ; arg: exit status
    mov rax, 60              ; sys_exit
    syscall


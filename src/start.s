.intel_syntax noprefix
.section .text
.global _start
.global read
.global write
.global openat
.global close
.global fstat
.global mmap
.global munmap
.global brk
.global exit_group

/* External C entrypoint: your compiler will provide main */
.extern main

/* ----------------------------
   _start: bootstrap argc/argv/envp
   Stack layout at program start:
     [rsp]       = argc (long)
     [rsp+8]     = argv[0] (pointer)
     [rsp+8+8*argc] = NULL
     then envp pointers...
   This code:
    - loads argc into rdi
    - loads &argv[0] into rsi
    - scans argv until NULL, sets rdx to &envp
    - calls main(argc, argv, envp)
    - syscall exit_group(return_code)
   ---------------------------- */
_start:
    /* rdi <- argc */
    mov rdi, qword ptr [rsp]

    /* rsi <- &argv[0] (pointer to first argv) */
    lea rsi, [rsp + 8]

    /* find end of argv: rcx = index; loop until qword [rsi + rcx*8] == 0 */
    xor rcx, rcx
.find_argv_end:
    mov rax, qword ptr [rsi + rcx*8]
    test rax, rax
    jnz .inc_argv
    /* envp pointer = rsi + (rcx+1)*8  (skip the NULL) */
    lea rdx, [rsi + rcx*8 + 8]
    jmp .call_main
.inc_argv:
    inc rcx
    jmp .find_argv_end

.call_main:
    /* Call main(argc, argv, envp) */
    call main

    /* on return: rax = return value; call exit_group(return_code) */
    mov rdi, rax          /* exit code -> rdi (1st syscall arg) */
    mov rax, 231          /* sys_exit_group */
    syscall

/* ----------------------------
   Syscall wrappers
   - Follow Linux x86_64 syscall arg registers:
       rdi, rsi, rdx, r10, r8, r9
     But C function call uses:
       rdi, rsi, rdx, rcx, r8, r9
     So when we need the 4th argument for syscall we must move rcx -> r10.
   - syscall returns in rax (or negative value in rax for -errno)
   - wrappers are minimal and do no errno translation
   ---------------------------- */

/* ssize_t read(int fd, void *buf, size_t count) */
read:
    mov rax, 0        /* __NR_read = 0 */
    syscall
    ret

/* ssize_t write(int fd, const void *buf, size_t count) */
write:
    mov rax, 1        /* __NR_write = 1 */
    syscall
    ret

/* int close(int fd) */
close:
    mov rax, 3        /* __NR_close = 3 */
    syscall
    ret

/* long fstat(int fd, struct stat *statbuf) */
fstat:
    mov rax, 5        /* __NR_fstat = 5 */
    syscall
    ret

/* int openat(int dirfd, const char *pathname, int flags, mode_t mode)
   - C args: rdi=dirfd, rsi=pathname, rdx=flags, rcx=mode
   - syscall args: rdi, rsi, rdx, r10=mode
*/
openat:
    mov rax, 257      /* __NR_openat = 257 on x86_64 */
    mov r10, rcx      /* move 4th arg into r10 for syscall */
    syscall
    ret

/* void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
   C args: rdi, rsi, rdx, rcx, r8, r9
   Syscall args: rdi, rsi, rdx, r10, r8, r9  => move rcx->r10
*/
mmap:
    mov rax, 9        /* __NR_mmap = 9 */
    mov r10, rcx
    syscall
    ret

/* int munmap(void *addr, size_t length) */
munmap:
    mov rax, 11       /* __NR_munmap = 11 */
    syscall
    ret

/* void *brk(void *addr) */
brk:
    mov rax, 12       /* __NR_brk = 12 */
    syscall
    ret

/* int exit_group(int status) -- also provided for completeness */
exit_group:
    mov rax, 231      /* __NR_exit_group = 231 */
    syscall
    /* never returns, but keep a ret for assembler sanity */
    ret

/* End of file */


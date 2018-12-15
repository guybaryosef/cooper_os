/*
 * ECE 357: Operating Systems
 * Pset 7: Problem 4 - Hello World in assembly, version 3
 * By: Guy Bar Yosef
 *
 * This program is to be run using X86-64. (yes, this doesn't work)
 */
 
        .global _start

        .text
_start:
        # the system call number will go into rax.
        # args 1, 2, 3, will go into rdi, rsi, and rdx, respectively.

        # coding up: write(STDOUT, 10, 13)
        mov     $1, %rax             # the write syscall number is 1
        mov     $1, %rdi             # stdout is file descriptor number 1
        mov     $10, %rsi           # address of message
        mov     $13, %rdx            # message length
        syscall                      # execute syscall

        # coding up: _exit(3) 
        mov     $60, %rax            # the _exit syscall number is 
        mov     $3, %rdi             # call exit with a specific non-zero 
        syscall                      # execute syscall

msg:
        .ascii  "Hello World!\n"

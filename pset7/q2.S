/*
 * ECE 357: Operating Systems
 * Pset 7: Problem 2 - Hello World in assembly
 * By: Guy Bar Yosef
 *
 * This program is to be run using X86-64, and yes, it seg faults.
 */
 
        .global _start

        .text
_start:
        # the system call number will go into rax.
        # args 1, 2, 3, will go into rdi, rsi, and rdx, respectively.

        # coding up: write(STDOUT, msg, 13)
        mov     $1, %rax             # the write syscall number is 1
        mov     $1, %rdi             # stdout is file descriptor number 1
        mov     $msg, %rsi           # address of message
        mov     $13, %rdx            # message length
        syscall                      # execute syscall

msg:
        .ascii  "Hello World!\n"

; basic assembly functions for system calls

GLOBAL syscall

section .text
syscall:
    ; first argument is the syscall id, the rest are the arguments to the syscall
    ; uses the same argument order as C, so that no rearangement is needed 

    ; makes the system call
    int 0x80
    ret



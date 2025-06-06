section .text

GLOBAL _start

_start:
    mov rax, 1051
    mov rbx, 1051
    mov rcx, 1051
    mov rdx, 1051
    mov rsi, 1051
    mov rdi, 1051
    mov r8, 1051
    mov r9, 1051
    mov r10, 1051
    mov r11, 1051
    mov r12, 1051
    mov r13, 1051
    mov r14, 1051
    mov r15, 1051

    
    int 0x80

    mov rax, 69
    mov rdi, 0
    div rdi

    ret
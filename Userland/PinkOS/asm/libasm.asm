GLOBAL get_stack_pointer, _hlt

section .text

get_stack_pointer:
    mov rax, rsp
    add rax, 8
    ret

_hlt:
    sti
    hlt
    ret
GLOBAL get_stack_pointer, _hlt
GLOBAL make_0x0_exception, make_0x6_exception

section .text

get_stack_pointer:
    mov rax, rsp
    add rax, 8
    ret

make_0x0_exception:
    mov rax, 69
    mov rdi, 0
    div rdi
    ret

make_0x6_exception:
    ud2
    ret

_hlt:
    sti
    hlt
    ret
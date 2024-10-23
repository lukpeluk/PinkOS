
; función que devuelve el keycode de teclado

section .text
global getKeyCode

getKeyCode:
    push rbp
    mov rbp, rsp
    mov rax, 0
    in al, 0x60
    pop rbp
    ret


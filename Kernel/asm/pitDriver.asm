section .data
    PIT_CMD db 0x36            ; Canal 0, modo 3, lobyte/hibyte, binario

section .text
global init_pit
init_pit:
    ; Salvar RAX y flags
    push    rax
    pushfq

    ; 1) Seleccionar canal 0, modo 3, lobyte/hibyte
    mov     al, 0x36           ; 0011 0110b → canal 0, lobyte/hibyte, modo 3, binario
    out     0x43, al           ; Comando al PIT

    ; 2) Cargar divisor = 239 para ≈5 000 Hz
    mov     ax, 239            ; AX = 0x00EF
    mov     dx, 0x40           ; Puerto de datos canal 0
    out     dx, al             ; Enviar byte bajo (0xEF)
    mov     al, ah             ; AH = 0x00
    out     dx, al             ; Enviar byte alto (0x00)

    ; Restaurar flags y RAX
    popfq
    pop     rax
    ret
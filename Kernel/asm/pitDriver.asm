section .data
    PIT_CMD db 0x36            ; Canal 0, modo 3, lobyte/hibyte, binario

section .text
global init_pit
init_pit:
    ; Salvar RAX y flags
    push    rax
    pushfq

    ; 1) Seleccionar canal 0, modo 3, lobyte/hibyte
    mov     al, [PIT_CMD]
    out     0x43, al            ; Enviar comando al PIT

    ; 2) Cargar divisor = 23863 → 0x5D47
    mov     ax, 23863           ; AX = 0x5D47
    mov     dx, 0x40            ; Puerto de datos del canal 0

    out     dx, al              ; Enviar byte bajo (0x47)
    mov     al, ah
    out     dx, al              ; Enviar byte alto (0x5D)

    ; Restaurar flags y RAX
    popfq
    pop     rax
    ret
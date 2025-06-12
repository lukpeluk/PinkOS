section .data
    PIT_CMD db 0x36            ; Canal 0, modo 3, lobyte/hibyte, binario

section .text
global init_pit
init_pit:
    ; Salvar RAX y flags
    push    rax
    pushfq

    ; 1) Seleccionar canal 0, modo 3, lobyte/hibyte
    mov     al, 0x36             ; 00 11 011 0b → canal 0, lobyte/hibyte, modo 3, binario
    out     0x43, al             ; Comando PIT

    ; 2) Cargar divisor = 0 → 65 536 (frecuencia ≈ 1 193 182 / 65 536 ≈ 18.206 Hz)
    mov     al, 0x00             ; byte bajo
    out     0x40, al             ; enviar al puerto 0x40
    mov     al, 0x00             ; byte alto
    out     0x40, al             ; enviar al puerto 0x40

    ; Restaurar flags y RAX
    popfq
    pop     rax
    ret
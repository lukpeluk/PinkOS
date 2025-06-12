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

    ; 2) Cargar divisor = 23863 → 0x5D47 (50 Hz)
    mov     ax, 23863           ; AX = 0x5D47
    mov     dx, 0x40            ; Puerto de datos del canal 0
    out     dx, al              ; Enviar byte bajo (0x47)
    mov     al, ah
    out     dx, al              ; Enviar byte alto (0x5D)

;    ; 2) Cargar divisor = 0 → 65 536 (frecuencia ≈ 1 193 182 / 65 536 ≈ 18.206 Hz)
;    mov     al, 0x00             ; byte bajo
;    out     0x40, al             ; enviar al puerto 0x40
;    mov     al, 0x00             ; byte alto
;    out     0x40, al             ; enviar al puerto 0x40

;    ; 2) Cargar divisor = 1193 para ~1 000 Hz
;    mov     ax, 1193           ; AX = 0x04A9
;    mov     dx, 0x40           ; Puerto de datos canal 0
;    out     dx, al             ; Enviar byte bajo (0xA9)
;    mov     al, ah
;    out     dx, al             ; Enviar byte alto (0x04)

;    ; 2) Cargar divisor = 239 para ≈5 000 Hz
;    mov     ax, 239            ; AX = 0x00EF
;    mov     dx, 0x40           ; Puerto de datos canal 0
;    out     dx, al             ; Enviar byte bajo (0xEF)
;    mov     al, ah             ; AH = 0x00
;    out     dx, al             ; Enviar byte alto (0x00)


    ; Restaurar flags y RAX
    popfq
    pop     rax
    ret
section .data
    PIT_COMMAND_BYTE db 0x36   ; Canal 0, modo 3 (Square Wave Generator), binary
    DESIRED_FREQUENCY dd 18.2      ; Frecuencia deseada

section .text
global init_pit

init_pit:
    mov eax, 1193182           ; Frecuencia base del PIT
    mov ebx, [DESIRED_FREQUENCY] ; Frecuencia objetivo
    div ebx                     ; Calcula el divisor en eax
    mov dx, 0x40                ; Puerto de datos del PIT (canal 0)
    
    mov al, byte [PIT_COMMAND_BYTE]
    out 0x43, al                ; Envía el comando para configurar el modo del PIT

    mov al, al                  ; Escribe la parte baja del divisor
    out dx, al                  ; Escribe al puerto 0x40
    mov al, ah                  ; Escribe la parte alta del divisor
    out dx, al                  ; Escribe al puerto 0x40

    ret

section .text

GLOBAL init_rtc
GLOBAL rtc_acknowledge_interrupt
GLOBAL get_time_utc

init_rtc:
    ; Configurar el RTC para interrupciones periódicas

    ; Deshabilitar interrupciones temporalmente para configuración
    mov al, 0x8B                ; Seleccionar Registro B
    out 0x70, al
    in  al, 0x71                ; Leer valor actual del registro B
    or al, 0x10                 ; Habilitar UIE (Update Interrupt Enable, bit 4)
    out 0x71, al

    ret                         ; Retornar al código en C

rtc_acknowledge_interrupt:
    mov dx, 0x70           ; Cargar el puerto de control en DX
    mov al, 0x0C           ; Seleccionar registro C
    out dx, al             ; Desactivar NMI
    in al, 0x71              ; Leer registro C y descartar el valor
    
    
    mov al, 0x20
	out 0xA0, al  ; EOI para el PIC esclavo

    ret

; Estructura RTC_Time:
; offset 0: seconds
; offset 1: minutes
; offset 2: hours
; offset 3: day
; offset 4: month
; offset 5: year
; offset 6: day_of_week

get_time_utc:
    ; RDI apunta a la estructura RTC_Time

    cli  ; Deshabilitar interrupciones

    ; Leer y convertir segundos
    mov al, 0x00
    out 0x70, al
    in  al, 0x71
    call convert_bcd_to_binary
    mov [rdi], al

    ; Leer y convertir minutos
    mov al, 0x02
    out 0x70, al
    in  al, 0x71
    call convert_bcd_to_binary
    mov [rdi + 1], al

    ; Leer y convertir horas
    mov al, 0x04
    out 0x70, al
    in  al, 0x71
    call convert_bcd_to_binary
    mov [rdi + 2], al

    ; Leer y convertir día
    mov al, 0x07
    out 0x70, al
    in  al, 0x71
    call convert_bcd_to_binary
    mov [rdi + 3], al

    ; Leer y convertir mes
    mov al, 0x08
    out 0x70, al
    in  al, 0x71
    call convert_bcd_to_binary
    mov [rdi + 4], al

    ; Leer y convertir año
    mov al, 0x09
    out 0x70, al
    in  al, 0x71
    call convert_bcd_to_binary
    mov [rdi + 5], al

    ; Leer día de la semana (no necesita conversión)
    mov al, 0x06
    out 0x70, al
    in  al, 0x71
    mov [rdi + 6], al

    sti  ; Habilitar interrupciones
    ret

; Subrutina para convertir BCD a binario
convert_bcd_to_binary:
    ; Al tiene el valor en BCD
    push rbx         ; Guardar BL
    push rcx         ; Guardar CL

    mov cl, al
    and cl, 0x0F
    and al, 0xF0
    shr al, 4
    mov bl, 10
    mul bl
    add al, cl

    pop rcx          ; Restaurar CL
    pop rbx          ; Restaurar BL

    ret

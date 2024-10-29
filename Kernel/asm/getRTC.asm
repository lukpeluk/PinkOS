section .text
global rtc_handler, read_rtc_time
global init_rtc
global disable_ints, enable_ints
global outportb, inportb
global rtc_acknowledge_interrupt

GLOBAL get_hours, get_minutes, get_seconds, get_day, get_month, get_year, get_day_of_week   

rtc_acknowledge_interrupt:
    mov dx, 0x70           ; Cargar el puerto de control en DX
    mov al, 0x0C           ; Seleccionar registro C
    out dx, al             ; Desactivar NMI
    in al, 0x71              ; Leer registro C y descartar el valor
    
    
    mov al, 0x20
	out 0xA0, al  ; EOI para el PIC esclavo

    ret


get_hours:
	cli ; disable interrupts

	mov al, 0x04 ; read hours
	out 70h, al
	in al, 71h

	sti ; enable interrupts
	ret

get_minutes:
	cli ; disable interrupts

	mov al, 0x02 ; read minutes
	out 70h, al
	in al, 71h

	sti ; enable interrupts
	ret

get_seconds:
	cli ; disable interrupts

	mov al, 0x00 ; read seconds
	out 70h, al
	in al, 71h

	sti ; enable interrupts
	ret

get_day:
    cli ; disable interrupts

    mov al, 0x07 ; read day
    out 70h, al
    in al, 71h

    sti ; enable interrupts
    ret

get_month:
    cli ; disable interrupts

    mov al, 0x08 ; read month
    out 70h, al
    in al, 71h

    sti ; enable interrupts
    ret

get_year:
    cli ; disable interrupts

    mov al, 0x09 ; read year
    out 70h, al
    in al, 71h

    sti ; enable interrupts
    ret

get_day_of_week:
    cli ; disable interrupts

    mov al, 0x06 ; read day of week
    out 70h, al
    in al, 71h

    sti ; enable interrupts
    ret

rtc_handler:
    ; Guardar los registros manualmente en 64 bits
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11

    ; Leer registro C del RTC para confirmar la interrupción
    mov al, 0x0C                ; Seleccionar Registro C
    out 0x70, al                ; Enviar al puerto de índice (0x70)
    in  al, 0x71                ; Leer desde el puerto de datos (0x71)

    ; Restaurar los registros
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq                       ; Retornar de la interrupción (64 bits)

init_rtc:
    ; Configurar el RTC para interrupciones periódicas

    ; Deshabilitar interrupciones temporalmente para configuración
    mov al, 0x8B                ; Seleccionar Registro B
    out 0x70, al
    in  al, 0x71                ; Leer valor actual del registro B
    or al, 0x10                 ; Habilitar UIE (Update Interrupt Enable, bit 4)
    out 0x71, al

    ret                         ; Retornar al código en C

read_rtc_time:
    cli                         ; Deshabilitar interrupciones

    ; Leer segundos
    mov al, 0x00                ; Registro de segundos
    out 0x70, al
    in  al, 0x71                ; Leer segundos
    call bcd_to_binary          ; Convertir a binario
    mov [rdi], al               ; Guardar en *(rdi)

    ; Leer minutos
    mov al, 0x02                ; Registro de minutos
    out 0x70, al
    in  al, 0x71                ; Leer minutos
    call bcd_to_binary          ; Convertir a binario
    mov [rdi + 1], al           ; Guardar en *(rdi + 1)

    ; Leer horas
    mov al, 0x04                ; Registro de horas
    out 0x70, al
    in  al, 0x71                ; Leer horas
    call bcd_to_binary          ; Convertir a binario
    mov [rdi + 2], al           ; Guardar en *(rdi + 2)

    sti                         ; Habilitar interrupciones
    ret                         ; Retornar al código en C
    
bcd_to_binary:
    ; (AL contiene el valor BCD)
    movzx rax, al              ; Extender AL a RAX (64 bits)

    ; Separar decenas y unidades
    shr rax, 4                 ; Desplazar decenas a bits bajos
    imul rax, rax, 10          ; Multiplicar decenas por 10

    ; Obtener las unidades de BCD y sumarlas
    and al, 0x0F               ; Enmascarar unidades en AL
    add rax, rax               ; Sumar unidades a RAX

    ; Guardar el resultado final en AL (parte baja de RAX)
    mov al, bl                 ; Usamos un registro temporal de 8 bits

    ret

section .text
global disable_ints, enable_ints, outportb, inportb

; Deshabilitar interrupciones
disable_ints:
    cli                    ; Clear Interrupt Flag
    ret                    ; Retornar

; Habilitar interrupciones
enable_ints:
    sti                    ; Set Interrupt Flag
    ret                    ; Retornar

; Enviar un byte al puerto (I/O)
outportb:
    ; Entrada: Puerto en DX, dato en AL
    mov dx, di             ; Mover el puerto desde DI a DX
    mov al, sil            ; Mover el dato desde SIL a AL (8 bits)
    out dx, al             ; Enviar el byte al puerto
    ret                    ; Retornar

; Leer un byte desde un puerto (I/O)
inportb:
    ; Entrada: Puerto en DX, Salida: AL
    mov dx, di             ; Mover el puerto desde DI a DX
    in al, dx              ; Leer el byte desde el puerto
    ret                    ; Retornar

GLOBAL cpuVendor
GLOBAL disable_ints, enable_ints, outportb, inportb, magic_recover, magic_recover_old, load_interrupt_frame, load_stack_int, push_to_custom_stack_pointer, get_stack_pointer, lightspeed_memcpy

; TODO: 
; 	- Ofrecer funciones para acceder a instrucciones como in y hlt

section .text
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

;; Resetea el stack y hace un iretq (popear la struct StackInt en el orden correcto)
;; Recibe como argumento el puntero al struct StackInt
;; typedef struct {
;;     uint64_t rip;
;;     uint64_t cs;
;;     uint64_t rflags;
;;     uint64_t rsp;
;;     uint64_t ss;
;; } StackInt;
magic_recover_old:
    ; rdi apunta a la estructura InterruptStackFrame en memoria

    ; Cargar valores desde la estructura InterruptStackFrame al stack en el orden correcto para iretq
    mov rax, [rdi + 0]   ; Cargar ss
    push rax              ; Apilar ss
    mov rax, [rdi + 8]   ; Cargar rsp
    push rax              ; Apilar rsp
    mov rax, [rdi + 16]    ; Cargar rip
    push rax              ; Apilar rip
    mov rax, [rdi + 24]    ; Cargar cs
    push rax              ; Apilar cs
    mov rax, [rdi + 32]   ; Cargar rflags
    push rax              ; Apilar rflags

    mov rdi, rsi

    ; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al


    ; Ejecutar iretq para retornar usando el contexto cargado
    iretq

; Recibe un puntero a una estructura Registers que debe tener el backup de registros, y en base a ella retoma la ejecución de un proceso
; Requiere que el rsp de Registers apunte a un interrupt stack frame válido desde el cual hacer iretq
magic_recover:
    ; rdi apunta a la estructura Registers en memoria

    ; Restaurar todos los registros desde la estructura Registers
    ; Orden en la estructura: rax, rbx, rcx, rdx, rsi, rdi, rsp, rbp, r8, r9, r10, r11, r12, r13, r14, r15
    
    ; Primero cambiar al stack pointer del proceso
    mov rax, [rdi + 48]     ; Cargar rsp desde Registers (offset 48 = 6 * 8 bytes)
    mov rsp, rax            ; Establecer el stack pointer del proceso
    
    ; Restaurar registros (guardando rdi para el final)
    mov rax, [rdi + 0]      ; Restaurar rax
    mov rbx, [rdi + 8]      ; Restaurar rbx
    mov rcx, [rdi + 16]     ; Restaurar rcx
    mov rdx, [rdi + 24]     ; Restaurar rdx
    mov rsi, [rdi + 32]     ; Restaurar rsi
    mov rbp, [rdi + 56]     ; Restaurar rbp (offset 56 = 7 * 8 bytes, saltando rdi y rsp)
    mov r8,  [rdi + 64]     ; Restaurar r8
    mov r9,  [rdi + 72]     ; Restaurar r9
    mov r10, [rdi + 80]     ; Restaurar r10
    mov r11, [rdi + 88]     ; Restaurar r11
    mov r12, [rdi + 96]     ; Restaurar r12
    mov r13, [rdi + 104]    ; Restaurar r13
    mov r14, [rdi + 112]    ; Restaurar r14
    mov r15, [rdi + 120]    ; Restaurar r15
    
    ; Finalmente restaurar rdi (debe ser el último porque lo estamos usando como puntero)
    mov rdi, [rdi + 40]     ; Restaurar rdi (offset 40 = 5 * 8 bytes)

    ; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

    ; Ejecutar iretq para retornar usando el contexto cargado
    iretq



; Pone en el stack pointer pasado como segundo arg, el interrupt frame (InterruptStackFrame) pasado como primer arg
; Esto permite que luego hacer iretq con el stack pointer pasado restaure el contexto del InterruptStackFrame 
; Retorna el nuevo valor del stack pointer después de cargar el frame
load_interrupt_frame:
    ; rdi apunta a la estructura InterruptStackFrame en memoria
    ; rsi es el stack pointer donde cargar el frame

    ; Guardar el stack pointer actual
    push rbp
    mov rbp, rsp
    
    ; Cambiar al stack pointer pasado como argumento
    mov rsp, rsi

    ; Cargar valores desde la estructura InterruptStackFrame al stack en el orden correcto para iretq
    mov rax, [rdi + 0]   ; Cargar ss
    push rax              ; Apilar ss
    mov rax, [rdi + 8]   ; Cargar rsp
    push rax              ; Apilar rsp
    mov rax, [rdi + 16]    ; Cargar rip
    push rax              ; Apilar rip
    mov rax, [rdi + 24]    ; Cargar cs
    push rax              ; Apilar cs
    mov rax, [rdi + 32]   ; Cargar rflags
    push rax              ; Apilar rflags

    ; Guardar el nuevo stack pointer (después de los push) en rax para retornarlo
    mov rax, rsp

    ; Restaurar el stack pointer original
    mov rsp, rbp
    pop rbp
    ret

; loads a value to a specific point in the stack
; rdi: pointer to the stack
; rsi: value to load
push_to_custom_stack_pointer:
    push rbp            ; Guardar el valor de rbp
    mov rbp, rsp        ; rbp apunta al inicio del stack (StackFrame)
    mov rsp, rdi        ; rsp ahora apunta al inicio del stack que queremos cargar
    push rsi            ; Apilar el valor que queremos cargar
    mov rsp, rbp        ; Restaurar el valor de rsp
    pop rbp             ; Restaurar el valor de rbp (dejamos el stack como estaba)
    ret

; get the actual stack pointer
get_stack_pointer:
    mov rax, rsp
    add rax, 8
    ret

; Recibe un puntero a un struct StackInt y carga los valores de la estructura en el stack
; typedef struct {
;     uint64_t rip;
;     uint64_t cs;
;     uint64_t rflags;
;     uint64_t rsp;
;     uint64_t ss;
; } StackInt;
;load_stack_int:
;    ; rdi apunta a la estructura InterruptStackFrame en memoria
;    ; rbx apunta al final de la estructura en el stack (ss)
;
;    mov rbx, [pointerToStack] ; Obtener el puntero al final de la estructura en el stack (SS)
;
;    ; Copiar cada campo desde el stack al destino en el orden adecuado
;    mov rax, [rbx + 0]        ; Obtener ss
;    mov [rdi + 32], rax       ; Guardar ss en la estructura destino
;    mov rax, [rbx + 8]        ; Obtener rsp
;    mov [rdi + 24], rax       ; Guardar rsp en la estructura destino
;    mov rax, [rbx + 16]       ; Obtener rflags
;    mov [rdi + 16], rax       ; Guardar rflags en la estructura destino
;    mov rax, [rbx + 24]       ; Obtener cs
;    mov [rdi + 8], rax        ; Guardar cs en la estructura destino
;    mov rax, [rbx + 32]       ; Obtener rip
;    mov [rdi + 0], rax        ; Guardar rip en la estructura destino
;    ret


	

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


; -----------------------------------------------------------------------------
; void *lightspeed_memcpy(void *dest, const void *src, uint64_t len)
;   RDI = dest
;   RSI = src
;   RDX = len
; returns RAX = dest
; -----------------------------------------------------------------------------
lightspeed_memcpy:
    ; Guardar destino en RAX para retorno
    mov     rax, rdi

    ; Preparar conteos
    mov     rcx, rdx         ; RCX = total bytes
    shr     rcx, 3           ; RCX = cuantos qwords (len / 8)
    jz      .tail_bytes      ; si era <8 bytes, saltar

    ; Copiar bloques de 8 bytes (qwords)
    rep movsq                ; [RDI] ← [RSI], RDI+=8, RSI+=8, RCX--

    ; RDX_mod = len % 8
    mov     rcx, rdx         ; RCX = total bytes
    and     rcx, 7           ; RCX = resto (len & 7)
    jz      .done            ; si 0 bytes sobrantes, fin

.tail_bytes:
    ; Copiar los bytes restantes uno a uno
    rep movsb                ; [RDI] ← [RSI], RDI++, RSI++, RCX--

.done:
    ret
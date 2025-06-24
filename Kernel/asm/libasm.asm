GLOBAL cpuVendor
GLOBAL disable_ints, enable_ints, outportb, inportb, magic_recover, magic_recover_old, load_interrupt_frame, load_stack_int, push_to_custom_stack_pointer, get_stack_pointer, lightspeed_memcpy
GLOBAL lightspeed_memset, lightspeed_memcmp, lightspeed_memmove, lightspeed_memzero
GLOBAL quitWrapper
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

; void *lightspeed_memset(void *dest, int value, uint64_t len)
;   RDI = dest
;   RSI = value (extendido a 8 bytes)
;   RDX = len
; returns RAX = dest
; -----------------------------------------------------------------------------
lightspeed_memset:
    ; Guardar destino en RAX para retorno
    mov     rax, rdi

    ; Verificar si hay algo que hacer
    test    rdx, rdx
    jz      .memset_done

    ; Preparar el valor en AL para stosb
    mov     al, sil          ; AL = byte a escribir (LSB de RSI)

    ; Si len < 8, usar byte a byte
    cmp     rdx, 8
    jb      .memset_bytes

    ; Expandir el byte a qword completo
    mov     ah, al           ; AX = 0xXXXX
    mov     r8w, ax          ; R8W = AX
    shl     r8, 16           ; R8 = 0xXXXX0000
    or      r8w, ax          ; R8 = 0xXXXXXXXX (32-bit)
    mov     r9, r8           ; R9 = R8
    shl     r9, 32           ; R9 = 0xXXXXXXXX00000000
    or      rax, r9          ; RAX = 0xXXXXXXXXXXXXXXXX (64-bit pattern)

    ; Calcular cuántos qwords completos
    mov     rcx, rdx         ; RCX = total bytes
    shr     rcx, 3           ; RCX = qwords (len / 8)
    jz      .memset_tail     ; si no hay qwords completos

    ; Escribir qwords
    rep stosq                ; [RDI] ← RAX, RDI+=8, RCX--

.memset_tail:
    ; Bytes restantes
    mov     rcx, rdx         ; RCX = total bytes
    and     rcx, 7           ; RCX = resto (len & 7)
    jz      .memset_done     ; si no hay resto

    ; Restaurar AL para stosb
    mov     al, sil

.memset_bytes:
    ; Escribir bytes restantes
    rep stosb                ; [RDI] ← AL, RDI++, RCX--

.memset_done:
    ; RAX ya contiene el destino original
    ret

; void *lightspeed_memzero(void *dest, uint64_t len)
;   RDI = dest
;   RSI = len
; returns RAX = dest
; Optimización especial para llenar con ceros
; -----------------------------------------------------------------------------
lightspeed_memzero:
    ; Guardar destino en RAX para retorno
    mov     rax, rdi

    ; RDX = len, RCX se usará para rep
    mov     rdx, rsi
    test    rdx, rdx
    jz      .zero_done

    ; Limpiar RAX para llenar con ceros
    xor     rax, rax

    ; Si len < 8, usar byte a byte
    cmp     rdx, 8
    jb      .zero_bytes

    ; Calcular qwords completos
    mov     rcx, rdx         ; RCX = total bytes
    shr     rcx, 3           ; RCX = qwords
    jz      .zero_tail

    ; Escribir qwords de ceros
    rep stosq

.zero_tail:
    ; Bytes restantes
    mov     rcx, rdx
    and     rcx, 7
    jz      .zero_done

.zero_bytes:
    ; Escribir bytes restantes
    rep stosb

.zero_done:
    ; Restaurar valor de retorno
    mov     rax, rdi
    ret

; int lightspeed_memcmp(const void *s1, const void *s2, uint64_t len)
;   RDI = s1
;   RSI = s2
;   RDX = len
; returns RAX = 0 si iguales, <0 si s1<s2, >0 si s1>s2
; -----------------------------------------------------------------------------
lightspeed_memcmp:
    ; Verificar si hay algo que comparar
    test    rdx, rdx
    jz      .cmp_equal

    ; Si len < 8, comparar byte a byte
    cmp     rdx, 8
    jb      .cmp_bytes

    ; Comparar qwords completos
    mov     rcx, rdx
    shr     rcx, 3           ; RCX = número de qwords
    jz      .cmp_tail

.cmp_qwords:
    mov     rax, [rdi]       ; Cargar qword de s1
    mov     r8, [rsi]        ; Cargar qword de s2
    cmp     rax, r8
    jne     .cmp_qword_diff  ; Si diferentes, encontrar el byte diferente
    add     rdi, 8
    add     rsi, 8
    dec     rcx
    jnz     .cmp_qwords

.cmp_tail:
    ; Comparar bytes restantes
    mov     rcx, rdx
    and     rcx, 7
    jz      .cmp_equal

.cmp_bytes:
    mov     al, [rdi]        ; Cargar byte de s1
    mov     r8b, [rsi]       ; Cargar byte de s2
    cmp     al, r8b
    jne     .cmp_byte_diff
    inc     rdi
    inc     rsi
    dec     rcx
    jnz     .cmp_bytes

.cmp_equal:
    xor     rax, rax         ; Retornar 0 (iguales)
    ret

.cmp_qword_diff:
    ; Encontrar el primer byte diferente en el qword
    ; Comparar byte a byte dentro del qword
    mov     rcx, 8
.find_diff_byte:
    mov     al, [rdi]
    mov     r8b, [rsi]
    cmp     al, r8b
    jne     .cmp_byte_diff
    inc     rdi
    inc     rsi
    dec     rcx
    jnz     .find_diff_byte
    ; Esto no debería pasar, pero por si acaso
    xor     rax, rax
    ret

.cmp_byte_diff:
    ; AL = byte de s1, R8B = byte de s2
    movzx   rax, al          ; Zero-extend s1 byte
    movzx   r8, r8b          ; Zero-extend s2 byte
    sub     rax, r8          ; RAX = s1_byte - s2_byte
    ret

; void *lightspeed_memmove(void *dest, const void *src, uint64_t len)
;   RDI = dest
;   RSI = src
;   RDX = len
; returns RAX = dest
; Como memcpy pero maneja overlapping correctamente
; -----------------------------------------------------------------------------
lightspeed_memmove:
    ; Guardar destino para retorno
    mov     rax, rdi

    ; Verificar si hay algo que mover
    test    rdx, rdx
    jz      .move_done

    ; Verificar si hay overlap
    cmp     rdi, rsi
    je      .move_done       ; src == dest, no hacer nada
    jb      .move_forward    ; dest < src, copiar hacia adelante

    ; dest > src, verificar si hay overlap
    mov     r8, rsi
    add     r8, rdx          ; R8 = src + len
    cmp     rdi, r8
    jae     .move_forward    ; dest >= src+len, no overlap

    ; Hay overlap y dest > src, copiar hacia atrás
    add     rdi, rdx         ; RDI apunta al final de dest
    add     rsi, rdx         ; RSI apunta al final de src
    dec     rdi              ; Ajustar para último byte
    dec     rsi

    ; Copiar hacia atrás byte a byte
    mov     rcx, rdx
    std                      ; Set direction flag (backwards)
    rep movsb
    cld                      ; Clear direction flag
    jmp     .move_done

.move_forward:
    ; No hay overlap o dest < src, usar memcpy normal
    mov     rcx, rdx
    shr     rcx, 3           ; Qwords
    jz      .move_tail_bytes

    ; Copiar qwords
    rep movsq

    ; Bytes restantes
    mov     rcx, rdx
    and     rcx, 7
    jz      .move_done

.move_tail_bytes:
    rep movsb

.move_done:
    ; RAX ya contiene el destino original
    ret

quitWrapper:
    mov rdi, 2
    int 0x80
    ret
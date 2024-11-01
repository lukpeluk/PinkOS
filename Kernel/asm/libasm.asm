GLOBAL cpuVendor
GLOBAL disable_ints, enable_ints, outportb, inportb, magic_recover, load_stack_int

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

; Resetea el stack y hace un iretq (popear la struct StackInt en el orden correcto)
; Recibe como argumento el puntero al struct StackInt
; typedef struct {
;     uint64_t rip;
;     uint64_t cs;
;     uint64_t rflags;
;     uint64_t rsp;
;     uint64_t ss;
; } StackInt;
magic_recover:
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
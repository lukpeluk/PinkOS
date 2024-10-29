GLOBAL cpuVendor
GLOBAL disable_ints, enable_ints, outportb, inportb

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
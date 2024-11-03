
GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt

GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _irq02Handler
GLOBAL _irq03Handler
GLOBAL _irq04Handler
GLOBAL _irq05Handler
GLOBAL _irq08Handler

GLOBAL _irq80Handler

GLOBAL _exception0Handler
GLOBAL _exception6Handler

GLOBAL rax_backup, rbx_backup, rcx_backup, rdx_backup, rbp_backup, rdi_backup, rsi_backup, r8_backup, r9_backup, r10_backup, r11_backup, r12_backup, r13_backup, r14_backup, r15_backup
GLOBAL cri_rip, cri_rflags, cri_rsp

EXTERN irqDispatcher
EXTERN syscallDispatcher
EXTERN exceptionDispatcher

SECTION .text

%macro pushState 0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popState 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

; don't push/pop rax, to be able to return stuff
%macro pushStateBesidesReturn 0
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popStateBesidesReturn 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
%endmacro


%macro irqHandlerMaster 1
	pushState

	mov rdi, %1 ; pasaje de parametro
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState
	iretq
%endmacro

%macro makeBackup 0
	mov [rax_backup], rax
	mov [rbx_backup], rbx
	mov [rcx_backup], rcx
	mov [rdx_backup], rdx
	mov [rbp_backup], rbp
	mov [rdi_backup], rdi
	mov [rsi_backup], rsi
	mov [r8_backup], r8
	mov [r9_backup], r9
	mov [r10_backup], r10
	mov [r11_backup], r11
	mov [r12_backup], r12
	mov [r13_backup], r13
	mov [r14_backup], r14
	mov [r15_backup], r15

	mov rax, [rsp]
	mov [cri_rip], rax
	mov rax, [rsp+16]
	mov [cri_rflags], rax
	mov rax, [rsp+24]
	mov [cri_rsp], rax

	mov rax, [rax_backup]
%endmacro

; acá supongo que deberíamos desactivar las interrupciones
%macro exceptionHandler 1
	cli
	makeBackup
	pushState

	mov rdi, %1 ; pasaje de parametro
	call exceptionDispatcher

	popState
	iretq
%endmacro


_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
    pop rbp
    retn

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
    pop     rbp
    retn


;8254 Timer (Timer Tick)
_irq00Handler:
	irqHandlerMaster 0

;Keyboard
_irq01Handler:
	irqHandlerMaster 1

;Cascade pic never called
_irq02Handler:
	irqHandlerMaster 2

;Serial Port 2 and 4
_irq03Handler:
	irqHandlerMaster 3

;Serial Port 1 and 3
_irq04Handler:
	irqHandlerMaster 4

;USB
_irq05Handler:
	irqHandlerMaster 5

;RTC
_irq08Handler:
	irqHandlerMaster 8

;Syscall
_irq80Handler:
	pushState

	call syscallDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState
	iretq

;Zero Division Exception
_exception0Handler:
	exceptionHandler 0

;Invalid Opcode Exception
_exception6Handler:
	exceptionHandler 6

haltcpu:
	cli
	hlt
	ret


SECTION .data 
; Registers backup (84 bytes)
	rax_backup dq 0
	rbx_backup dq 0
	rcx_backup dq 0
	rdx_backup dq 0
	rbp_backup dq 0
	rdi_backup dq 0
	rsi_backup dq 0
	r8_backup dq 0
	r9_backup dq 0
	r10_backup dq 0
	r11_backup dq 0
	r12_backup dq 0
	r13_backup dq 0
	r14_backup dq 0
	r15_backup dq 0

; CRI backup
	cri_rip dq 0
	cri_rflags dq 0
	cri_rsp dq 0

SECTION .bss
	aux resq 1

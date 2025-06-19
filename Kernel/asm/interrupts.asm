
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
GLOBAL _irq0CHandler

GLOBAL _irq80Handler

GLOBAL _exception0Handler
GLOBAL _exception1Handler
GLOBAL _exception2Handler
GLOBAL _exception3Handler
GLOBAL _exception4Handler
GLOBAL _exception5Handler
GLOBAL _exception6Handler
GLOBAL _exception7Handler
GLOBAL _exception8Handler
GLOBAL _exception9Handler
GLOBAL _exception10Handler
GLOBAL _exception11Handler
GLOBAL _exception12Handler
GLOBAL _exception13Handler
GLOBAL _exception14Handler
GLOBAL _exception15Handler
GLOBAL _exception16Handler
GLOBAL _exception17Handler
GLOBAL _exception18Handler
GLOBAL _exception19Handler
GLOBAL _exception20Handler
GLOBAL _exception21Handler
GLOBAL _exception22Handler
GLOBAL _exception23Handler
GLOBAL _exception24Handler
GLOBAL _exception25Handler
GLOBAL _exception26Handler
GLOBAL _exception27Handler
GLOBAL _exception28Handler
GLOBAL _exception29Handler
GLOBAL _exception30Handler
GLOBAL _exception31Handler

GLOBAL rax_backup, rbx_backup, rcx_backup, rdx_backup, rsp_backup, rbp_backup, rdi_backup, rsi_backup, r8_backup, r9_backup, r10_backup, r11_backup, r12_backup, r13_backup, r14_backup, r15_backup
GLOBAL cri_rip, cri_rflags, cri_rsp

EXTERN irqDispatcher
EXTERN syscallDispatcher
EXTERN exceptionDispatcher
EXTERN saveRegisters
EXTERN backupCurrentProcessRegisters
EXTERN log_decimal
EXTERN panic_if_ints_enabled

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
	makeBackup
;	pushState
;
;  	mov rdi, log_msg_int
;  	mov rsi, rax
;  	call log_decimal
;	call panic_if_ints_enabled
;
;	popState
	pushState

	call saveRegisters
	call backupCurrentProcessRegisters

	popState
	pushState

	mov rdi, %1 ; pasaje de parametro
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState
	iretq
%endmacro


%macro irqHandlerSlave 1
	makeBackup
	pushState
	call saveRegisters
	call backupCurrentProcessRegisters

	popState
	pushState

	mov rdi, %1
	call irqDispatcher

	; EOI para esclavo y luego maestro
	mov al, 0x20
	out 0xA0, al       ; PIC esclavo
	out 0x20, al       ; PIC maestro

	popState
	iretq
%endmacro


%macro makeBackup 0
	mov [rax_backup], rax
	mov [rbx_backup], rbx
	mov [rcx_backup], rcx
	mov [rdx_backup], rdx
	mov [rsp_backup], rsp
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
	; makeBackup
	
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
	irqHandlerSlave 8

; Mouse
_irq0CHandler:
	irqHandlerSlave 12


;Syscall
_irq80Handler:
	makeBackup
;	pushState
;
;  	mov rdi, log_msg_syscall
;  	mov rsi, rax
;  	call log_decimal
;	call panic_if_ints_enabled
;
;	popState
	pushState

	call saveRegisters
	call backupCurrentProcessRegisters
	
	popState
	pushStateBesidesReturn

	call syscallDispatcher
	mov rdi, rax

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	mov rax, rdi

	popStateBesidesReturn
;	pushState
;
;  	mov rdi, log_msg_sys_ret
;  	mov rsi, rax
;  	call log_decimal
;
;	popState
	iretq

;Zero Division Exception
_exception0Handler:
	exceptionHandler 0

;Debug Exception
_exception1Handler:
	exceptionHandler 1

;Non Maskable Interrupt
_exception2Handler:
	exceptionHandler 2

;Breakpoint Exception
_exception3Handler:
	exceptionHandler 3

;Overflow Exception
_exception4Handler:
	exceptionHandler 4

;Bound Range Exceeded Exception
_exception5Handler:
	exceptionHandler 5

;Invalid Opcode Exception
_exception6Handler:
	exceptionHandler 6

;Device Not Available Exception
_exception7Handler:
	exceptionHandler 7

;Double Fault Exception
_exception8Handler:
	exceptionHandler 8

;Coprocessor Segment Overrun
_exception9Handler:
	exceptionHandler 9

;Invalid TSS Exception
_exception10Handler:
	exceptionHandler 10

;Segment Not Present Exception
_exception11Handler:
	exceptionHandler 11

;Stack-Segment Fault Exception
_exception12Handler:
	exceptionHandler 12

;General Protection Fault Exception
_exception13Handler:
	exceptionHandler 13

;Page Fault Exception
_exception14Handler:
	exceptionHandler 14

;Reserved
_exception15Handler:
	exceptionHandler 15

;x87 Floating-Point Exception
_exception16Handler:
	exceptionHandler 16

;Alignment Check Exception
_exception17Handler:
	exceptionHandler 17

;Machine Check Exception
_exception18Handler:
	exceptionHandler 18

;SIMD Floating-Point Exception
_exception19Handler:
	exceptionHandler 19

;Virtualization Exception
_exception20Handler:
	exceptionHandler 20

;Control Protection Exception
_exception21Handler:
	exceptionHandler 21

;Reserved
_exception22Handler:
	exceptionHandler 22

;Reserved
_exception23Handler:
	exceptionHandler 23

;Reserved
_exception24Handler:
	exceptionHandler 24

;Reserved
_exception25Handler:
	exceptionHandler 25

;Reserved
_exception26Handler:
	exceptionHandler 26

;Reserved
_exception27Handler:
	exceptionHandler 27

;Reserved
_exception28Handler:
	exceptionHandler 28

;Reserved
_exception29Handler:
	exceptionHandler 29

;Reserved
_exception30Handler:
	exceptionHandler 30

;Reserved
_exception31Handler:
	exceptionHandler 31

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
	rsp_backup dq 0
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

SECTION .data
	log_msg_sys_ret db "Syscall returned: ", 0
	log_msg_int db "IRQ Handler called with value: ", 0
	log_msg_syscall db "Syscall Handler called with value: ", 0

SECTION .bss
	aux resq 1

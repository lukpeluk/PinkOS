section .text

GLOBAL init_serial, test_serial, read_serial, write_serial

; Serial port initialization routine
init_serial:
    ; Set baud rate to 9600
    mov dx, 0x3FB    ; Line Control Register (LCR) - COM1
    mov al, 0x80     ; Enable DLAB (Divisor Latch Access Bit)
    out dx, al
    mov dx, 0x3F8    ; Divisor Latch (Low Byte)
    mov al, 0x03     ; Low byte of divisor (9600 baud)
    out dx, al
    mov dx, 0x3F9    ; Divisor Latch (High Byte)
    mov al, 0x00     ; High byte of divisor
    out dx, al

    ; Set data bits to 8, stop bits to 1, parity to none
    mov dx, 0x3FB    ; Line Control Register (LCR)
    mov al, 0x03     ; 8 data bits, 1 stop bit, no parity
    out dx, al

    ; Enable FIFO buffer
    mov dx, 0x3F2    ; FIFO Control Register (FCR)
    mov al, 0x01     ; Enable FIFO, clear receive FIFO
    out dx, al

    ; Optionally, enable interrupts for data received
    mov al, 0x01     ; Enable Received Data Available Interrupt
    mov dx, 0x3F9    ; COM1 Interrupt Enable Register (IER)
    out dx, al
 
    ; Done with initialization
    ret


; Define the handler for IRQ3 (COM1)
; Returns
read_serial:
    xor rax, rax
    ; Handle received data from COM1
    mov dx, 0x3F8  ; COM1 Data Register
    in al, dx      ; Read the received byte into AL
    
    ret

; Write a byte to the serial port, received in RDI
write_serial:
    mov dx, 0x3F8  ; COM1 Data Register
    mov al, dil    ; Character to send
    out dx, al     ; Send the character to the serial port
     
    ret


test_serial:
    mov dx, 0x3F8    ; COM1 Data Register
    mov al, 'A'      ; Character to send
    out dx, al       ; Send the character to the serial port
    ret
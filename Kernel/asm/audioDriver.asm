global play_sound, stop_sound


section .text
play_sound:
    push rbp
    mov rbp, rsp
    cli

    mov rax, 1193180
    mov rdx, 0
    div rdi

    mov rcx, rax
    mov rax, 10110110b ; Accessing channel 2, lobyte/hibyte, square wave, 16-bit binary
    out 0x43, al
    mov rax, rcx
    out 0x42, al       ; Passing low byte of frequency
    mov al, ah
    out 0x42, al       ; Passing high byte of frequency

    ; Enable the speaker after setting the full frequency
    in al, 0x61
    or al, 3
    out 0x61, al
    
    sti
    mov rsp, rbp
    pop rbp
    ret

stop_sound:
    push rbp
    mov rbp, rsp

    in al, 0x61
    and al, 0xFC
    out 0x61, al

    mov rsp, rbp
    pop rbp
    ret

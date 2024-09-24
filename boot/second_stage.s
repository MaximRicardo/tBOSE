[org 0x7e00]

segment .text

second_stage_start:

    mov si, msg
    call PrintStr

halt_loop:
    hlt
    jmp halt_loop

;Prints a null-terminated string
;Pointer to the string is must be passed in the si register
;Does not preserve any registers
PrintStr:
    
    .PrintStr_Loop:
        mov al, [si]  ;Move the current character into al to be printed by the BIOS
        
        ;Stop at a null terminator
        cmp al, 0
        je .PrintStr_LoopEnd

        ;Setup the BIOS interrupt argument
        mov ah, 0x0e
        ;Set the page to 0
        xor bx, bx
        ;Print the character
        int 0x10

        ;Move to the next character
        inc si

        jmp .PrintStr_Loop

    .PrintStr_LoopEnd:

    ret

msg: db "Second bootloader stage is running!", 0x0a, 0x0d, 0x0

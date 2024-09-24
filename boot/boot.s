[org 0x7c00]

segment .text

start:
    ;Setup the segment registers
    xor ax, ax
    mov es, ax
    mov ds, ax

    mov [BOOT_DISK], dl

    ;Setup the stack
    mov bp, 0x8000
    mov sp, bp

    ;Print the msg
    mov si, msg
    call print_str

halt_loop:
    hlt
    jmp halt_loop

;Prints a null-terminated string
;Pointer to the string is must be passed in the si register
;Does not preserve any registers
print_str:
    
    .print_str_loop:
        mov al, [si]  ;Move the current character into al to be printed by the BIOS
        
        ;Stop at a null terminator
        cmp al, 0
        je .print_str_loop_end

        ;Setup the BIOS interrupt argument
        mov ah, 0x0e
        ;Set the page to 0
        xor bx, bx
        ;Print the character
        int 0x10

        ;Move to the next character
        inc si

        jmp .print_str_loop

    .print_str_loop_end:

    mov sp, bp
    pop bp
    ret

BOOT_DISK: db 0

msg: db "Hello world!", 0x0a, 0x0d, 0x0

times 510-($-$$) db 0
dw 0xaa55

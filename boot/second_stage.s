[org 0x7e00]

CODE_SEG equ GDT_CodeDescriptor - GDT_Start
DATA_SEG equ GDT_DataDescriptor - GDT_Start

segment .text

SecondStageStart:

    mov si, msg
    call PrintStr

    ;Enable A20 if it isn't enabled already
    call CheckA20
    cmp  ax, 0
    jne  EnableA20Done
    call SetA20BIOS
    call CheckA20
    cmp  ax, 0
    jne  EnableA20Done
    call SetA20Keyboard
    call CheckA20
    cmp  ax, 0
    jne  EnableA20Done
    call SetA20FastGate
    call CheckA20
    xchg bx, bx
    cmp  ax, 0
    jne  EnableA20Done

EnableA20Fail:
    ;If A20 line couldn't be enabled, halt the program
    mov si, enabling_a_20_line_failed_msg
    call PrintStr
    jmp HaltLoop

EnableA20Done:
    mov si, enabling_a_20_line_success_msg
    call PrintStr

    ;Enter protected mode in order to start running the kernel

    mov si, entering_protected_mode_msg
    call PrintStr

    jmp EnterProtectedMode

HaltLoop:
    hlt
    jmp HaltLoop


SetA20BIOS:
    mov ax, 0x2401
    int 0x15
    ret

SetA20Keyboard:
    cli                         ; Disable interrupts

    call    Wait8042Command     ; When controller ready for command
    mov     al,0xAD             ; Send command 0xad (disable keyboard).
    out     0x64,al

    call    Wait8042Command     ; When controller ready for command
    mov     al,0xD0             ; Send command 0xd0 (read from input)
    out     0x64,al

    call    Wait8042Data        ; When controller has data ready
    in      al,0x60             ; Read input from keyboard
    push    eax                 ; ... and save it

    call    Wait8042Command     ; When controller is ready for command
    mov     al,0xD1             ; Set command 0xd1 (write to output)
    out     0x64,al            

    call    Wait8042Command     ; When controller is ready for command
    pop     eax                 ; Write input back, with bit #2 set
    or      al,2
    out     0x60,al

    call    Wait8042Command     ; When controller is ready for command
    mov     al,0xAE             ; Write command 0xae (enable keyboard)
    out     0x64,al

    call    Wait8042Command     ; Wait until controller is ready for command

    sti                         ; Enable interrupts
    ret

    
Wait8042Command:
    in      al,0x64
    test    al,2
    jnz     Wait8042Command
    ret


Wait8042Data:
    in      al,0x64
    test    al,1
    jz      Wait8042Data
    ret


SetA20FastGate:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret


CheckA20:
    pushf                           ; Save registers that
    push ds                         ; we are going to
    push es                         ; overwrite.
    push di
    push si

    cli                             ; No interrupts, please

    xor ax, ax                      ; Set es:di = 0000:0500
    mov es, ax
    mov di, 0x0500

    mov ax, 0xffff                  ; Set ds:si = ffff:0510
    mov ds, ax
    mov si, 0x0510

    mov al, byte es:[di]            ; Save byte at es:di on stack.
    push ax                         ; (we want to restore it later)

    mov al, byte ds:[si]            ; Save byte at ds:si on stack.
    push ax                         ; (we want to restore it later)

    mov byte es:[di], 0x00          ; [es:di] = 0x00
    mov byte ds:[si], 0xFF          ; [ds:si] = 0xff

    cmp byte es:[di], 0xFF          ; Did memory wrap around?

    pop ax
    mov byte ds:[si], al            ; Restore byte at ds:si

    pop ax
    mov byte es:[di], al            ; Restore byte at es:di

    mov ax, 0
    je .CheckA20Exit                ; If memory wrapped around, return 0.

    mov ax, 1                       ; else return 1.

.CheckA20Exit:
    pop si                          ; Restore saved registers.
    pop di
    pop es
    pop ds
    popf
    ret

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

enabling_a_20_line_failed_msg:
    db "ERROR: Couldn't enable A20 line.", 0x0a, 0x0d, 0x0

enabling_a_20_line_success_msg: 
    db "Enabled A20 Line!", 0x0a, 0x0d, 0x0

entering_protected_mode_msg:
    db "Entering 32-bit Protected mode...", 0x0a, 0x0d, 0x0


;A dummy IDT so the processor won't complain when entering protected mode
IDT_Descriptor:
    dw 0
    dd 0

GDT_Start:
    GDT_nullDescriptor:
        dd 0    ;All zeroes
        dd 0
    GDT_CodeDescriptor:
        dw 0xffff   ;First 16 bits of the limit
        dw 0x0      ;First 24 bits of the base
        db 0x0
        db 0b10011010   ;Descriptor and type flags
        db 0b11001111   ;Other flags and last four bits of limit
        db 0            ;Last 8 bits of base
    GDT_DataDescriptor:
        dw 0xffff   ;First 16 bits of the limit
        dw 0x0      ;First 24 bits of the base
        db 0x0
        db 0b10010010   ;Descriptor and type flags
        db 0b11001111   ;Other flags and last four bits of limit
        db 0            ;Last 8 bits of base
    GDT_End:

    GDT_Descriptor:
        dw GDT_End - GDT_Start - 1  ;Size
        dd GDT_Start                ;Start
    
EnterProtectedMode:
    ;#############################################
    ;#  NO MORE BIOS INTERRUPTS PAST THIS POINT  #
    ;#############################################
    ;Switch into 32 bit protected mode
    cli
    lidt [IDT_Descriptor]
    lgdt [GDT_Descriptor]
    ;Change the least signifcant bit of cr0 to 1
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ;Far jump to the protected mode code
    jmp CODE_SEG:ProtectedModeStart
    ;Make sure the instruction prefetch is cleared by filling it with NOPs
    nop
    nop
    nop
    nop

[bits 32]
;########################################################
;#            PROTECTED MODE CODE STARTS HERE           #
;########################################################
ProtectedModeStart:
    ;Setup segment registers and the stack
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x80000
    mov esp, ebp

    mov dword[0xb8000], 0x07690748

ProtectedMode_HaltLoop:
    hlt
    jmp ProtectedMode_HaltLoop

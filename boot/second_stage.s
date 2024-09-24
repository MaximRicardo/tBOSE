;Second stage of the boot loader

[org 0x7e00]

CODE_SEG equ GDT_CodeDescriptor - GDT_Start
DATA_SEG equ GDT_DataDescriptor - GDT_Start

%define KERNEL_LOCATION 0x1000
%define KERNEL_SECTORS_TO_READ 40

%define SECOND_STAGE_SIZE_IN_SECTORS 4

segment .text

SecondStageStart:

    ;The first stage loads the boot disk into dl, right before entering the
    ;second stage
    mov [BOOT_DISK], dl

    mov si, msg
    call PrintStr

    mov si, loading_kernel_msg
    call PrintStr

LoadKernel:
    ;Load in the kernel from disk
    mov bx, KERNEL_LOCATION
    mov ah, 2
    mov al, KERNEL_SECTORS_TO_READ
    mov ch, 0
    mov cl, 2+SECOND_STAGE_SIZE_IN_SECTORS
    mov dh, 0
    mov dl, [BOOT_DISK]
    int 0x13

    jc KernelLoadError
    cmp al, KERNEL_SECTORS_TO_READ
    jne KernelLoadError

    mov si, loading_kernel_success_msg
    call PrintStr

    jmp KernelLoadSuccess

KernelLoadError:
    ;Try to load the kernel 4 times before giving up
    inc byte[n_kernel_load_attemps]
    mov al, [n_kernel_load_attemps]
    cmp al, 4
    jle LoadKernel

    mov si, loading_kernel_failed_msg
    call PrintStr

    jmp HaltLoop

KernelLoadSuccess:

    ;Get VESA BIOS information
   ; mov ax, 0x4f00
   ; lea di, [vbe_info_struct.signature]
   ; int 0x10
   ; 
   ; cmp ax, 0x004f
   ; jne VESANotSupported

   ; mov si, vesa_valid_signature
   ; mov di, vbe_info_struct ;The signature is the first item in the struct
   ; mov cx, 4
   ; rep cmpsb
   ; jne VESANotSupported  ;If the signatures are different, then VESA is not supported

   ; ;Check if vesa mode 0x0100 is supported
   ; mov ax, 0x0100
   ; call IsVideoModeSupported
   ; ;If it is not supported, then RIP you.
   ; cmp ax, 0
   ; je VESANotSupported

   ; ;Get VBE mode info
   ; mov ax, 0x4f01
   ; mov cx, 0x0100
   ; lea di, [vbe_mode_info_struct]
   ; int 0x10

   ; cmp ax, 0x004f
   ; jne VESANotSupported

   ; ;Make sure the VBE mode supports a linear frame buffer
   ; ;Whether or not it does is stored in the 7th bit of vbe_mode_info_struct.attributes
   ; mov al, [vbe_mode_info_struct.attributes]
   ; cmp al, 0x80
   ; jl VESANotSupported ;If the 7th bit is off, then this VBE mode does not support a linear frame buffer

   ; ;Finally, set the VBE mode
   ; mov ax, 0x4f02
   ; mov bx, 0x4100  ;Mode 0x0100
   ; int 0x10

   ; cmp ax, 0x004f
   ; jne VESANotSupported

    mov ax, 640
    mov bx, 480,
    mov cl, 32
    call vbe_set_mode
    jc VESANotSupported

    ;Note that printing now does nothing, since a graphics mode has now been enabled

    jmp EnableA20Line

EnableA20Line:
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
    jmp HaltLoop

EnableA20Done:
    ;Enter protected mode in order to start running the kernel
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

;Checks if VESA video mode is supported
;video mode is passed in ax.
;returns 0 if it is not supported, and a non-zero number if it is supported
;Preserves segment registers
IsVideoModeSupported:

    push ds

    ;Set cx to false
    mov cx, 0
    
    ;Point ds:si at the array of video modes
    mov si, word[vbe_info_struct.video_modes]   ;Low word (offset)
    mov ds, word[vbe_info_struct.video_modes+2] ;High word (segment)

    .find_video_mode_loop:
        mov bx, [ds:si]
        
        cmp bx, 0xffff  ;This denotes the end of the array
        je .find_video_mode_loop_end

        cmp ax, bx
        je .video_mode_supported

        ;Move to the next element
        add si, 2

        jmp .find_video_mode_loop

    .video_mode_supported:
        mov cx, 1
        jmp .find_video_mode_loop_end

    .find_video_mode_loop_end:
    
    pop ds

    ;Move the result into ax
    mov ax, cx

    ret

VESANotSupported:
    mov si, vesa_not_supported_msg
    call PrintStr

    jmp HaltLoop

;Prints a null-terminated string
;Pointer to the string is must be passed in the si register
;Preserves segment registers
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

;Code from: https://wiki.osdev.org/User:Omarrx024/VESA_Tutorial
; vbe_set_mode:
; Sets a VESA mode
; In\	AX = Width
; In\	BX = Height
; In\	CL = Bits per pixel
; Out\	FLAGS = Carry clear on success
; Out\	Width, height, bpp, physical buffer, all set in vbe_screen structure
vbe_set_mode:
	mov [.width], ax
	mov [.height], bx
	mov [.bpp], cl

	sti

	push es					; some VESA BIOSes destroy ES, or so I read
	mov ax, 0x4F00				; get VBE BIOS info
	mov di, vbe_info_struct
	int 0x10
	pop es

	cmp ax, 0x4F				; BIOS doesn't support VBE?
	jne .error

	mov ax, word[vbe_info_struct.video_modes]
	mov [.offset], ax
	mov ax, word[vbe_info_struct.video_modes+2]
	mov [.segment], ax

	mov ax, [.segment]
	mov fs, ax
	mov si, [.offset]

.find_mode:
	mov dx, [fs:si]
	add si, 2
	mov [.offset], si
	mov [.mode], dx
	mov ax, 0
	mov fs, ax

	cmp word[.mode], 0xFFFF			; end of list?
	je .error

	push es
	mov ax, 0x4F01				; get VBE mode info
	mov cx, [.mode]
	mov di, vbe_mode_info_struct
	int 0x10
	pop es

	cmp ax, 0x4F
	jne .error

	mov ax, [.width]
	cmp ax, [vbe_mode_info_struct.width]
	jne .next_mode

	mov ax, [.height]
	cmp ax, [vbe_mode_info_struct.height]
	jne .next_mode

	mov al, [.bpp]
	cmp al, [vbe_mode_info_struct.bpp]
	jne .next_mode

	; If we make it here, we've found the correct mode!
	mov ax, [.width]
	;mov word[vbe_screen.width], ax
	mov ax, [.height]
	;mov word[vbe_screen.height], ax
	mov eax, [vbe_mode_info_struct.framebuffer]
	;mov dword[vbe_screen.physical_buffer], eax
	mov ax, [vbe_mode_info_struct.pitch]
	;mov word[vbe_screen.bytes_per_line], ax
	mov eax, 0
	mov al, [.bpp]
	;mov byte[vbe_screen.bpp], al
	shr eax, 3
	;mov dword[vbe_screen.bytes_per_pixel], eax

	mov ax, [.width]
	shr ax, 3
	dec ax
	;mov word[vbe_screen.x_cur_max], ax

	mov ax, [.height]
	shr ax, 4
	dec ax
	;mov word[vbe_screen.y_cur_max], ax

	; Set the mode
	push es
	mov ax, 0x4F02
	mov bx, [.mode]
	or bx, 0x4000			; enable LFB
	mov di, 0			; not sure if some BIOSes need this... anyway it doesn't hurt
	int 0x10
	pop es

	cmp ax, 0x4F
	jne .error

	clc
	ret

.next_mode:
	mov ax, [.segment]
	mov fs, ax
	mov si, [.offset]
	jmp .find_mode

.error:
	stc
	ret

.width				dw 0
.height				dw 0
.bpp				db 0
.segment			dw 0
.offset				dw 0
.mode				dw 0

BOOT_DISK: db 0

;Number of times the program has attempted to load the kernel without success
n_kernel_load_attemps: db 0

msg: db "Second bootloader stage is running!", 0x0a, 0x0d, 0x0

vesa_not_supported_msg:
    db "ERROR: Either VESA is not supported by this machine, or it does not support mode 0x0100!", 0x0a, 0x0d, 0x0

loading_kernel_msg:
    db "Loading kernel from disk...", 0x0a, 0x0d, 0x0

loading_kernel_failed_msg:
    db "ERROR: Couldn't load kernel from disk!", 0x0a, 0x0d, 0x0

loading_kernel_success_msg:
    db "Successfully loaded the kernel from disk!", 0x0a, 0x0d, 0x0


vesa_valid_signature:
    db "VESA"

vbe_info_struct:
    .signature: db "VBE2"
    .version: dw 0
    .oem: dd 0
    .capabilities: dd 0
    .video_modes: dd 0
    .video_memory: dw 0
    .software_rev: dw 0
    .vendor: dd 0
    .product_name: dd 0
    .product_rev: dd 0
    .reserved: times 222 db 0
    .oem_data: times 256 db 0

vbe_mode_info_struct:
    .attributes: dw 0
    .window_a: db 0
    .window_b: db 0
    .granularity: dw 0
    .window_size: dw 0
    .segment_a: dw 0
    .segment_b: dw 0
    .win_func_ptr: dd 0
    .pitch: dw 0
    .width: dw 0
    .height: dw 0
    .w_char: db 0
    .y_char: db 0
    .planes: db 0
    .bpp: db 0
    .banks: db 0
    .memory_model: db 0
    .bank_size: db 0
    .image_pages: db 0
    .reserved0: db 0
    
    .red_mask: db 0
    .red_position: db 0
    .green_mask: db 0
    .green_position: db 0
    .blue_mask: db 0
    .blue_position: db 0
    .reserved_mask: db 0
    .reserved_position: db 0
    .direct_color_attributes: db 0

    .framebuffer: dd 0
    .off_screen_mem_off: dd 0
    .off_screen_mem_size: dw 0
    .reserved1: times 206 db 0


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

    ;Pass the address of the VBE information structs as arguments to the kernel. This allows it to know, for example, the address of the frame buffer.
    lea eax, [vbe_info_struct]  ;eax holds the vbe_info
    lea ebx, [vbe_mode_info_struct] ;ebx holds the vbe_mode_info

    jmp KERNEL_LOCATION ;Enter the kernel (Finally!)

;Make the stage exactly 2 sectors large
;This makes it easier to load in the kernel, since this makes the start of the kernel always be at a multiple of 512 bytes in disk
times 512*SECOND_STAGE_SIZE_IN_SECTORS-($-$$) db 0

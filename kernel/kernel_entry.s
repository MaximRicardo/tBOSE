[bits 32]

extern k_main

segment .text.kernel_entry

;Get the linker to stfu about not finding _start
global _start

;Start of the kernel
;The boot loader hands off control to the kernel from here 
;NOTE: No return address is on the stack here. Therefore, to access arguments via the ebp, the return address must not be accounted for
_start:

    push ebp
    mov ebp, esp

    mov [vbe_info_ptr], eax
    mov [vbe_mode_info_ptr], ebx
    mov [boot_disk], ecx

    ;init the FPU
    fninit
    fldcw [fcw]

    ;Jump into the kernel's main function (Which is actually written in C! Yay)
    push dword[boot_disk]
    push dword[vbe_mode_info_ptr]
    push dword[vbe_info_ptr]
    call k_main

    ;k_main never returns, so no point in destroying the stack frame

HaltLoop:
    hlt
    jmp HaltLoop

segment .rodata
fcw: dw 0x037f

segment .data

vbe_info_ptr: dd 0
vbe_mode_info_ptr: dd 0
boot_disk:  dd 0

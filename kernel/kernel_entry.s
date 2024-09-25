[bits 32]

;Get the linker to stfu about not finding _start
global _start

extern k_main

segment .text

;Start of the kernel
;The boot loader hands off control to the kernel from here 
;NOTE: No return address is on the stack here. Therefore, to access arguments via the ebp, the return address must not be accounted for
_start:

    push ebp
    mov ebp, esp

    mov [VBEInfoPtr], eax
    mov [VBEModeInfoPtr], ebx
    
    ;init the FPU
    fninit
    fldcw [fcw]

    ;Jump into the kernel's main function (Which is actually written in C! Yay)
    ;One last thing before that, which is to pass the pointers to VBE info that was passed here by the boot loader, through to k_main.
    push dword[VBEModeInfoPtr]
    push dword[VBEInfoPtr]
    call k_main

    ;k_main never returns, so no point in destroying the stack frame

HaltLoop:
    hlt
    jmp _start

fcw: dw 0x037f

VBEInfoPtr: dd 0
VBEModeInfoPtr: dd 0

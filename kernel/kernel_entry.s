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

    ;Jump into the kernel's main function (Which is actually written in C! Yay)
    ;One last thing before that, which is to pass the pointers to VBE info that was passed here by the boot loader, through to k_main.
    ;mov eax, [ebp+4]
    ;push eax  ;vbe_mode_info_struct
    ;mov eax, [ebp+8]
    ;push eax  ;vbe_info_struct
    push ebx
    push eax
    call k_main

    ;k_main never returns, so no point in destroying the stack frame

HaltLoop:
    hlt
    jmp _start

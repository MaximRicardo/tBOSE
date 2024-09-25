[bits 32]

extern INTERRUPT_default_handler

section .text

global INTERRUPT_default

;Pushes every register the interrupt handler might modify
%macro m_PUSH_HANDLER_MODIFIED_REGS 0
    push eax
    push ecx
    push edx
%endmacro

;Pops every register the interrupt handler might modify
%macro m_POP_HANDLER_MODIFIED_REGS 0
    pop edx
    pop ecx
    pop eax
%endmacro

INTERRUPT_default:

    m_PUSH_HANDLER_MODIFIED_REGS
    push dword[esp]   ;Pass the address where the interrupt happened as an argument
    call INTERRUPT_default_handler
    add esp, 4
    m_POP_HANDLER_MODIFIED_REGS

    jmp HaltLoop

HaltLoop:
    hlt
    jmp HaltLoop

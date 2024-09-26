[bits 32]

extern INTERRUPT_default_handler

section .text.interrupts

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

    push ebp

    m_PUSH_HANDLER_MODIFIED_REGS
    push dword[ebp+4] ;Pass the address where the interrupt happened as an argument. This works by using the return address on the stack
    call INTERRUPT_default_handler
    add esp, 4
    m_POP_HANDLER_MODIFIED_REGS

    pop ebp

    jmp HaltLoop

HaltLoop:
    hlt
    jmp HaltLoop

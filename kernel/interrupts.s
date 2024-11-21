[bits 32]

extern INTERRUPT_default_handler

section .text.interrupts

global INTERRUPT_0
global INTERRUPT_1
global INTERRUPT_2
global INTERRUPT_3
global INTERRUPT_4
global INTERRUPT_5
global INTERRUPT_6
global INTERRUPT_7
global INTERRUPT_8
global INTERRUPT_9
global INTERRUPT_10
global INTERRUPT_11
global INTERRUPT_12
global INTERRUPT_13
global INTERRUPT_14
global INTERRUPT_15
global INTERRUPT_16
global INTERRUPT_17
global INTERRUPT_18
global INTERRUPT_19
global INTERRUPT_20
global INTERRUPT_21
global INTERRUPT_22
global INTERRUPT_23
global INTERRUPT_24
global INTERRUPT_25
global INTERRUPT_26
global INTERRUPT_27
global INTERRUPT_28
global INTERRUPT_29
global INTERRUPT_30
global INTERRUPT_31

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

INTERRUPT_0:
    inc dword[int_num]
INTERRUPT_1:
    inc dword[int_num]
INTERRUPT_2:
    inc dword[int_num]
INTERRUPT_3:
    inc dword[int_num]
INTERRUPT_4:
    inc dword[int_num]
INTERRUPT_5:
    inc dword[int_num]
INTERRUPT_6:
    inc dword[int_num]
INTERRUPT_7:
    inc dword[int_num]
INTERRUPT_8:
    inc dword[int_num]
INTERRUPT_9:
    inc dword[int_num]
INTERRUPT_10:
    inc dword[int_num]
INTERRUPT_11:
    inc dword[int_num]
INTERRUPT_12:
    inc dword[int_num]
INTERRUPT_13:
    inc dword[int_num]
INTERRUPT_14:
    inc dword[int_num]
INTERRUPT_15:
    inc dword[int_num]
INTERRUPT_16:
    inc dword[int_num]
INTERRUPT_17:
    inc dword[int_num]
INTERRUPT_18:
    inc dword[int_num]
INTERRUPT_19:
    inc dword[int_num]
INTERRUPT_20:
    inc dword[int_num]
INTERRUPT_21:
    inc dword[int_num]
INTERRUPT_22:
    inc dword[int_num]
INTERRUPT_23:
    inc dword[int_num]
INTERRUPT_24:
    inc dword[int_num]
INTERRUPT_25:
    inc dword[int_num]
INTERRUPT_26:
    inc dword[int_num]
INTERRUPT_27:
    inc dword[int_num]
INTERRUPT_28:
    inc dword[int_num]
INTERRUPT_29:
    inc dword[int_num]
INTERRUPT_30:
    inc dword[int_num]
INTERRUPT_31:
    inc dword[int_num]

    push ebp

    m_PUSH_HANDLER_MODIFIED_REGS

    mov eax, 32
    sub eax, [int_num]
    push eax
    push dword[ebp+4] ;Pass the address where the interrupt happened as an argument. This works by using the return address on the stack
    call INTERRUPT_default_handler
    add esp, 4

    m_POP_HANDLER_MODIFIED_REGS

    pop ebp

    jmp HaltLoop

HaltLoop:
    hlt
    jmp HaltLoop

section .data
    
int_num: dd 0

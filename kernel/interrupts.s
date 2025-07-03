[bits 32]

extern Interrupt_default_handler
extern Interrupt_pit_int_handler
extern Interrupt_others_handler

section .text.interrupts

global Interrupt_0
global Interrupt_1
global Interrupt_2
global Interrupt_3
global Interrupt_4
global Interrupt_5
global Interrupt_6
global Interrupt_7
global Interrupt_8
global Interrupt_9
global Interrupt_10
global Interrupt_11
global Interrupt_12
global Interrupt_13
global Interrupt_14
global Interrupt_15
global Interrupt_16
global Interrupt_17
global Interrupt_18
global Interrupt_19
global Interrupt_20
global Interrupt_21
global Interrupt_22
global Interrupt_23
global Interrupt_24
global Interrupt_25
global Interrupt_26
global Interrupt_27
global Interrupt_28
global Interrupt_29
global Interrupt_30
global Interrupt_31
global Interrupt_32

global Interrupt_others

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

Interrupt_0:
    inc dword[int_num]
Interrupt_1:
    inc dword[int_num]
Interrupt_2:
    inc dword[int_num]
Interrupt_3:
    inc dword[int_num]
Interrupt_4:
    inc dword[int_num]
Interrupt_5:
    inc dword[int_num]
Interrupt_6:
    inc dword[int_num]
Interrupt_7:
    inc dword[int_num]
Interrupt_8:
    inc dword[int_num]
Interrupt_9:
    inc dword[int_num]
Interrupt_10:
    inc dword[int_num]
Interrupt_11:
    inc dword[int_num]
Interrupt_12:
    inc dword[int_num]
Interrupt_13:
    inc dword[int_num]
Interrupt_14:
    inc dword[int_num]
Interrupt_15:
    inc dword[int_num]
Interrupt_16:
    inc dword[int_num]
Interrupt_17:
    inc dword[int_num]
Interrupt_18:
    inc dword[int_num]
Interrupt_19:
    inc dword[int_num]
Interrupt_20:
    inc dword[int_num]
Interrupt_21:
    inc dword[int_num]
Interrupt_22:
    inc dword[int_num]
Interrupt_23:
    inc dword[int_num]
Interrupt_24:
    inc dword[int_num]
Interrupt_25:
    inc dword[int_num]
Interrupt_26:
    inc dword[int_num]
Interrupt_27:
    inc dword[int_num]
Interrupt_28:
    inc dword[int_num]
Interrupt_29:
    inc dword[int_num]
Interrupt_30:
    inc dword[int_num]
Interrupt_31:
    inc dword[int_num]

    pushad

    mov eax, 32
    sub eax, [int_num]
    push eax
    push dword[ebp+4] ;Pass the address where the interrupt happened as an argument. This works by using the return address on the stack
    call Interrupt_default_handler
    add esp, 4

    popad

    jmp HaltLoop

Interrupt_32:

    pushad

    call Interrupt_pit_int_handler

    popad

    iret

Interrupt_others:

    pushad

    call Interrupt_others_handler

    popad

    jmp HaltLoop

HaltLoop:
    hlt
    jmp HaltLoop

section .data
    
int_num: dd 0

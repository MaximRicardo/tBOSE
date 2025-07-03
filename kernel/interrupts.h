#pragma once

void Interrupt_0(void);
void Interrupt_1(void);
void Interrupt_2(void);
void Interrupt_3(void);
void Interrupt_4(void);
void Interrupt_5(void);
void Interrupt_6(void);
void Interrupt_7(void);
void Interrupt_8(void);
void Interrupt_9(void);
void Interrupt_10(void);
void Interrupt_11(void);
void Interrupt_12(void);
void Interrupt_13(void);
void Interrupt_14(void);
void Interrupt_15(void);
void Interrupt_16(void);
void Interrupt_17(void);
void Interrupt_18(void);
void Interrupt_19(void);
void Interrupt_20(void);
void Interrupt_21(void);
void Interrupt_22(void);
void Interrupt_23(void);
void Interrupt_24(void);
void Interrupt_25(void);
void Interrupt_26(void);
void Interrupt_27(void);
void Interrupt_28(void);
void Interrupt_29(void);
void Interrupt_30(void);
void Interrupt_31(void);
void Interrupt_32(void);
void Interrupt_others(void);

extern void (*Interrupt_jump_table[256])(void);

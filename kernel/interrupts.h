#pragma once

void INTERRUPT_0(void);
void INTERRUPT_1(void);
void INTERRUPT_2(void);
void INTERRUPT_3(void);
void INTERRUPT_4(void);
void INTERRUPT_5(void);
void INTERRUPT_6(void);
void INTERRUPT_7(void);
void INTERRUPT_8(void);
void INTERRUPT_9(void);
void INTERRUPT_10(void);
void INTERRUPT_11(void);
void INTERRUPT_12(void);
void INTERRUPT_13(void);
void INTERRUPT_14(void);
void INTERRUPT_15(void);
void INTERRUPT_16(void);
void INTERRUPT_17(void);
void INTERRUPT_18(void);
void INTERRUPT_19(void);
void INTERRUPT_20(void);
void INTERRUPT_21(void);
void INTERRUPT_22(void);
void INTERRUPT_23(void);
void INTERRUPT_24(void);
void INTERRUPT_25(void);
void INTERRUPT_26(void);
void INTERRUPT_27(void);
void INTERRUPT_28(void);
void INTERRUPT_29(void);
void INTERRUPT_30(void);
void INTERRUPT_31(void);

extern void (*INTERRUPT_jump_table[256])(void);

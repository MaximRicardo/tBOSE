#pragma once

#include <stdint.h>

/*
 * IDT = Interrupt Descriptor Table
 * Tells the CPU where to look for interrupt handlers
 */

/*
 * IDT_SEL_VALUE
 * The 0th and 1st bits are the privilege level of the selector
 * The 2nd bit is 0 for GDT andd 1 for LDT
 * The reset of the bits are the index of the code segment, which is 1
 */
#define m_IDT_SEL_VALUE 0x8 //0b1000, rest of the bits should be 0

#define m_IDT_FLAGS_VALUE 0x8e  //0b10001110

#define m_N_IDT_ENTRIES 256

struct IDT_Entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t always_0;
    uint8_t flags;
    uint16_t base_hi;
} __attribute__((packed));

//Describes the IDT itself. Is passed as the argument to "lidt"
struct IDT_Descriptor {
    uint16_t size;
    uint32_t base;
} __attribute__((packed));

//Contains everything needed for an IDT
struct IDT {
    struct IDT_Entry entries[m_N_IDT_ENTRIES];
    struct IDT_Descriptor descriptor;
} __attribute__((packed));

struct IDT_Entry IDT_create_entry(uint32_t base);
struct IDT_Entry IDT_create_zero_entry(void); //Returns a zeroed-out IDT entry

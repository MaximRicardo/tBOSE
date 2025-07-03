#pragma once

#include <stdint.h>

/*
 * GDT = Global Descriptor Table
 * Manages the different segments of code (A segment for Kernel Space, A segment for User Space, etc..)
 */

#define m_N_GDT_ENTRIES 6

#define m_SEGDESCTYPE(x) \
	((x) << 0x04) // Descriptor type (0 for system, 1 for code/data)
#define m_SEGPRES(x) ((x) << 0x07) // Present
#define m_SEGSAVL(x) ((x) << 0x0C) // Available for system use
#define m_SEGLONG(x) ((x) << 0x0D) // Long mode
#define m_SEGSIZE(x) ((x) << 0x0E) // Size (0 for 16-bit, 1 for 32)
#define m_SEGGRAN(x) \
	((x) << 0x0F) // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB)
#define m_SEGPRIV(x) (((x)&0x03) << 0x05) // Set privilege level (0 - 3)

#define m_SEGDATA_RD 0x00 // Read-Only
#define m_SEGDATA_RDA 0x01 // Read-Only, accessed
#define m_SEGDATA_RDWR 0x02 // Read/Write
#define m_SEGDATA_RDWRA 0x03 // Read/Write, accessed
#define m_SEGDATA_RDEXPD 0x04 // Read-Only, expand-down
#define m_SEGDATA_RDEXPDA 0x05 // Read-Only, expand-down, accessed
#define m_SEGDATA_RDWREXPD 0x06 // Read/Write, expand-down
#define m_SEGDATA_RDWREXPDA 0x07 // Read/Write, expand-down, accessed
#define m_SEGCODE_EX 0x08 // Execute-Only
#define m_SEGCODE_EXA 0x09 // Execute-Only, accessed
#define m_SEGCODE_EXRD 0x0A // Execute/Read
#define m_SEGCODE_EXRDA 0x0B // Execute/Read, accessed
#define m_SEGCODE_EXC 0x0C // Execute-Only, conforming
#define m_SEGCODE_EXCA 0x0D // Execute-Only, conforming, accessed
#define m_SEGCODE_EXRDC 0x0E // Execute/Read, conforming
#define m_SEGCODE_EXRDCA 0x0F // Execute/Read, conforming, accessed

#define m_GDT_CODE_KERNEL                                               \
	m_SEGDESCTYPE(1) | m_SEGPRES(1) | m_SEGSAVL(0) | m_SEGLONG(0) | \
		m_SEGSIZE(1) | m_SEGGRAN(1) | m_SEGPRIV(0) | m_SEGCODE_EXRD

#define m_GDT_DATA_KERNEL                                               \
	m_SEGDESCTYPE(1) | m_SEGPRES(1) | m_SEGSAVL(0) | m_SEGLONG(0) | \
		m_SEGSIZE(1) | m_SEGGRAN(1) | m_SEGPRIV(0) | m_SEGDATA_RDWR

#define m_GDT_CODE_USER                                                 \
	m_SEGDESCTYPE(1) | m_SEGPRES(1) | m_SEGSAVL(0) | m_SEGLONG(0) | \
		m_SEGSIZE(1) | m_SEGGRAN(1) | m_SEGPRIV(3) | m_SEGCODE_EXRD

#define m_GDT_DATA_USER                                                 \
	m_SEGDESCTYPE(1) | m_SEGPRES(1) | m_SEGSAVL(0) | m_SEGLONG(0) | \
		m_SEGSIZE(1) | m_SEGGRAN(1) | m_SEGPRIV(3) | m_SEGDATA_RDWR

struct GDT_Entry {
	uint16_t limit_bits_0_15;
	uint16_t base_bits_0_15;
	uint8_t base_bits_16_23;
	uint8_t flags_bits_0_7;
	uint8_t falgs_bits_8_11_and_limit_16_19;
	uint8_t base_bits_24_31;
} __attribute__((packed));

//Describes the GDT itself. Is passed as the argument to "lgdt"
struct GDT_Descriptor {
	uint16_t size;
	uint32_t base;
} __attribute__((packed));

//Contains everything needed for an IDT
struct GDT {
	struct GDT_Entry entries[m_N_GDT_ENTRIES];
	struct GDT_Descriptor descriptor;
} __attribute__((packed));

struct GDT_Entry GDT_create_entry(uint32_t base, uint32_t limit,
				  uint16_t flags);
struct GDT_Entry GDT_create_zero_entry(void); //Creates a zeroed-out GDT entry

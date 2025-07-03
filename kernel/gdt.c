#include <stdint.h>

#include "gdt.h"

struct GDT_Entry GDT_create_entry(uint32_t base, uint32_t limit, uint16_t flags)
{
	struct GDT_Entry gdt_entry;

	//Each GDT entry is exacty 8 bytes / 64 bits large
	uint64_t *entry = (uint64_t *)(&gdt_entry);

	*entry = limit & 0x000f0000;
	*entry |= (flags << 8) & 0x00f0ff00;
	*entry |= (base >> 16) & 0x000000ff;
	*entry |= base & 0xff000000;

	*entry <<= 32;

	*entry |= base << 16;
	*entry |= limit & 0x0000ffff;

	return gdt_entry;
}

struct GDT_Entry GDT_create_zero_entry(void)
{
	struct GDT_Entry gdt_entry;

	uint64_t *entry = (uint64_t *)(&gdt_entry);
	*entry = 0;

	return gdt_entry;
}

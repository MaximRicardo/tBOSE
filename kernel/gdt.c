#include <stdint.h>

#include "gdt.h"

struct GDTEntry GDT_create_entry(uint32_t base, uint32_t limit, uint16_t flags)
{
	struct GDTEntry gdt_entry;

	//each GDT entry is exactly 8 bytes large
	//-fno-strict-aliasing, god bless u
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

struct GDTEntry GDT_create_zero_entry(void)
{
	struct GDTEntry gdt_entry;

	uint64_t *entry = (uint64_t *)(&gdt_entry);
	*entry = 0;

	return gdt_entry;
}

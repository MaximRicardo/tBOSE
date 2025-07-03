#pragma once

#include <stdint.h>
#include <stddef.h>

#define m_MEMORY_MAP_BASE_ADDRESS 0x0500

#define m_MEMORY_MAP_ENTRY_FREE_TYPE 1

struct MemoryMapEntry {
	uint32_t base_lo; //Address of the region
	uint32_t base_hi; //Ignored since only the low 4GiB of address space will be used by the OS

	uint32_t length_lo; //Length of the region in bytes
	uint32_t length_hi;

	uint32_t type; //Region type

	uint32_t unused_field; //The ACPI 3.0 Extended Attributes. The OS will ignore this.
} __attribute__((packed));

struct MemoryMapDescriptor {
	uint16_t n_entries; //Number of entries in the memory map
} __attribute__((packed));

extern struct MemoryMapDescriptor *MemoryMap_descriptor;
extern struct MemoryMapEntry *MemoryMap_entries;

//Prepares the memory map for use (Deals with overlapping regions, regions extending past 4GiB, etc..)
void MemoryMap_set_up(void);

//Returns the type of the region location points to.
//Returns 0 if the location isn't in any entry.
unsigned MemoryMap_region_type(uint32_t location);

//Returns the amount of detected free memory, in bytes.
size_t MemoryMap_available_mem(void);

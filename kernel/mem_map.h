#pragma once

#include <stdint.h>
#include <stddef.h>

#define m_MEMORY_MAP_BASE_ADDRESS 0x0500

#define m_MEMORY_MAP_ENTRY_FREE_TYPE 1

struct MEMORY_MAP_Entry {
	uint32_t base_lo; //Address of the region
	uint32_t base_hi; //Ignored since only the low 4GiB of address space will be used by the OS

	uint32_t length_lo; //Length of the region in bytes
	uint32_t length_hi;

	uint32_t type; //Region type

	uint32_t unused_field; //The ACPI 3.0 Extended Attributes. The OS will ignore this.
} __attribute__((packed));

struct MEMORY_MAP_Descriptor {
	uint16_t n_entries; //Number of entries in the memory map
} __attribute__((packed));

extern struct MEMORY_MAP_Descriptor *MEMORY_MAP_descriptor;
extern struct MEMORY_MAP_Entry *MEMORY_MAP_entries;

//Prepares the memory map for use (Deals with overlapping regions, regions extending past 4GiB, etc..)
void MEMORY_MAP_set_up();

//Returns the type of the region location points to.
//Returns 0 if the location isn't in any entry.
unsigned MEMORY_MAP_region_type(uint32_t location);

//Returns the amount of detected free memory, in bytes.
size_t MEMORY_MAP_available_mem();

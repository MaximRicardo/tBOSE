int a;

#ifdef I_AM_MAYBE_MAKING_THIS_LATER

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "mem_map.h"

struct Region {
	uint32_t base;
	uint32_t length;
};

struct AllocEntry {
	struct Region region;
	struct AllocEntry *next_entry_ptr;
};

#define m_N_ALLOC_ENTRIES 100

//Every entry in a linked list. The list is sorted with addresses in rising order
struct AllocEntry alloc_entries[m_N_ALLOC_ENTRIES] = { 0 };
size_t n_used_entries = 0;

bool entry_disabled(const struct AllocEntry *entry_ptr)
{
	return entry_ptr->region.length == 0;
}

void *get_region_end(const struct Region *region_ptr)
{
	return (void *)(region_ptr->base + region_ptr->length - 1);
}

void *alloc_free_unused_area_in_mem_map_entry(
	uint32_t size, const struct MEMORY_MAP_Entry *mem_map_entry_ptr)
{
	//Temporary bad code. Later, should start at the entry with the lowest base value
	struct AllocEntry *cur_entry_ptr = &alloc_entries[0];

	//Loop through every entry in the alloc_entries array
	while (cur_entry_ptr->next_entry_ptr != NULL) {
		struct AllocEntry *next_entry_ptr =
			cur_entry_ptr->next_entry_ptr;

		uint32_t entries_gap_size =
			next_entry_ptr->region.base -
			(uint32_t)get_region_end(&cur_entry_ptr->region) - 1;

		//If the gap is large enough, then return the pointer to the start of that gap
		if (entries_gap_size >= size) {
			return get_region_end(&cur_entry_ptr->region) + 1;
		}

		//Move to the next entry
		cur_entry_ptr = next_entry_ptr;
	}

	return NULL;
}

//Returns a pointer to any unallocated area large enough to fit size bytes
//Ignores any areas not registered as free by the memory map
void *alloc_free_unused_area(uint32_t size)
{
	//Loop through every free memory map entry
	for (unsigned i = 0; i < MEMORY_MAP_descriptor_ptr->n_entries; i++) {
		const struct MEMORY_MAP_Entry *cur_mem_map_entry_ptr =
			&MEMORY_MAP_entries[i];
		if (cur_mem_map_entry_ptr->type != m_MEMORY_MAP_ENTRY_FREE_TYPE)
			continue;

		void *free_area = alloc_free_unused_area_in_mem_map_entry(
			size, cur_mem_map_entry_ptr);
		if (free_area)
			return free_area;
	}

	return NULL;
}

void *k_malloc(size_t size)
{
	if (size == 0)
		return NULL;
}

#endif

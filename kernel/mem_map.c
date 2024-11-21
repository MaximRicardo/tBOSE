#include <stdbool.h>
#include <stdint.h>

#include "mem_map.h"

struct MEMORY_MAP_Descriptor *MEMORY_MAP_descriptor_ptr = (struct MEMORY_MAP_Descriptor*)m_MEMORY_MAP_BASE_ADDRESS;
struct MEMORY_MAP_Entry *MEMORY_MAP_entries = (struct MEMORY_MAP_Entry*)(m_MEMORY_MAP_BASE_ADDRESS+sizeof(struct MEMORY_MAP_Descriptor));

//Zeroes-out the entry
static void clear_entry(struct MEMORY_MAP_Entry *entry_ptr) {
    
    entry_ptr->base_lo = 0;
    entry_ptr->base_hi = 0;
    entry_ptr->length_hi = 0;
    entry_ptr->length_lo = 0;
    entry_ptr->type = 0;
    entry_ptr->unused_field = 0;

}

static void limit_entries_to_4_GiB() {

    for (unsigned i = 0; i < MEMORY_MAP_descriptor_ptr->n_entries; i++) {
        struct MEMORY_MAP_Entry *current_entry_ptr = &MEMORY_MAP_entries[i];
        
        //Clear the entry if it starts past 4GiB into the address space
        if (current_entry_ptr->base_hi != 0) {
            clear_entry(current_entry_ptr);
            continue;
        }

                                                                                //This checks if the result wrapped around. If so, then the value went past 4 GiB
        bool entry_extends_past_4_GiB = current_entry_ptr->length_hi != 0 || (current_entry_ptr->base_lo+current_entry_ptr->length_lo < current_entry_ptr->base_lo);

        if (!entry_extends_past_4_GiB) continue;

        //Make the current entry end exactly at 0xffffffff. Which is the end of the 4 GiB address space
        current_entry_ptr->length_lo = UINT32_MAX - current_entry_ptr->base_lo + 1;
    }

}

void MEMORY_MAP_set_up() {

    //Deal with regions extending past 4GiB first.
    limit_entries_to_4_GiB();

}

unsigned MEMORY_MAP_region_type(uint32_t location) {

    for (unsigned i = 0; i < MEMORY_MAP_descriptor_ptr->n_entries; i++) {
        struct MEMORY_MAP_Entry *current_entry_ptr = &MEMORY_MAP_entries[i];

        uint32_t current_entry_start = current_entry_ptr->base_lo;
        uint32_t current_entry_end = current_entry_ptr->base_lo + current_entry_ptr->length_lo - 1;
        bool location_in_cur_entry = current_entry_start <= location && location <= current_entry_end;
        if (!location_in_cur_entry) continue;

        return current_entry_ptr->type;
    }

    return 0;

}

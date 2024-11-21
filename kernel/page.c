#include <stdint.h>
#include <stdbool.h>

#include "page.h"
#include "mem_map.h"

static void update_pages(void) {
    __asm__ volatile(
            "mov eax, cr3\n"
            "mov cr3, eax\n");
}

static void invalidate_tlb_entry(void *virt_address) {

    __asm__ volatile(
            "invlpg [%[virt_address]]"
            :
            : [virt_address] "m" (virt_address)
            );

}

void PAGE_create(const void *page_start, const void *page_virtual_address, uint32_t *page_table, uint32_t *page_dir, uint32_t page_flags, uint32_t page_dir_flags) {

    //Index into the page directory, that holds the relevant page table
    size_t page_dir_idx = (uint32_t)page_virtual_address / m_PAGE_TABLE_SIZE;

    //Pointer to the beginning of the first page in the page table
    void *page_table_base_ptr = (void*)(page_dir_idx*m_PAGE_TABLE_SIZE);

    //Can probably improve this later
    for (unsigned i = 0; i < m_N_PAGES_IN_A_TABLE; i++) {
        void *cur_page_base_ptr = (void*)((uint32_t)page_table_base_ptr + m_PAGE_SIZE*i);

        bool is_page_to_allocate = cur_page_base_ptr == page_start;
        if (!is_page_to_allocate) continue;

        page_table[i] = (uint32_t)page_start | page_flags;
    }

    page_dir[page_dir_idx] = (uint32_t)page_table | page_dir_flags;

    update_pages();

}

void PAGE_create_table(const void *table_start, const void *table_virtual_address, uint32_t *page_table, uint32_t *page_dir, uint32_t page_flags, uint32_t page_dir_flags) {

    size_t page_dir_idx = (uint32_t)table_virtual_address / m_PAGE_TABLE_SIZE;

    for (unsigned i = 0; i < m_N_PAGES_IN_A_TABLE; i++) {
        void *cur_page_base_ptr = (void*)((uint32_t)table_start + m_PAGE_SIZE*i);
        page_table[i] = (uint32_t)cur_page_base_ptr | page_flags;
    }

    page_dir[page_dir_idx] = (uint32_t)page_table | page_dir_flags;
    
    update_pages();

}

void* PAGE_alloc(size_t n) {

    size_t total_bytes = n*m_PAGE_SIZE;

    void *page_start_ptr = NULL;

    for (unsigned i = 0; i < MEMORY_MAP_descriptor_ptr->n_entries; i++) {
        struct MEMORY_MAP_Entry *cur_entry_ptr = &MEMORY_MAP_entries[i];
        
        //The current entry needs to be free for use
        if (cur_entry_ptr->type != m_MEMORY_MAP_ENTRY_FREE_TYPE) continue;

        //Holds the base of the first page boundary inside the entry
        uint32_t page_aligned_base = cur_entry_ptr->base_lo / m_PAGE_SIZE * m_PAGE_SIZE;
        if (cur_entry_ptr->base_lo > page_aligned_base) page_aligned_base += m_PAGE_SIZE;

        uint32_t page_aligned_length = cur_entry_ptr->base_lo+cur_entry_ptr->length_lo - page_aligned_base;

        //Does the current entry have enough space for total_bytes?
        if (total_bytes > page_aligned_length) continue;

        //For now, just use the first entry with enough space
        page_start_ptr = (void*)page_aligned_base;
    }

    //Couldn't find enough space to allocate that many pages
    if (!page_start_ptr) return NULL;

    //TODO: Implement the rest of the function
    return NULL;

}

#include "phys_alloc.h"
#include "mem_map.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//NOTE: DON'T PRINT ANYTHING HERE SINCE THESE FUNCTIONS MAY BE CALLED BEFORE THE FRAMEBUFFER HAS BEEN MAPPED IN VIRTUAL MEMORY

//3221225472B = 3GiB
#define m_MEM_BITMAP_SIZE (3221225472/4096/8)

//Bitmap over the first 3GB of main memory
//Each bit in the bitmap corresponds to a 4KiB page in physical memory. If the bit is clear, then that specific page is free, else it is occupied.
//The 0th bit is the 0th page, 1st bit is the 1st page, 2nd bit is the 2nd page, etc.
uint8_t *mem_bitmap = (uint8_t*)0x7e00;

void PHYS_ALLOC_init_bitmap(void) {

    memset(mem_bitmap, 0, m_MEM_BITMAP_SIZE);

}

void* PHYS_ALLOC_malloc_page(void) {

    //Looping through every page.
    //The first MiB of memory is skipped, because it is used for various things that should not be overwritten.
    for (size_t i = 1048576/4096; i < m_MEM_BITMAP_SIZE*8; i++) {
        size_t mem_bitmap_idx = i/8;
        unsigned bit_idx = i%8;
        //Ignore any occupied entries
        if ((mem_bitmap[mem_bitmap_idx] >> bit_idx)&0x1)
            continue;

        uint32_t page_base = i*4096;
        //TODO: Optimize this
        if (MEMORY_MAP_region_type(page_base) != m_MEMORY_MAP_ENTRY_FREE_TYPE)
            continue;

        //Mark the page as now being allocated
        mem_bitmap[mem_bitmap_idx] |= 1<<bit_idx;

        return (void*)page_base;
    }

    return NULL;

}

void PHYS_ALLOC_free_page(void *ptr) {

    uint32_t aligned_ptr = (uint32_t)ptr & -4096;

    size_t mem_bitmap_idx = (aligned_ptr/4096)/8;
    unsigned bit_idx = (aligned_ptr/4096)%8;

    mem_bitmap[mem_bitmap_idx] &= ~((uint8_t)1 << bit_idx);

}

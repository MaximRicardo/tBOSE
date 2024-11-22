#include "virt_alloc.h"
#include "phys_alloc.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*
//4294967296B = 4GiB
#define m_N_PAGES (4294967296/4096)
*/

void *VIRT_ALLOC_malloc_page(size_t n, uint32_t flags, bool use_kernel_space) {

    if (n == 0)
        return NULL;

    uint32_t *recursive_paging = (uint32_t*)0xffc00000;

    size_t start_idx = use_kernel_space ? 0xc0000000/4096 : 0;
    size_t end_idx = use_kernel_space ? 0xffffffff/4096 : 0xbfffffff/4096;

    //Looping through every page.
    //The first 8 MiB of virtual memory is skipped, because it is already mapped.
    for (size_t i = start_idx; i <= end_idx; i++) {
        bool can_alloc = true;
        for (size_t look_ahead = 0; look_ahead < n; look_ahead++) {
            uint32_t *page_entry = &recursive_paging[i+look_ahead];

            //Ignore any present pages. A page being present means it is already occupied.
            if (*page_entry&0x1) {
                can_alloc = false;
                break;
            }
        }

        if (!can_alloc)
            continue;

        for (size_t look_ahead = 0; look_ahead < n; look_ahead++) {
            //Use recursive paging to change the address the page maps to
            recursive_paging[i+look_ahead] = (uint32_t)PHYS_ALLOC_malloc_page() | flags;
        }

        uint32_t page_base = i*4096;
        *(uint32_t*)page_base = n;  //Store the number of pages as metadata
        return (void*)(page_base+4);
    }

    return NULL;

}

void VIRT_ALLOC_free_page(void *ptr) {

    uint32_t *recursive_paging = (uint32_t*)0xffc00000;

    size_t page_idx = (uint32_t)ptr / 4096;
    size_t n_pages_to_free = *((uint32_t*)ptr-1);

    for (size_t n = 0; n < n_pages_to_free; n++) {
        recursive_paging[page_idx+n] &= ~1;
        PHYS_ALLOC_free_page((void*)(recursive_paging[page_idx+n] & -4096));
    }

}

#pragma once

#include <stddef.h>

void PHYS_ALLOC_init_bitmap(void);

//Allocates a page (4 KiB) in physical memory.
void* PHYS_ALLOC_malloc_page(void);
//Frees a page (4 KiB). The pointer is automatically aligned to 4 KiB.
void PHYS_ALLOC_free_page(void *ptr);

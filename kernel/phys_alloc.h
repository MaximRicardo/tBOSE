#pragma once

#include <stddef.h>

void PhysAlloc_init_bitmap(void);

//Allocates a page (4 KiB) in physical memory.
void *PhysAlloc_malloc_page(void);
//Frees a page (4 KiB). The pointer is automatically aligned to 4 KiB.
void PhysAlloc_free_page(void *ptr);

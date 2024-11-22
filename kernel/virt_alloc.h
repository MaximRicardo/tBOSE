#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
 * n                  - Number of pages to allocate.
 * flags              - Specifies the allocated page's flags.
 * use_kernel_space   - If true, it only allocates in the kernel space of virtual memory (0xc0000000-0xffffffff). Else, only the user space region is used (0x0-0xbfffffff).
 * Returns the virtual address of start of the first page + 4.
 * The first 4 bytes of the first page contain the number of consecutively allocated pages
 */
void *VIRT_ALLOC_malloc_page(size_t n, uint32_t flags, bool use_kernel_space);

//Disables the page entry's present bit
void VIRT_ALLOC_free_page(void *ptr);

#include "phys_alloc.h"
#include "mem_map.h"
#include "page.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

//NOTE:
//  DON'T PRINT ANYTHING HERE SINCE THESE FUNCTIONS MAY BE CALLED BEFORE THE
//  FRAMEBUFFER HAS BEEN MAPPED IN VIRTUAL MEMORY

//4294967296B = 4GiB
#define m_MEM_BITMAP_SIZE (4294967296 / m_PAGE_SIZE / 8)

//bitmap over the entire 4GiB of main memory
//each bit in the bitmap corresponds to a 4KiB page in physical memory.
//if the bit is clear, then that specific page is free, else it is occupied.
//the 0th bit is the 0th page, 1st bit is the 1st page, 2nd bit is the 2nd
//page, etc.
static uint8_t *mem_bitmap = (uint8_t *)0x6000;

void PhysAlloc_init_bitmap(void)
{
	memset(mem_bitmap, 0, m_MEM_BITMAP_SIZE);
}

static bool is_page_occupied(size_t page_idx)
{
	size_t mem_bitmap_idx = page_idx / 8;
	unsigned bit_idx = page_idx % 8;

	return (mem_bitmap[mem_bitmap_idx] >> bit_idx) & 0x1;
}

static void mark_page_occupied(size_t page_idx)
{
	size_t mem_bitmap_idx = page_idx / 8;
	unsigned bit_idx = page_idx % 8;

	mem_bitmap[mem_bitmap_idx] |= 1 << bit_idx;
}

static void mark_page_free(size_t page_idx)
{
	size_t mem_bitmap_idx = page_idx / 8;
	unsigned bit_idx = page_idx % 8;

	mem_bitmap[mem_bitmap_idx] &= ~((uint8_t)1 << bit_idx);
}

void *PhysAlloc_malloc_page(void)
{
	//low mem is skipped, because it is used for various things that should
	//not be overwritten.
	for (size_t i = 1048576 / m_PAGE_SIZE; i < m_MEM_BITMAP_SIZE * 8; i++) {
		if (is_page_occupied(i))
			continue;

		uint32_t page_base = i * 4096;

		//some might call this slow. i say stfu.
		if (MemoryMap_region_type(page_base) !=
		    m_MEMORY_MAP_ENTRY_FREE_TYPE)
			continue;

		mark_page_occupied(i);

		return (void *)page_base;
	}

	return NULL;
}

void PhysAlloc_free_page(void *ptr)
{
	mark_page_free((uint32_t)ptr / m_PAGE_SIZE);
}

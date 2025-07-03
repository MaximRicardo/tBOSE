#include "virt_alloc.h"
#include "phys_alloc.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

static void invalidate_tlb_entry(void *virt_address)
{
	__asm__ volatile("invlpg %[virt_address]"
			 :
			 : [virt_address] "m"(virt_address));
}

static bool is_page_occupied(uint32_t idx)
{
	uint32_t *recursive_paging = (uint32_t *)0xffc00000;
	uint32_t *page_entry = &recursive_paging[idx];

	return *page_entry & 0x1;
}

static bool can_alloc_at_page(uint32_t page_idx, uint32_t pages_to_alloc)
{
	for (size_t look_ahead = 0; look_ahead < pages_to_alloc; look_ahead++) {
		if (is_page_occupied(page_idx + look_ahead))
			return false;
	}

	return true;
}

/* returns false if the allocation failed.
 * should probably split this into multiple functions at some point.
 * that point is never coming tho. */
static bool allocate_pages(uint32_t start_idx, uint32_t n, uint32_t flags)
{
	uint32_t *recursive_paging = (uint32_t *)0xffc00000;

	for (size_t look_ahead = 0; look_ahead < n; look_ahead++) {
		//Use recursive paging to change the address the page maps to
		uint32_t phys_location = (uint32_t)PHYS_ALLOC_malloc_page();
		if (phys_location != 0) {
			recursive_paging[start_idx + look_ahead] =
				phys_location | flags;

			invalidate_tlb_entry(
				(void *)((start_idx + look_ahead) * 4096));
		} else if (look_ahead > 0) {
			//if the allocation failed go back and deallocate all
			//the previously allocated pages

			for (size_t go_back = look_ahead - 1;
			     go_back < look_ahead; go_back--) {
				recursive_paging[start_idx + go_back] &= ~1;
				invalidate_tlb_entry(
					(void *)((start_idx + go_back) * 4096));
			}

			return false;
		} else
			return false;
	}

	return true;
}

void *VIRT_ALLOC_malloc_page(size_t n, uint32_t flags, bool use_kernel_space)
{
	if (n == 0)
		return NULL;

	size_t start_idx = use_kernel_space ? 0xc0000000 / 4096 : 0;

	//0xffbfffff is to not end up using the recursive paging region thingy
	size_t end_idx = use_kernel_space ? 0xffbfffff / 4096 :
					    0xbfffffff / 4096;

	//the first 8 MiB of virtual memory is skipped because it's already
	//mapped.
	for (size_t i = start_idx; i <= end_idx; i++) {
		if (!can_alloc_at_page(i, n))
			continue;

		if (!allocate_pages(i, n, flags))
			return NULL;

		uint32_t page_base = i * 4096;
		//The number of pages gets stored as metadata right below the
		//returned ptr
		*(uint32_t *)page_base = n;
		return (void *)(page_base + 4);
	}

	return NULL;
}

void VIRT_ALLOC_free_page(void *ptr)
{
	uint32_t *recursive_paging = (uint32_t *)0xffc00000;

	size_t page_idx = (uint32_t)ptr / 4096;
	size_t n_pages_to_free = *((uint32_t *)ptr - 1);

	for (size_t n = 0; n < n_pages_to_free; n++) {
		recursive_paging[page_idx + n] &= ~1;
		PHYS_ALLOC_free_page(
			(void *)(recursive_paging[page_idx + n] & -4096));
		//Update the cache
		invalidate_tlb_entry((void *)((page_idx + n) * 4096));
	}
}

void *k_malloc(size_t n, uint32_t flags, bool use_kernel_space)
{
	size_t n_pages = (n + 4) % 4096 != 0 ? (n + 4) / 4096 + 1 :
					       (n + 4) / 4096;
	return VIRT_ALLOC_malloc_page(n_pages, flags, use_kernel_space);
}

void k_free(void *ptr)
{
	if (ptr == NULL)
		return;
	VIRT_ALLOC_free_page(ptr);
}

void *k_calloc(size_t n_memb, size_t memb_size, uint32_t flags,
	       bool use_kernel_space)
{
	size_t n = n_memb * memb_size;
	void *ptr = k_malloc(n, flags, use_kernel_space);
	memset(ptr, 0, n);
	return ptr;
}

void *k_realloc(void *ptr, size_t n, uint32_t flags, bool use_kernel_space)
{
	if (ptr == NULL) {
		return k_malloc(n, flags, use_kernel_space);
	}

	void *new_ptr = k_malloc(n, flags, use_kernel_space);
	if (new_ptr == NULL)
		return NULL;

	//Rounded up to the nearest whole page, then minus 4 to not count the metadata.
	size_t old_arr_size = *((uint32_t *)ptr - 1) * 4096 - 4;

	memcpy(new_ptr, ptr, old_arr_size < n ? old_arr_size : n);

	return new_ptr;
}

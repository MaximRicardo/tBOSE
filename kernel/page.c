#include <stdint.h>
#include <stdbool.h>

#include "page.h"

static void update_pages(void)
{
	__asm__ volatile("mov %cr3, %eax\n"
			 "mov %eax, %cr3\n");
}

/*
static void invalidate_tlb_entry(void *virt_address) {

    __asm__ volatile(
            "invlpg [%[virt_address]]"
            :
            : [virt_address] "m" (virt_address)
            );

}*/

void PAGE_create_table(const void *table_start,
		       const void *table_virtual_address, uint32_t *page_table,
		       uint32_t *page_table_phys_address, uint32_t *page_dir,
		       uint32_t page_flags, uint32_t page_dir_flags)
{
	size_t page_dir_idx =
		(uint32_t)table_virtual_address / m_PAGE_TABLE_SIZE;

	for (unsigned i = 0; i < m_N_PAGES_IN_A_TABLE; i++) {
		void *cur_page_base_ptr =
			(void *)((uint32_t)table_start + m_PAGE_SIZE * i);
		page_table[i] = (uint32_t)cur_page_base_ptr | page_flags;
	}

	page_dir[page_dir_idx] = (uint32_t)page_table_phys_address |
				 page_dir_flags;

	update_pages();
}

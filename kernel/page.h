#pragma once

#include <stddef.h>
#include <stdint.h>

//Size of a single page in bytes.
#define m_PAGE_SIZE 4096

#define m_N_PAGES_IN_A_TABLE 1024

//Size of a table of 1024 pages
#define m_PAGE_TABLE_SIZE (m_PAGE_SIZE*m_N_PAGES_IN_A_TABLE)

/*
 * The page directory must be initialized. This function only initializes the relevant page directory entry to point to the page table.
 * Same goes for the page table. This function only initializes the relevant entry in the page table.
 *
 * page_start: Where the page maps to in physical memory. MUST be aligned to m_PAGE_SIZE.
 * page_virtual_address: Where the page maps to in virtual memory. MUST be aligned to m_PAGE_SIZE.
 * page_table: Pointer to the start of the relevant page table.
 * page_dir: Pointer to the page directory.
 * page_flags: Flags the page should have.
 * page_dir_flags: Flags the relevant page directory entry should have.
 */
void PAGE_create(const void *page_start, const void *page_virtual_address, uint32_t *page_table, uint32_t *page_dir, uint32_t page_flags, uint32_t page_dir_flags);

/*
 * Creates a table of 1024 pages, totaling in 4 MiB covered.
 * table_start: Where the page table maps to in physical memory. MUST be aligned to m_PAGE_TABLE_SIZE.
 * table_virtual_address: Where the page table maps to in virtual memory. MUST be aligned to m_PAGE_TABLE_SIZE.
 * page_table: Pointer to the start of the page table to be initialized.
 * page_dir: Pointer to the start of the page directory.
 * page_flags: Flags each page should have.
 * page_dir_flags: Flags the relevant page directory entry should have.
 */
void PAGE_create_table(const void *table_start, const void *table_virtual_address, uint32_t *page_table, uint32_t *page_dir, uint32_t page_flags, uint32_t page_dir_flags);

//Allocates n consecutive pages in virtual memory. Returns a pointer to be beginning of the first page
void* PAGE_alloc(size_t n);

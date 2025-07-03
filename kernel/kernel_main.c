#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <cpuid.h>
#include <string.h>
#include <time.h>
#include "color.h"
#include "vbe.h"
#include "pixel.h"
#include "print.h"
#include "gdt.h"
#include "idt.h"
#include "interrupts.h"
#include "mem_map.h"
#include "page.h"
#include "page_flags.h"
#include "phys_alloc.h"
#include "virt_alloc.h"
#include "kernel_options.h"
#include "tss.h"
#include "pic.h"
#include "pit.h"
#include "io.h"
#include "drivers/ata.h"

static struct GDT gdt;
static struct IDT idt;
static struct TSS tss;

uint32_t *page_directory = (uint32_t *)0x5d000;

//Look in low_mem_map.txt for context
uint32_t *page_tables_table = (uint32_t *)0x5f000;

uint32_t boot_disk;

__attribute__((noreturn)) static void halt_forever(void)
{
	while (true) {
		__asm__ volatile("hlt");
	}
}

static bool msr_supported(void)
{
	uint32_t unused, edx, ecx;
	__get_cpuid(1, &unused, &unused, &ecx, &edx);

	return (edx >> 5) & 1;
}

static void get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
	__asm__ volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

static void set_msr(uint32_t msr, uint32_t lo, uint32_t hi)
{
	__asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

__attribute__((noreturn)) static void enter_ring_3(void (*func)(void))
{
	void *user_sp = k_malloc(m_KERNEL_STACK_SIZE, 0x7, false);
	k_printf("user_sp = %p, func = %p\n", user_sp, (void *)func);

	__asm__ volatile(
		/*
            "mov $((4*8) | 3), %%ax\n"
            "mov %%ax, %%ds\n"
            "mov %%ax, %%es\n"
            "mov %%ax, %%ss\n"
            "mov %%ax, %%fs\n"
            "mov %%ax, %%gs\n"*/
		"\n"
		//Store the variables in registers before they become invalid
		"mov %0, %%ebx\n"
		"mov %1, %%ecx\n"
		"\n"
		"mov %%ecx, %%esp\n"
		"mov %%esp, %%eax\n"
		"pushl $((4*8) | 3)\n" //Data selector
		"push %%eax\n"
		"pushf\n"
		"pushl $((3*8) | 3)\n" //Code selector
		"push %%ebx\n"
		"iret\n"
		:
		: "r"(func), "r"(user_sp));

	//Shuts the compiler up
	halt_forever();
}

//we don't wanna have to rely on the gdt provided by the bootloader since it'll
//get overwritten later
static void create_gdt(void)
{
	gdt.entries[0] = GDT_create_zero_entry(); //The NULL descriptor
	gdt.entries[1] = GDT_create_entry(0x00000000, 0xfffff,
					  (m_GDT_CODE_KERNEL)); //Kernel code
	gdt.entries[2] = GDT_create_entry(0x00000000, 0xfffff,
					  (m_GDT_DATA_KERNEL)); //Kernel data
	gdt.entries[3] = GDT_create_entry(0x00000000, 0xfffff,
					  (m_GDT_CODE_USER)); //User code
	gdt.entries[4] = GDT_create_entry(0x00000000, 0xfffff,
					  (m_GDT_DATA_USER)); //User data
	gdt.entries[5] = GDT_create_entry((uint32_t)&tss,
					  (uint32_t)&tss + sizeof(struct TSS),
					  0xe9); //TSS

	gdt.descriptor.size = m_N_GDT_ENTRIES * sizeof(struct GDT_Entry);
	gdt.descriptor.base = (uint32_t)gdt.entries;

	__asm__("lgdt %0\n" ::"m"(gdt.descriptor));
}

//the idt created by the boot loader was useless
static void create_idt(void)
{
	for (uint32_t i = 0; i < 256; i++) {
		idt.entries[i] =
			IDT_create_entry((uint32_t)INTERRUPT_jump_table[i]);
	}

	idt.descriptor.size = m_IDT_SIZE;
	idt.descriptor.base = (uint32_t)(&idt.entries[0]);

	//interrupts cant be enabled yet else the PIT spams the kernel before everything
	//is set up.
	__asm__ volatile("lidt %0\n" : : "m"(idt.descriptor));
}

static void alloc_page_tables_table(void)
{
	for (unsigned i = 0; i < m_N_PAGES_IN_A_TABLE; i++) {
		void *cur_page_base_ptr = PHYS_ALLOC_malloc_page();
		page_tables_table[i] = (uint32_t)cur_page_base_ptr |
				       m_PAGEFLAG_rw;
	}

	uint32_t idx = m_PAGE_TABLES_TABLE_VIRTUAL_ADDRESS / m_PAGE_TABLE_SIZE;
	page_directory[idx] = (uint32_t)page_tables_table | m_PAGEFLAG_rw;

	//refreshes the tlb
	__asm__ volatile("mov %cr3, %eax\n"
			 "mov %eax, %cr3\n");
}

static void add_write_combine_cache()
{
	if (msr_supported()) {
		uint32_t lo, hi;
		get_msr(0x277, &lo, &hi);
		hi &= ~0x7;
		hi |= 0x1; //Write combining
		set_msr(0x277, lo, hi);
	}
}

/* identity maps all pages except for the frame buffer page */
static void alloc_normal_p_tables(void)
{
	for (unsigned i = 2; i < 1023; i++) {
		//skip the table for 0xc0000000-0xc0400000, as that is where
		//the kernel lives and will be allocated seperately.
		if (i == 768)
			continue;

		uint32_t *page_table_phys =
			(uint32_t *)(page_tables_table[i] & -4096);
		uint32_t *page_table_virt =
			(uint32_t *)(m_PAGE_TABLES_TABLE_VIRTUAL_ADDRESS +
				     4096 * i);

		uint32_t page_table_base = i * m_PAGE_TABLE_SIZE;

		//0xc0000000 and above is accessible only to the kernel (with
		//the exception of the frame buffer)

		//uint32_t page_dir_flags = i*1048576*4 < 0xc0000000 ? 0x7 : 0x3;
		//honestly idk why i'm using this version. past me made this
		//decision and i trust him for now.
		uint32_t page_dir_flags = page_table_base < 0xc0000000 ? 0x7 :
									 0x7;

		PAGE_create_table((void *)page_table_base,
				  (void *)page_table_base, page_table_virt,
				  page_table_phys, page_directory, 0x0,
				  page_dir_flags);
	}
}

static void ident_map_low_mem(void)
{
	uint32_t *page_table_phys = (uint32_t *)(page_tables_table[0] & -4096);
	uint32_t *page_table_virt =
		(uint32_t *)m_PAGE_TABLES_TABLE_VIRTUAL_ADDRESS;
	PAGE_create_table((void *)0, (void *)0x0, page_table_virt,
			  page_table_phys, page_directory, 0x3, 0x3);
}

//The 768th page, which starts at 0xc0000000, needs to be handled
//carefully. This is because the kernel is being executed from that
//address, so it always needs to be mapped correctly.
static void alloc_kernel_page(void)
{
	uint32_t *page_table_phys =
		(uint32_t *)(page_tables_table[768] & -4096);
	uint32_t *page_table_virt =
		(uint32_t *)(m_PAGE_TABLES_TABLE_VIRTUAL_ADDRESS + 4096 * 768);
	//PAGE_create_table((void*)0, (void*)0xc0000000, page_table_virt, page_table_phys, page_directory, 0x3, 0x3);
	//don't know why i'm using this version, but past me made this decision
	//and i think he's trustworthy.
	PAGE_create_table((void *)0, (void *)0xc0000000, page_table_virt,
			  page_table_phys, page_directory, 0x7, 0x7);
}

static void map_framebuffer_page(void)
{
	uint32_t *framebuffer_page_table_phys =
		(uint32_t *)(page_tables_table[960] & -4096);

	uint32_t *framebuffer_page_table_virt =
		(uint32_t *)(m_PAGE_TABLES_TABLE_VIRTUAL_ADDRESS + 4096 * 960);

	uint32_t page_table_entry_flags =
		msr_supported() ? 0x87 : 0x17; //0b10000011, and 0b10011

	uint32_t floored_base = VBE_mode_info.framebuffer / m_PAGE_TABLE_SIZE *
				m_PAGE_TABLE_SIZE;

	PAGE_create_table((void *)floored_base,
			  (void *)m_FRAMEBUFFER_VIRTUAL_ADDRESS,
			  framebuffer_page_table_virt,
			  framebuffer_page_table_phys, page_directory,
			  page_table_entry_flags, 0x7);
}

static void setup_pages(void)
{
	alloc_page_tables_table();

	alloc_normal_p_tables();
	ident_map_low_mem();
	alloc_kernel_page();
	map_framebuffer_page();
}

/* clears to black */
static void clear_scr(void)
{
	for (unsigned y = 0; y < VBE_mode_info.height; y++) {
		for (unsigned x = 0; x < VBE_mode_info.width; x++) {
			struct COLOR_rgb pixel_color = { .r = 0.f,
							 .g = 0.f,
							 .b = 0.f };
			PIXEL_plot_norm_rgb(x, y, pixel_color);
		}
	}
}

/* moves the memory map to a dynamically allocated pointer to prevent depending
 * on the bootloader-allocated one. */
static void move_mem_map(void)
{
	void *new_memory_map_ptr =
		k_malloc(MEMORY_MAP_descriptor->n_entries *
					 sizeof(struct MEMORY_MAP_Entry) +
				 sizeof(struct MEMORY_MAP_Descriptor),
			 0x3, true);
	if (new_memory_map_ptr == NULL) {
		k_printf("ERROR: Couldn't allocate memory map!\n");
		halt_forever();
	}

	//Copying over the descriptor
	*(struct MEMORY_MAP_Descriptor *)new_memory_map_ptr =
		*MEMORY_MAP_descriptor;

	//Copying over the entries
	struct MEMORY_MAP_Entry *new_entries =
		(struct MEMORY_MAP_Entry
			 *)((uint8_t *)new_memory_map_ptr +
			    sizeof(struct MEMORY_MAP_Descriptor));

	memcpy(new_entries, MEMORY_MAP_entries,
	       MEMORY_MAP_descriptor->n_entries *
		       sizeof(struct MEMORY_MAP_Entry));

	MEMORY_MAP_descriptor = new_memory_map_ptr;
	MEMORY_MAP_entries = new_entries;
}

static void alloc_back_buffer(void)
{
	PIXEL_back_buffer = k_calloc(VBE_mode_info.width * VBE_mode_info.height,
				     sizeof(*PIXEL_back_buffer), 0x7, false);
	if (PIXEL_back_buffer == NULL) {
		k_printf("ERROR: Couldn't allocate the back buffer!\n");
		halt_forever();
	}
}

static void *alloc_kernel_stack(void)
{
	void *ptr = k_malloc(m_KERNEL_STACK_SIZE, m_PAGEFLAG_rw, true);
	if (!ptr) {
		k_printf("ERROR: Couldn't allocate the kernel stack!\n");
		halt_forever();
	}

	return ptr;
}

static void init_tss(void *kernel_stack_ptr)
{
	memset(&tss, 0, sizeof(tss));
	tss.ss0 = 0x10;
	tss.esp0 = (uint32_t)kernel_stack_ptr;
	tss.ds = 0x10;
	tss.ss = 0x10;
	tss.es = 0x10;
	tss.fs = 0x10;
	tss.gs = 0x10;
	tss.cs = 0x08;
	TSS_update();
}

static void init_pic(void)
{
	PIC_init();
	PIC_irq_clear_mask(0); //PIT interrupt
	PIC_irq_clear_mask(6); //Floppy disk interrupt
}

/* makes the PIT send an interrupt m_PIT_IRQ_PER_SECOND times per second */
static void init_pit(void)
{
	IO_out_port_b(0x43, 0x34);
	IO_out_port_b(0x40, m_PIT_RESET_TIME & 0xff);
	IO_out_port_b(0x40, (m_PIT_RESET_TIME >> 8) & 0xff);
}

/* essentially just sets everything up and removes any dependencies on
 * the bootloader so it can safely be overwritten later if needed. */
__attribute__((noreturn)) void
k_main(const struct VBE_Info *old_vbe_info,
       const struct VBE_ModeInfo *old_vbe_mode_info, uint32_t boot_disk_arg)
{
	boot_disk = boot_disk_arg;

	/* pls don't change the order of any of this unless u know better */

	VBE_setup_infos(old_vbe_info, old_vbe_mode_info);

	create_gdt();
	create_idt();

	add_write_combine_cache();

	MEMORY_MAP_set_up();
	PHYS_ALLOC_init_bitmap();
	setup_pages();

	move_mem_map();

	clear_scr();

	PRINT_reset_cursor_pos();

	alloc_back_buffer();

	void *k_stack = alloc_kernel_stack();

	init_tss(k_stack);

	init_pic();
	init_pit();

	/* everything has been setup so the PIT won't single-handedly nuke the
         * kernel */
	__asm__ volatile("sti\n");

	__asm__ volatile("movl %0, %%esp\n"
			 "jmp k_main_setup_done"
			 :
			 : "rm"(k_stack));

	//this is never run, but stops the compiler from complaining
	halt_forever();
}

static void print_mem_map(void)
{
	for (uint32_t i = 0; i < MEMORY_MAP_descriptor->n_entries; i++) {
		const struct MEMORY_MAP_Entry *entry = &MEMORY_MAP_entries[i];

		k_printf("ENTRY #%u\n", i);
		k_printf("base lo = 0x%lx ", (unsigned long)entry->base_lo);
		k_printf("base hi = 0x%lx ", (unsigned long)entry->base_hi);
		k_printf("size lo = 0x%lx ", (unsigned long)entry->length_lo);
		k_printf("size hi = 0x%lx ", (unsigned long)entry->length_hi);
		k_printf("type = %ld ", (unsigned long)entry->type);
		k_printf("unused = 0x%lx ", (unsigned long)entry->unused_field);
		k_printf("size (KiB) = %lu\n\n",
			 (unsigned long)entry->length_lo / 1024);
	}
}

__attribute__((noreturn)) void k_main_setup_done(void)
{
	//NOTE:
	//    THE KERNEL SETUP PROCESS IS NOW DONE! THE MAP OF LOW MEMORY NOW
	//    LOOKS LIKE THE SECOND ONE IN "low_mem_map.txt"!

	uint32_t sp_value;
	__asm__ volatile("mov %%esp, %0\n" : "=r"(sp_value));

	char *str = k_calloc(10, sizeof(*str), 0x3, true);
	k_printf("str = \"%s\", str = %p\n", str, (void *)str);
	strcpy(str, "hello!");
	k_printf("str = \"%s\", str = %p\n", str, (void *)str);
	str = k_realloc(str, 8, 0x3, true);
	k_printf("str = \"%s\", str = %p\n", str, (void *)str);
	k_free(str);

	k_printf("OS boot disk = %u\n", boot_disk);
	k_printf("msr supported: %s\n", msr_supported() ? "true" : "false");
	k_printf("stack pointer = %p\n", (void *)sp_value);

	k_printf("\nn memory map entries = %u. mem map descriptor ptr = %p\n\n",
		 MEMORY_MAP_descriptor->n_entries,
		 (void *)MEMORY_MAP_descriptor);
	print_mem_map();

	k_printf("%uMiB of free memory detected.\n",
		 MEMORY_MAP_available_mem() / 1024 / 1024);

	ATA_soft_reset(&ATA_device);

	void *ptr = NULL;
	ATA_read_sector(&ATA_device, ptr);

	enter_ring_3(halt_forever);

	halt_forever();
}

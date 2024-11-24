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
#include "phys_alloc.h"
#include "virt_alloc.h"
#include "kernel_options.h"
#include "tss.h"

struct GDT gdt;
struct IDT idt;
struct TSS tss;

uint32_t *page_directory = (uint32_t*)0x5d000;

uint32_t *page_tables_table = (uint32_t*)0x5f000; //Look in low_mem_map.txt for context

__attribute__((noreturn))
static void halt_forever(void) {

    while (true) {
        __asm__ volatile("hlt");
    }

}

static bool msr_supported(void) {

    uint32_t unused, edx, ecx;
    __get_cpuid(1, &unused, &unused, &ecx, &edx);

    return (edx >> 5) & 1;

}

static void get_msr(uint32_t msr, uint32_t *lo, uint32_t *hi) {
   __asm__ volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

static void set_msr(uint32_t msr, uint32_t lo, uint32_t hi) {
   __asm__ volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}

__attribute__((noreturn))
static void enter_ring_3(void (*func)(void)) {

    void *user_sp = k_malloc(m_KERNEL_STACK_SIZE, 0x7, false);
    k_printf("user_sp = %p, func = %p\n", user_sp, (void*)func);

    __asm__ volatile(
            /*
            "mov $((4*8) | 3), %%ax\n"
            "mov %%ax, %%ds\n"
            "mov %%ax, %%es\n"
            "mov %%ax, %%ss\n"
            "mov %%ax, %%fs\n"
            "mov %%ax, %%gs\n"*/
            "\n"
            "mov %0, %%ebx\n" //Store the variables in registers before they become invalid
            "mov %1, %%ecx\n"
            "\n"
            "mov %%ecx, %%esp\n"
            "mov %%esp, %%eax\n"
            "pushl $((4*8) | 3)\n"  //Data selector
            "push %%eax\n"
            "pushf\n"
            "pushl $((3*8) | 3)\n"  //Code selector
            "push %%ebx\n"
            "iret\n"
            :
            :
            "r" (func),
            "r" (user_sp)
            );

    //Shuts the compiler up
    halt_forever();

}

__attribute__((noreturn))
void k_main_setup_done();

__attribute__((noreturn))
void k_main(const struct VBE_Info *old_vbe_info_ptr, const struct VBE_ModeInfo *old_vbe_mode_info_ptr) {

    //Create a page for the frame buffer
    VBE_setup_infos(old_vbe_info_ptr, old_vbe_mode_info_ptr);
    
    //Create a new GDT, so the kernel doesn't rely on the one in the bootloader
    gdt.entries[0] = GDT_create_zero_entry(); //The NULL descriptor
    gdt.entries[1] = GDT_create_entry(0x00000000, 0xfffff, (m_GDT_CODE_KERNEL));  //Kernel code
    gdt.entries[2] = GDT_create_entry(0x00000000, 0xfffff, (m_GDT_DATA_KERNEL));  //Kernel data
    gdt.entries[3] = GDT_create_entry(0x00000000, 0xfffff, (m_GDT_CODE_USER));    //User code
    gdt.entries[4] = GDT_create_entry(0x00000000, 0xfffff, (m_GDT_DATA_USER));    //User data
    gdt.entries[5] = GDT_create_entry((uint32_t)&tss, (uint32_t)&tss+sizeof(struct TSS), 0xe9); //TSS

    gdt.descriptor.size = m_N_GDT_ENTRIES*sizeof(struct GDT_Entry);
    gdt.descriptor.base = (uint32_t)gdt.entries;

    //Set the new GDT as the current GDT
    __asm__("lgdt %0\n" :: "m"(gdt.descriptor));

    //Create a new IDT, since the one created by the boot loader was useless
    for (unsigned i = 0; i < 256; i++) {
        idt.entries[i] = IDT_create_entry((uint32_t)INTERRUPT_jump_table[i]);
    }

    //Setup the IDT descriptor
    idt.descriptor.size = 2048; //The IDT table is 2048 bytes large (8 bytes per entry * 256 entries)
    idt.descriptor.base = (uint32_t)(&idt.entries[0]);  //Starts at the first entry

    //Set the new IDT as the current IDT. Also interrupts can now be enabled
    __asm__ volatile(
            "lidt %0\n"
            //"sti\n"
            :
            : "m"(idt.descriptor)
            );
    
    //Now the IDT and GDT are stored by the kernel, instead of the boot loader. That means the bootloader can safely be overwritten later if needed

    //Setting the MSR to add write combine cache. The 4th MSR entry is modified.
    if (msr_supported()) {
        uint32_t lo, hi;
        get_msr(0x277, &lo, &hi);
        hi &= ~0x7;
        hi |= 0x1;  //Write combining
        set_msr(0x277, lo, hi);
    }

    //Set up the memory map properly so it can later be used for memory allocation
    MEMORY_MAP_set_up();
    PHYS_ALLOC_init_bitmap();
    
    //Allocating the page tables table
    for (unsigned i = 0; i < m_N_PAGES_IN_A_TABLE; i++) {
        void *cur_page_base_ptr = PHYS_ALLOC_malloc_page();
        page_tables_table[i] = (uint32_t)cur_page_base_ptr | 0x3;
    }
    page_directory[m_PAGE_TABLES_TABLE_VIRTUAL_ADDRESS/m_PAGE_TABLE_SIZE] = (uint32_t)page_tables_table | 0x3;
    __asm__ volatile(
            "mov %cr3, %eax\n"
            "mov %eax, %cr3\n");

    //Skip the last page table, since it maps to the page directory itself. And also skip the first 2 tables, since they need to be handled differently.
    for (unsigned i = 2; i < 1023; i++) {
        //Skip the table for the first 4MiB of the upper GiB.
        if (i == 768)
            continue;
        uint32_t *page_table_phys = (uint32_t*)(page_tables_table[i] & -4096);
        uint32_t *page_table_virt = (uint32_t*)(m_PAGE_TABLES_TABLE_VIRTUAL_ADDRESS+4096*i);

        //0xc0000000 and above is accessible only to the kernel (with the exception of the frame buffer, and the first 8MiB of the virtual address space)
        //uint32_t page_dir_flags = i*1048576*4 < 0xc0000000 ? 0x7 : 0x3;
        uint32_t page_dir_flags = i*1048576*4 < 0xc0000000 ? 0x7 : 0x7;
        PAGE_create_table((void*)(4096*1024*i), (void*)(4096*1024*i), page_table_virt, page_table_phys, page_directory, 0x0, page_dir_flags);
    }

    //Identity map low memory
    {
        uint32_t *page_table_phys = (uint32_t*)(page_tables_table[0] & -4096);
        uint32_t *page_table_virt = (uint32_t*)m_PAGE_TABLES_TABLE_VIRTUAL_ADDRESS;
        PAGE_create_table((void*)0, (void*)0x0, page_table_virt, page_table_phys, page_directory, 0x3, 0x3);
    }

    //The 768th page, which starts at 0xc0000000, needs to be handled carefully. This is because the kernel is being executed from that address, so it always needs to be
    //mapped correctly.
    {
        uint32_t *page_table_phys = (uint32_t*)(page_tables_table[768] & -4096);
        uint32_t *page_table_virt = (uint32_t*)(m_PAGE_TABLES_TABLE_VIRTUAL_ADDRESS+4096*768);
        //PAGE_create_table((void*)0, (void*)0xc0000000, page_table_virt, page_table_phys, page_directory, 0x3, 0x3);
        PAGE_create_table((void*)0, (void*)0xc0000000, page_table_virt, page_table_phys, page_directory, 0x7, 0x7);
    }

    //Mapping the framebuffer to 0xf0000000
    {
        uint32_t *framebuffer_page_table_phys = (uint32_t*)(page_tables_table[960] & -4096);
        uint32_t *framebuffer_page_table_virt = (uint32_t*)(m_PAGE_TABLES_TABLE_VIRTUAL_ADDRESS+4096*960);
        uint32_t page_table_entry_flags = msr_supported() ? 0x87 : 0x17;  //0b10000011, and 0b10011
        PAGE_create_table(
                (void*)(VBE_mode_info.framebuffer/m_PAGE_TABLE_SIZE*m_PAGE_TABLE_SIZE), (void*)m_FRAMEBUFFER_VIRTUAL_ADDRESS,
                framebuffer_page_table_virt, framebuffer_page_table_phys, page_directory, page_table_entry_flags, 0x7
                );
    }

    //Clear the screen to black
    for (unsigned y = 0; y < VBE_mode_info.height; y++) {
        for (unsigned x = 0; x < VBE_mode_info.width; x++) {
            struct COLOR_rgb pixel_color = {0.f, 0.f, 0.f};
            PIXEL_plot_norm_rgb(x, y, pixel_color);
        }
    }

    PRINT_reset_cursor_pos();

    //Move the memory map
    void *new_memory_map_ptr = k_malloc(MEMORY_MAP_descriptor->n_entries*sizeof(struct MEMORY_MAP_Entry)+sizeof(struct MEMORY_MAP_Descriptor), 0x3, true);
    if (new_memory_map_ptr == NULL) {
        k_printf("ERROR: Couldn't allocate memory map!\n");
        halt_forever();
    }
    {
        //Copying over the descriptor
        *(struct MEMORY_MAP_Descriptor*)new_memory_map_ptr = *MEMORY_MAP_descriptor;

        //Copying over the entries
        struct MEMORY_MAP_Entry *new_entries = (struct MEMORY_MAP_Entry*)((uint8_t*)new_memory_map_ptr + sizeof(struct MEMORY_MAP_Descriptor));
        memcpy(new_entries, MEMORY_MAP_entries, MEMORY_MAP_descriptor->n_entries*sizeof(struct MEMORY_MAP_Entry));

        //Changing the pointers
        MEMORY_MAP_descriptor = new_memory_map_ptr;
        MEMORY_MAP_entries = new_entries;
    }

    PIXEL_back_buffer = k_calloc(VBE_mode_info.width*VBE_mode_info.height, sizeof(*PIXEL_back_buffer), 0x7, false);
    if (PIXEL_back_buffer == NULL) {
        k_printf("ERROR: Couldn't allocate the back buffer!\n");
        halt_forever();
    }

    void *new_stack_ptr = k_malloc(m_KERNEL_STACK_SIZE, 0x3, true);
    if (new_stack_ptr == NULL) {
        k_printf("ERROR: Couldn't allocate the kernel stack!\n");
        halt_forever();
    }

    //Init the TSS
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = 0x10;
    tss.esp0 = (uint32_t)new_stack_ptr;
    tss.ds = 0x10;
    tss.ss = 0x10;
    tss.es = 0x10;
    tss.fs = 0x10;
    tss.gs = 0x10;
    tss.cs = 0x08;
    TSS_update();

    /*
    __asm__ volatile(
            "int $0x10\n"
            );*/

    //Jumps into k_main_setup_done
    __asm__ volatile(
            "mov %0, %%esp\n"
            "jmp k_main_setup_done"
            :
            : "rm" (new_stack_ptr)
            );

    //This is never run, but stops the compiler from complaining
    halt_forever();

}

void k_main_setup_done() {

    //NOTE: THE KERNEL SETUP PROCESS IS NOW DONE! THE MAP OF LOW MEMORY NOW LOOKS LIKE THE SECOND ONE IN "low_mem_map.txt"!

    uint32_t sp_value;
    __asm__ volatile(
            "mov %%esp, %0\n"
            : "=r"(sp_value)
            );

    char *str = k_calloc(10, sizeof(*str), 0x3, true);
    k_printf("str = \"%s\", str = %p\n", str, (void*)str);
    strcpy(str, "hello!");
    k_printf("str = \"%s\", str = %p\n", str, (void*)str);
    str = k_realloc(str, 8, 0x3, true);
    k_printf("str = \"%s\", str = %p\n", str, (void*)str);
    k_free(str);

    k_printf("value = 0x%08lx\n", (unsigned long)*((uint32_t*)0xffc00000 + 0xa));
    k_printf("msr supported: %s\n", msr_supported() ? "true" : "false");
    k_printf("stack pointer = %p\n", (void*)sp_value);

    k_printf("\nn memory map entries = %u. mem map descriptor ptr = %p\n\n", MEMORY_MAP_descriptor->n_entries, (void*)MEMORY_MAP_descriptor);

    for (unsigned i = 0; i < MEMORY_MAP_descriptor->n_entries; i++) {
        k_printf("ENTRY #%u\n", i);
        k_printf("base lo = 0x%lx ", (unsigned long)MEMORY_MAP_entries[i].base_lo);
        k_printf("base hi = 0x%lx ", (unsigned long)MEMORY_MAP_entries[i].base_hi);
        k_printf("size lo = 0x%lx ", (unsigned long)MEMORY_MAP_entries[i].length_lo);
        k_printf("size hi = 0x%lx ", (unsigned long)MEMORY_MAP_entries[i].length_hi);
        k_printf("type = %ld ", (unsigned long)MEMORY_MAP_entries[i].type);
        k_printf("unused = 0x%lx ", (unsigned long)MEMORY_MAP_entries[i].unused_field);
        k_printf("size (KiB) = %lu\n\n", (unsigned long)MEMORY_MAP_entries[i].length_lo/1024);
    }

    k_printf("%uMiB of free memory detected.\n", MEMORY_MAP_available_mem()/1024/1024);

    enter_ring_3(halt_forever);

    halt_forever();

}

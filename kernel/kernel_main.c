#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "color.h"
#include "vbe.h"
#include "pixel.h"
#include "print.h"
#include "gdt.h"
#include "idt.h"
#include "interrupts.h"
#include "mem_map.h"

struct GDT gdt;
struct IDT idt;

__attribute__((noreturn))
static void halt_forever(void) {

    while (true) {
        __asm__("hlt");
    }

}

__attribute__((noreturn))
void k_main(const struct VBE_Info *old_vbe_info_ptr, const struct VBE_ModeInfo *old_vbe_mode_info_ptr, uint32_t value) {

    VBE_setup_infos(old_vbe_info_ptr, old_vbe_mode_info_ptr);

    //Create a new GDT, so the kernel doesn't rely on the one in the bootloader
    gdt.entries[0] = GDT_create_zero_entry(); //The NULL descriptor
    gdt.entries[1] = GDT_create_entry(0x00000000, 0xfffff, (m_GDT_CODE_KERNEL));
    gdt.entries[2] = GDT_create_entry(0x00000000, 0xfffff, (m_GDT_DATA_KERNEL));

    gdt.descriptor.size = m_N_GDT_ENTRIES*sizeof(struct GDT_Entry);
    gdt.descriptor.base = (uint32_t)gdt.entries;

    //Set the new GDT as the current GDT
    __asm__("lgdt %0\n" :: "m"(gdt.descriptor));

    //Create a new IDT, since the one created by the boot loader was useless
    for (unsigned i = 0; i < 256; i++) {
        idt.entries[i] = IDT_create_entry((uint32_t)INTERRUPT_default);
    }

    //Setup the IDT descriptor
    idt.descriptor.size = 2048; //The IDT table is 2048 bytes large (8 bytes per entry * 256 entries)
    idt.descriptor.base = (uint32_t)(&idt.entries[0]);  //Starts at the first entry

    //Set the new IDT as the current IDT. Also interrupts can now be enabled
    __asm__("lidt %0\n" :: "m"(idt.descriptor));
    
    //Now the IDT and GDT are stored by the kernel, instead of the boot loader. That means the bootloader can safely be overwritten later if needed

    //Clear the screen to black
    for (unsigned y = 0; y < VBE_mode_info.height; y++) {
        for (unsigned x = 0; x < VBE_mode_info.width; x++) {
            struct COLOR_rgb pixel_color = {0.f, 0.f, 0.f};
            PIXEL_plot_norm_rgb(x, y, pixel_color);
        }
    }

    //Set up the memory map properly so it can later be used for memory allocation
    MEMORY_MAP_set_up();

    k_printf("kernel starts around %p\n", (void*)value);

    k_printf("\nn memory map entries = %u. mem map descriptor ptr = %p\n\n", MEMORY_MAP_descriptor_ptr->n_entries, (void*)MEMORY_MAP_descriptor_ptr);

    for (unsigned i = 0; i < MEMORY_MAP_descriptor_ptr->n_entries; i++) {
        k_printf("ENTRY #%u\n", i);
        k_printf("base lo = 0x%lx ", (unsigned long)MEMORY_MAP_entries[i].base_lo);
        k_printf("base hi = 0x%lx ", (unsigned long)MEMORY_MAP_entries[i].base_hi);
        k_printf("size lo = 0x%lx ", (unsigned long)MEMORY_MAP_entries[i].length_lo);
        k_printf("size hi = 0x%lx ", (unsigned long)MEMORY_MAP_entries[i].length_hi);
        k_printf("type = %ld ", (unsigned long)MEMORY_MAP_entries[i].type);
        k_printf("unused = 0x%lx ", (unsigned long)MEMORY_MAP_entries[i].unused_field);
        k_printf("size (KiB) = %lu\n\n", (unsigned long)MEMORY_MAP_entries[i].length_lo/1024);
    }

    halt_forever();

}

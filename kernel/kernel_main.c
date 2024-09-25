#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "color.h"
#include "vbe.h"
#include "pixel.h"
#include "print.h"
#include "gdt.h"
#include "idt.h"
#include "interrupts.h"

struct GDT gdt;

struct IDT idt;

__attribute__((noreturn))
static void halt_forever(void) {

    while (true) {
        __asm__("hlt");
    }

}

__attribute__((noreturn))
void k_main(__attribute__((unused)) const uint32_t old_vbe_info_ptr_u32, const uint32_t old_vbe_mode_info_ptr_u32) {

    {
        const struct VBE_Info *const old_vbe_info_ptr = (struct VBE_Info*)old_vbe_info_ptr_u32;
        const struct VBE_ModeInfo *const old_vbe_mode_info_ptr = (struct VBE_ModeInfo*)old_vbe_mode_info_ptr_u32;
        VBE_setup_infos(old_vbe_info_ptr, old_vbe_mode_info_ptr);
    }

    //Create a new GDT, so the kernel doesn't rely on the one in the bootloader
    //This means the bootloader can be safely overwritten later
    memset(&gdt.entries[0], 0, sizeof(struct GDT_Entry)); //Null descriptor
    gdt.entries[1] = GDT_create_entry(0x00000000, 0xfffff, (m_GDT_CODE_KERNEL));
    gdt.entries[2] = GDT_create_entry(0x00000000, 0xfffff, (m_GDT_DATA_KERNEL));

    gdt.descriptor.size = m_N_GDT_ENTRIES*sizeof(struct GDT_Entry);
    gdt.descriptor.start = (uint32_t)gdt.entries;

    //Set the new GDT as the current GDT
    __asm__("lgdt %0\n" :: "m"(gdt.descriptor));

    //Create a new IDT, since the one created in the bootloader was useless
    for (unsigned i = 0; i < 256; i++) {
        idt.entries[i] = IDT_create_entry((uint32_t)INTERRUPT_default);
    }

    //Setup the IDT descriptor
    idt.descriptor.size = 2048; //The IDT table is 2048 bytes large (8 bytes per entry * 256 entries)
    idt.descriptor.base = (uint32_t)(&idt.entries[0]);  //Starts at the first entry

    //Set the new IDT as the current IDT. Also interrupts can now be enabled
    __asm__("lidt %0\n" :: "m"(idt.descriptor));

    //Clear the screen to black
    for (unsigned y = 0; y < VBE_mode_info.height; y++) {
        for (unsigned x = 0; x < VBE_mode_info.width; x++) {
            struct COLOR_rgb pixel_color = {0.f, 0.f, 0.f};
            PIXEL_plot_norm_rgb(x, y, pixel_color);
        }
    }

    k_printf("before interrupt\n");
    __asm__("mov ebx, 0\n"
            "div ebx\n");
    k_printf("after interrupt\n");
    
    halt_forever();

}

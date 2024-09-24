#include <stddef.h>
#include <stdint.h>

#include "vbe.h"

static void halt_forever() {

    __asm__ volatile (
            "halt_loop_%=:\n"
                "hlt\n"
                "jmp halt_loop_%=\n"
                :
            );

}

static void plot_pixel(unsigned x, unsigned y, uint32_t color) {

    uint8_t *const screen_ptr = (uint8_t*)(VBE_mode_info.framebuffer);

    size_t pixel_idx = y*VBE_mode_info.pitch + x*(VBE_mode_info.bpp/8);
    for (unsigned channel = 0; channel < VBE_mode_info.bpp/8; channel++) {
        screen_ptr[pixel_idx + channel] = (color >> (channel*8)) & 0xff;
    }

}

void k_main(__attribute__((unused)) const uint32_t old_vbe_info_ptr_u32, const uint32_t old_vbe_mode_info_ptr_u32) {

    {
        const struct VBE_Info *const old_vbe_info_ptr = (struct VBE_Info*)old_vbe_info_ptr_u32;
        const struct VBE_ModeInfo *const old_vbe_mode_info_ptr = (struct VBE_ModeInfo*)old_vbe_mode_info_ptr_u32;
        VBE_setup_infos(old_vbe_info_ptr, old_vbe_mode_info_ptr);
    }

    for (unsigned y = 0; y < 480; y++) {
        for (unsigned x = 0; x < 640; x++) {
            uint32_t color = (x/3&0xff) |       //R
                            ((y/3&0xff) << 8) | //G
                            ((0x00) << 16) |    //B
                            ((0x00) << 24);     //NONE
            plot_pixel(x, y, color);
        }
    }

    halt_forever();

}

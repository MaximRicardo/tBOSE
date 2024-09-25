#include <stddef.h>
#include <stdint.h>

#include "vbe.h"
#include "pixel.h"

static void halt_forever() {

    __asm__ volatile (
            "halt_loop_%=:\n"
                "hlt\n"
                "jmp halt_loop_%=\n"
                :
            );

}


void k_main(__attribute__((unused)) const uint32_t old_vbe_info_ptr_u32, const uint32_t old_vbe_mode_info_ptr_u32) {

    {
        const struct VBE_Info *const old_vbe_info_ptr = (struct VBE_Info*)old_vbe_info_ptr_u32;
        const struct VBE_ModeInfo *const old_vbe_mode_info_ptr = (struct VBE_ModeInfo*)old_vbe_mode_info_ptr_u32;
        VBE_setup_infos(old_vbe_info_ptr, old_vbe_mode_info_ptr);
    }

    for (unsigned y = 0; y < 480; y++) {
        for (unsigned x = 0; x < 640; x++) {
            PIXEL_plot_norm_rgb(x, y, (float)x/640.f, (float)y/480.f, 1.f);
        }
    }

    halt_forever();

}

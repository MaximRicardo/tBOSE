#include <stddef.h>
#include <stdint.h>

#include "color.h"
#include "vbe.h"
#include "pixel.h"
#include "print.h"

static void halt_forever(void) {

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

    //Clear the screen to black
    for (unsigned y = 0; y < VBE_mode_info.height; y++) {
        for (unsigned x = 0; x < VBE_mode_info.width; x++) {
            struct COLOR_rgb pixel_color = {0.f, 0.f, 0.f};
            PIXEL_plot_norm_rgb(x, y, pixel_color);
        }
    }

    k_printf("k_main is at %p\n", k_main);
    
    halt_forever();

}

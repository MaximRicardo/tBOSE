#include <stddef.h>

#include "pixel.h"
#include "color.h"
#include "vbe.h"

void PIXEL_plot(unsigned x, unsigned y, uint32_t color) {

    uint8_t *const screen_ptr = (uint8_t*)(VBE_mode_info.framebuffer);

    size_t pixel_idx = y*VBE_mode_info.pitch + x*(VBE_mode_info.bpp/8);
    for (unsigned byte = 0; byte < VBE_mode_info.bpp/8; byte++) {
        screen_ptr[pixel_idx + byte] = (color >> (byte*8)) & 0xff;
    }

}

//Plot pixel but with normalized RGB colors
void PIXEL_plot_norm_rgb(unsigned x, unsigned y, struct COLOR_rgb color) {
    
    uint8_t r_channel = color.r*255.f;
    uint8_t g_channel = color.g*255.f;
    uint8_t b_channel = color.b*255.f;

    //For some reason the colors are in reversed order
    uint32_t color_u32 = b_channel |
                    (g_channel << 8) |
                    (r_channel << 16) |
                    0x00000000;

    PIXEL_plot(x, y, color_u32);

}

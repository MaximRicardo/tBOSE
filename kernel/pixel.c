#include <stddef.h>

#include "pixel.h"
#include "vbe.h"

void PIXEL_plot(unsigned x, unsigned y, uint32_t color) {

    uint8_t *const screen_ptr = (uint8_t*)(VBE_mode_info.framebuffer);

    size_t pixel_idx = y*VBE_mode_info.pitch + x*(VBE_mode_info.bpp/8);
    for (unsigned byte = 0; byte < VBE_mode_info.bpp/8; byte++) {
        screen_ptr[pixel_idx + byte] = (color >> (byte*8)) & 0xff;
    }

}

//Plot pixel but with normalized RGB colors
void PIXEL_plot_norm_rgb(unsigned x, unsigned y, float r, float g, float b) {
    
    uint8_t r_channel = r*255.f;
    uint8_t g_channel = g*255.f;
    uint8_t b_channel = b*255.f;

    uint32_t color = r_channel |
                    (g_channel << 8) |
                    (b_channel << 16) |
                    0x00000000;

    PIXEL_plot(x, y, color);

}

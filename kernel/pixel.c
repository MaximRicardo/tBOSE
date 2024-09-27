#include <stddef.h>
#include <stdint.h>

#include "pixel.h"
#include "color.h"
#include "vbe.h"

void PIXEL_plot(unsigned x, unsigned y, uint32_t color) {

    uint8_t *const screen_ptr = (uint8_t*)m_FRAMEBUFFER_VIRTUAL_ADDRESS;

    size_t pixel_idx = y*VBE_mode_info.pitch + x*(VBE_mode_info.bpp/8);
    for (unsigned byte = 0; byte < VBE_mode_info.bpp/8; byte++) {
        screen_ptr[pixel_idx + byte] = (color >> (byte*8)) & 0xff;
    }

}

//Plot pixel but with normalized RGB colors
void PIXEL_plot_norm_rgb(unsigned x, unsigned y, struct COLOR_rgb color) {

    unsigned r_channel_max = (1 << VBE_mode_info.red_mask) - 1;
    unsigned g_channel_max = (1 << VBE_mode_info.green_mask) - 1;
    unsigned b_channel_max = (1 << VBE_mode_info.blue_mask) - 1;

    uint8_t r_channel = color.r*(float)r_channel_max;
    uint8_t g_channel = color.g*(float)g_channel_max;
    uint8_t b_channel = color.b*(float)b_channel_max;

    uint32_t color_u32 = 0;
    color_u32 |= r_channel << VBE_mode_info.red_position;
    color_u32 |= g_channel << VBE_mode_info.green_position;
    color_u32 |= b_channel << VBE_mode_info.blue_position;

    PIXEL_plot(x, y, color_u32);

}

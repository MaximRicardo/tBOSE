#include <stddef.h>
#include <stdint.h>

#include "pixel.h"
#include "color.h"
#include "vbe.h"

struct ColorRGB *Pixel_back_buffer = NULL;

void Pixel_plot(unsigned x, unsigned y, uint32_t color)
{
	uint8_t *const screen_ptr = (uint8_t *)m_FRAMEBUFFER_VIRTUAL_ADDRESS;

	size_t pixel_idx =
		y * VBE_mode_info.pitch + x * (VBE_mode_info.bpp / 8);
	for (unsigned byte = 0; byte < VBE_mode_info.bpp / 8; byte++) {
		screen_ptr[pixel_idx + byte] = (color >> (byte * 8)) & 0xff;
	}
}

//Plot pixel but with normalized RGB colors
void Pixel_plot_norm_rgb(unsigned x, unsigned y, struct ColorRGB color)
{
	unsigned r_channel_max = (1 << VBE_mode_info.red_mask) - 1;
	unsigned g_channel_max = (1 << VBE_mode_info.green_mask) - 1;
	unsigned b_channel_max = (1 << VBE_mode_info.blue_mask) - 1;

	uint8_t r_channel = color.r * (float)r_channel_max;
	uint8_t g_channel = color.g * (float)g_channel_max;
	uint8_t b_channel = color.b * (float)b_channel_max;

	uint32_t color_u32 = 0;
	color_u32 |= r_channel << VBE_mode_info.red_position;
	color_u32 |= g_channel << VBE_mode_info.green_position;
	color_u32 |= b_channel << VBE_mode_info.blue_position;

	Pixel_plot(x, y, color_u32);
}

void Pixel_partially_flip_buffer(unsigned x_start, unsigned y_start,
				 unsigned x_end, unsigned y_end)
{
	if (Pixel_back_buffer == NULL)
		return;

	for (unsigned y = y_start; y <= y_end; y++) {
		for (unsigned x = x_start; x <= x_end; x++) {
			Pixel_plot_norm_rgb(
				x, y,
				Pixel_back_buffer[VBE_mode_info.width * y + x]);
		}
	}
}

void Pixel_flip_buffer(void)
{
	Pixel_partially_flip_buffer(0, 0, VBE_mode_info.width - 1,
				    VBE_mode_info.height - 1);
}

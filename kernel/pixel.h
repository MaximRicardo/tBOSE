#pragma once

#include <stdint.h>

#include "color.h"

#define m_FRAMEBUFFER_VIRTUAL_ADDRESS 0xf0000000
extern struct ColorRGB *Pixel_back_buffer;

/*
 * Renders directly to the front buffer.
 * Plots a pixel by directly writing color to the pixel's address in the frame buffer.
 * If BPP=8, only the LSB of color is used.
 * If BPP=16, only the 2 LSBs of color are used.
 * etc..
 */
void Pixel_plot(unsigned x, unsigned y, uint32_t color);

//Pixel_plot but with normalized RGB colors.
//Renders directly to the front buffer.
void Pixel_plot_norm_rgb(unsigned x, unsigned y, struct ColorRGB color);

//Copies the back buffer to the LFB. If the back buffer is NULL, then this function does nothing.
void Pixel_partially_flip_buffer(unsigned x_start, unsigned y_start,
				 unsigned x_end, unsigned y_end);

//Copies the back buffer to the LFB. If the back buffer is NULL, then this function does nothing.
void Pixel_flip_buffer(void);

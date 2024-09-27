#pragma once

#include <stdint.h>

#include "color.h"

#define m_FRAMEBUFFER_VIRTUAL_ADDRESS 0xf0000000

/*
 * Plots a pixel by directly writing color to the pixel's address in the frame buffer
 * If BPP=8, only the LSB of color is used
 * If BPP=16, only the 2 LSBs of color are used
 * etc..
 */
void PIXEL_plot(unsigned x, unsigned y, uint32_t color);

//PIXEL_plot but with normalized RGB colors
void PIXEL_plot_norm_rgb(unsigned x, unsigned y, struct COLOR_rgb color);

#pragma once

#include <stdint.h>

#include "color.h"

void PIXEL_plot(unsigned x, unsigned y, uint32_t color);

//PIXEL_plot but with normalized RGB colors
void PIXEL_plot_norm_rgb(unsigned x, unsigned y, struct COLOR_rgb color);

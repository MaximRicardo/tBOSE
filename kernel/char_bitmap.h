#pragma once

#include <stdint.h>

extern unsigned CharBitmap_width;
extern unsigned CharBitmap_height;

//starts at ascii 32/0x20 ' ', and ends at ascii 126/0x7e '~'
extern uint8_t CharBitmap_bitmaps[95][13];

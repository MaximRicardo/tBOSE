#pragma once

#include <stdint.h>

struct CHS {
	uint16_t cyl;
	uint16_t head;
	uint16_t sector;
};

struct CHS CHS_lba_to_chs(uint32_t lba, unsigned sectors_per_track);

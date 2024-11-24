#pragma once

#include "../chs.h"
#include <stdint.h>

#define m_FLOPPY_144_SECTORS_PER_TRACK 18

struct CHS FLOPPY_144_lba_to_chs(uint32_t lba);

void FLOPPY_144_reset(void);
void FLOPPY_144_init(void);

void FLOPPY_144_load(struct CHS location, uint32_t n_sectors, void *output);

#pragma once

#include "../chs.h"
#include <stdint.h>

#define m_FLOPPY_144_SECTORS_PER_TRACK 18

struct CHS Floppy144_lba_to_chs(uint32_t lba);

void Floppy144_reset(void);
void Floppy144_init(void);

void Floppy144_load(struct CHS location, uint32_t n_sectors, unsigned drive,
		    void *output);

//Handles interrupt 6
void Floppy144_irq_handler(void);

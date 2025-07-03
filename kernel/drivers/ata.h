#pragma once

#include <stdint.h>

struct ATA_Device {

    uint16_t base;
    uint16_t dev_ctl;
    uint16_t reg_cyl_lo;
    uint16_t reg_cyl_hi;
    uint16_t reg_devsel;

};

extern struct ATA_Device ATA_device;

void ATA_soft_reset(struct ATA_Device *ctrl);

void ATA_read_sector(struct ATA_Device *ctrl, void *data);

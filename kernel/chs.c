#include "chs.h"

struct CHS CHS_lba_to_chs(uint32_t lba, unsigned sectors_per_track) {

    struct CHS chs;

    chs.cyl = lba / (2 * sectors_per_track);
    chs.head = (lba % (2 * sectors_per_track)) / sectors_per_track;
    chs.sector = (lba % (2 * sectors_per_track)) % sectors_per_track + 1;

    return chs;

}

#include <stdint.h>

#include "idt.h"

struct IDT_Entry IDT_create_entry(uint32_t base) {

    struct IDT_Entry entry;

    entry.base_lo = (uint16_t)(base&0xffff);
    entry.base_hi = (uint16_t)((base>>16) & 0xffff);
    entry.sel = m_IDT_SEL_VALUE;
    entry.always_0 = 0;
    entry.flags = m_IDT_FLAGS_VALUE;

    return entry;

}

struct IDT_Entry IDT_create_zero_entry(void) {

    struct IDT_Entry entry;

    entry.base_lo = 0;
    entry.sel = 0;
    entry.always_0 = 0;
    entry.flags = 0;
    entry.base_hi = 0;

    return entry;

}

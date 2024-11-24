#pragma once

#include <stdint.h>

#define m_PIC1            0x20    /* IO base address for master PIC */
#define m_PIC2            0xA0    /* IO base address for slave PIC */
#define m_PIC1_COMMAND    m_PIC1
#define m_PIC1_DATA       (m_PIC1+1)
#define m_PIC2_COMMAND    m_PIC2
#define m_PIC2_DATA       (m_PIC2+1)


#define m_PIC_EOI         0x20    /* End of interrupt command code */


#define m_PIC_ICW1_ICW4       0x01    /* Indicates that ICW4 will be present */
#define m_PIC_ICW1_SINGLE     0x02    /* Single (cascade) mode */
#define m_PIC_ICW1_INTERVAL4  0x04    /* Call address interval 4 (8) */
#define m_PIC_ICW1_LEVEL      0x08    /* Level triggered (edge) mode */
#define m_PIC_ICW1_INIT       0x10    /* Initialization - required! */
#define m_PIC_ICW1_INIT_ICW4  0x11

#define m_PIC_ICW2_MASTER_OFFSET  0x20
#define m_PIC_ICW2_SLAVE_OFFSET   0x28

#define m_PIC_ICW3_MASTER_SLAVE_IRQ   0x4
#define m_PIC_ICW3_SLAVE_MASTER_CASCADE   0x2

#define m_PIC_ICW4_8086       0x01    /* 8086/88 (MCS-80/85) mode */
#define m_PIC_ICW4_AUTO       0x02    /* Auto (normal) EOI */
#define m_PIC_ICW4_BUF_SLACE  0x08    /* Buffered mode/slave */
#define m_PIC_ICW4_BUF_MASTER 0x0c    /* Buffered mode/master */
#define m_PIC_ICW4_SFNM       0x10    /* Special fully nested (not) */


#define m_PIC_READ_IRR    0x0a    /* OCW3 irq ready next CMD read */
#define m_PIC_READ_ISR    0x0b    /* OCW3 irq service next CMD read */

void PIC_send_eoi(uint8_t irq);

void PIC_remap(uint8_t offset_1, uint8_t offset_2);

void PIC_disable(void);

void PIC_irq_set_mask(uint8_t irq_line);
void PIC_irq_clear_mask(uint8_t irq_line);

uint16_t PIC_get_irr(void);
uint16_t PIC_get_isr(void);

void PIC_init(void);

#include "pic.h"
#include "io.h"
#include <stdint.h>

void PIC_send_eoi(uint8_t irq) {

    if (irq >= 8) {
        IO_out_port_b(m_PIC2_COMMAND, m_PIC_EOI);
        IO_wait();
    }

    IO_out_port_b(m_PIC1_COMMAND, m_PIC_EOI);
    IO_wait();

}

void PIC_remap(uint8_t offset_1, uint8_t offset_2) {

    uint8_t a1 = IO_in_port_b(m_PIC1_DATA);
    uint8_t a2 = IO_in_port_b(m_PIC2_DATA);

    IO_out_port_b(m_PIC1_COMMAND, m_PIC_ICW1_INIT | m_PIC_ICW1_ICW4);
    IO_wait();
    IO_out_port_b(m_PIC2_COMMAND, m_PIC_ICW1_INIT | m_PIC_ICW1_ICW4);
    IO_wait();
    IO_out_port_b(m_PIC1_DATA, offset_1);
    IO_wait();
    IO_out_port_b(m_PIC2_DATA, offset_2);
    IO_wait();
    IO_out_port_b(m_PIC1_DATA, 4);
    IO_wait();
    IO_out_port_b(m_PIC2_DATA, 2);
    IO_wait();

    IO_out_port_b(m_PIC1_DATA, m_PIC_ICW4_8086);
    IO_wait();
    IO_out_port_b(m_PIC2_DATA, m_PIC_ICW4_8086);
    IO_wait();

    IO_out_port_b(m_PIC1_DATA, a1);
    IO_out_port_b(m_PIC2_DATA, a2);

}

void PIC_disable(void) {

    IO_out_port_b(m_PIC1_DATA, 0xff);
    IO_out_port_b(m_PIC2_DATA, 0xff);

}

void PIC_irq_set_mask(uint8_t irq_line) {

    uint16_t port;
    if (irq_line < 8)
        port = m_PIC1_DATA;
    else {
        port = m_PIC2_DATA;
        irq_line -= 8;
    }

    uint8_t value = IO_in_port_b(port) | (1 << irq_line);
    IO_out_port_b(port, value);

}

void PIC_irq_clear_mask(uint8_t irq_line) {

    uint16_t port;
    if (irq_line < 8)
        port = m_PIC1_DATA;
    else {
        port = m_PIC2_DATA;
        irq_line -= 8;
    }

    uint8_t value = IO_in_port_b(port) & ~(1 << irq_line);
    IO_out_port_b(port, value);

}

static uint16_t get_irq_reg(uint8_t ocw3) {

    IO_out_port_b(m_PIC1_COMMAND, ocw3);
    IO_out_port_b(m_PIC2_COMMAND, ocw3);

    uint16_t value = IO_in_port_b(m_PIC2_COMMAND) << 8 | IO_in_port_b(m_PIC1_COMMAND);
    return value;

}

uint16_t PIC_get_irr(void) {

    return get_irq_reg(m_PIC_READ_IRR);

}

uint16_t PIC_get_isr(void) {

    return get_irq_reg(m_PIC_READ_ISR);

}

void PIC_init(void) {

    uint8_t master_mask = 0xfc;
    uint8_t slave_mask = 0xff;

    IO_out_port_b(m_PIC1_COMMAND, m_PIC_ICW1_INIT_ICW4);
    IO_wait();
    IO_out_port_b(m_PIC2_COMMAND, m_PIC_ICW1_INIT_ICW4);
    IO_wait();

    IO_out_port_b(m_PIC1_DATA, m_PIC_ICW2_MASTER_OFFSET);
    IO_wait();
    IO_out_port_b(m_PIC2_DATA, m_PIC_ICW2_SLAVE_OFFSET);
    IO_wait();

    IO_out_port_b(m_PIC1_DATA, m_PIC_ICW3_MASTER_SLAVE_IRQ);
    IO_wait();
    IO_out_port_b(m_PIC2_DATA, m_PIC_ICW3_SLAVE_MASTER_CASCADE);
    IO_wait();

    IO_out_port_b(m_PIC1_DATA, m_PIC_ICW4_8086);
    IO_wait();
    IO_out_port_b(m_PIC2_DATA, m_PIC_ICW4_8086);
    IO_wait();

    IO_out_port_b(m_PIC1_DATA, master_mask);
    IO_wait();
    IO_out_port_b(m_PIC2_DATA, slave_mask);
    IO_wait();

}

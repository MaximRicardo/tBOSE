#include "io.h"
#include <stdint.h>

uint8_t IO_in_port_b(unsigned port)
{
	uint8_t ret;
	__asm__ volatile("inb %%dx,%%al" : "=a"(ret) : "d"(port));
	return ret;
}

void IO_out_port_b(unsigned port, uint8_t value)
{
	__asm__ volatile("outb %%al,%%dx" : : "d"(port), "a"(value));
}

void IO_wait(void)
{
	IO_out_port_b(0x80, 0);
}

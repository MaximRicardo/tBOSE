#pragma once

#include <stdint.h>

//Input a byte from a port
uint8_t IO_in_port_b(unsigned port);

//Output a byte to a port
void IO_out_port_b(unsigned port, uint8_t value);

void IO_wait(void);

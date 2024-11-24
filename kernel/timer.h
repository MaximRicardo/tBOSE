#pragma once

#include <stdint.h>

//Counts the number of milliseconds since the PIT started sending interrupts.
//Will wrap around after just over an hour, so it may be way off.
extern uint32_t TIMER_n_ms;

void TIMER_wait(uint32_t ms);

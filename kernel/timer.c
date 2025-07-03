#include "timer.h"
#include <stdint.h>

uint32_t Timer_n_ms = 0;

void Timer_wait(uint32_t ms)
{
	uint32_t start_ms = Timer_n_ms;
	uint32_t cur_ms = start_ms;

	while (cur_ms - start_ms < ms) {
		__asm__ volatile("hlt");
		cur_ms = Timer_n_ms;
	}
}

#include "print.h"
#include "pic.h"
#include "timer.h"
#include <stdint.h>

void INTERRUPT_default_handler(void *p, unsigned int_type)
{
	uint32_t esp;
	__asm__ volatile("mov %%esp, %0\n" : "=r"(esp));

	k_printf("INTERRUPT %u, AT ADDRESS: %p, ESP: 0x%08x\n", int_type, p,
		 esp);
}

void INTERRUPT_pit_int_handler(void)
{
	++TIMER_n_ms;
	PIC_send_eoi(32);
}

void INTERRUPT_others_handler(void)
{
	k_printf("MISC INTERRUPT!\n");
}

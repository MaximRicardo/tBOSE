#include "print.h"
#include "pic.h"
#include "timer.h"
#include <stdint.h>

void Interrupt_default_handler(void *p, unsigned int_type)
{
	uint32_t esp;
	__asm__ volatile("mov %%esp, %0\n" : "=r"(esp));

	k_printf("INTERRUPT %u, AT ADDRESS: %p, ESP: 0x%08x\n", int_type, p,
		 esp);
}

void Interrupt_pit_int_handler(void)
{
	++Timer_n_ms;
	PIC_send_eoi(32);
}

void Interrupt_others_handler(void)
{
	k_printf("MISC INTERRUPT!\n");
}

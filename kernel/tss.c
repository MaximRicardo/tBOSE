#include "tss.h"

void TSS_update(void)
{
	__asm__ volatile("mov $0x28, %ax\n"
			 "ltr %ax\n");
}

#include "print.h"
#include <stdint.h>

void INTERRUPT_default_handler(void* p, unsigned int_type) {

    uint32_t esp;
    __asm__ volatile(
            "mov %%esp, %0\n"
            : "=r" (esp)
            );

    k_printf("INTERRUPT %u, AT ADDRESS: %p, ESP: 0x%08x\n", int_type, p, esp);

}

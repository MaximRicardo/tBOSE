

#include "print.h"

void INTERRUPT_default_handler(void* p, unsigned int_type) {

    k_printf("INTERRUPT %u, AT ADDRESS: %p\n", int_type, p);

}

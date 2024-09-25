

#include "print.h"

void INTERRUPT_default_handler(void* p) {

    k_printf("INTERRUPT AT ADDRESS: %p\n", p);

}

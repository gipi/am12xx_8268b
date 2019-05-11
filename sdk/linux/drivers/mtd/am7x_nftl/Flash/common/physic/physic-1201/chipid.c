#if ! __KERNEL__
#include "syscfg.h"

/*
 *  Force the variable to be placed in the .data section rather
 *  than the .bss or .sbss section.
 */
struct am7x_chipinfo am7x_chipinfo __attribute__((section(".data"))) ;

#endif


#ifndef __ARCH_X86_64_SYM_INTERRUPTS__
#define __ARCH_X86_64_SYM_INTERRUPTS__

#define LOAD_IDT __asm__ __volatile__("lidt %0" : :  "m"(*location) );

#define STORE_IDT __asm__ __volatile__("sidt %0" : : "m"(*location) : "memory");
#endif

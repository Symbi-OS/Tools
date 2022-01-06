#ifndef __ARCH_X86_64_SYM_LIB_HACKS__
#define __ARCH_X86_64_SYM_LIB_HACKS__

#define PUSH_JUNK __asm__("pushq $42":::"memory");

#define POP_JUNK  __asm__("popq %%rax": /*no*/: /*no*/ : "rax");

#endif

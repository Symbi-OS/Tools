#ifndef __SYM_PROBE_LIB__
#define __SYM_PROBE_LIB__
#include "L0/sym_structs.h"

#ifdef CONFIG_X86_64
#include "../../arch/x86_64/L2/sym_probe.h"
#endif
// License C 2021-
// Author: Thomas Unger
// Level: 2

#define X86_TRAP_BP		 3

#define MY_DECREMENT_RETURN_RIP __asm__("decq (%rsp)");
#define MY_MY_IRET __asm__("iretq");

#define MY_INT3_HANDLER(LAB, TARG)              \
  MY_NEW_HANDLER(LAB)                           \
    MY_DECREMENT_RETURN_RIP                     \
    MY_PUSH_REGS                                \
    MY_MY_CALL(TARG)                            \
    MY_POP_REGS                                 \
    MY_MY_IRET

// Place a software interrupt generating instruction at addr.
unsigned char sym_set_probe(uint64_t addr);

// Replace software interrupt generating instruction with byte.
void sym_remove_probe(void *addr, unsigned char old_byte);

void sym_interpose_on_int3_ft_asm(unsigned char* new_idt);
void sym_interpose_on_int3_ft_c(unsigned char* new_idt);

#endif

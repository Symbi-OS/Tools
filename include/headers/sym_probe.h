#ifndef __SYM_PROBE_LIB__
#define __SYM_PROBE_LIB__
#include "./sym_structs.h"

// License C 2021-
// Author: Thomas Unger
// Level: 2

#define X86_TRAP_BP		 3

// Place a software interrupt generating instruction at addr.
unsigned char sym_set_probe(uint64_t addr);

// Replace software interrupt generating instruction with byte.
void sym_remove_probe(void *addr, unsigned char old_byte);

void sym_interpose_on_int3_ft_asm(unsigned char* new_idt);
void sym_interpose_on_int3_ft_c(unsigned char* new_idt);

#endif

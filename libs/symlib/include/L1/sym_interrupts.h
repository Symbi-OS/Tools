#ifndef __SYM_INTERRUPTS__
#define __SYM_INTERRUPTS__
#include "L0/sym_structs.h"

// License C 2021-
// Author: Thomas Unger
// Level: 1

// Memcpy the idt
// This is the kernel location for the IDT, it's one page in length.
void sym_copy_system_idt(unsigned char *sym_idt_base);
void * sym_memcpy(void * dest, void *src, size_t sz);

// Load idtr with raw base and bound
void sym_set_idtr(unsigned long base, unsigned long bound);

// Store into IDTR with struct dtr *
void sym_load_idtr(struct dtr *location);

// Get IDTR
void sym_store_idt_desc(struct dtr *location);
void sym_set_idt_desc(unsigned char *idt_base, unsigned int idx, union idt_desc *new_desc);
union idt_desc * sym_get_idt_desc(unsigned char *idt_base, unsigned int idx);

void * sym_get_addr_from_desc(union idt_desc *desc);
void * sym_update_desc_handler(union idt_desc *desc, void *p);

void sym_load_addr_from_desc(union idt_desc *desc, union idt_addr *addr);

void sym_load_desc_from_addr(union idt_desc *desc, union idt_addr *addr);

// TODO: Fix this first arg type ...
void sym_print_idt_desc(unsigned char *idt, unsigned int idx);
#endif

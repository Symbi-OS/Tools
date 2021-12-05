#ifndef __SYM_INTERRUPTS__
#define __SYM_INTERRUPTS__
#include "./sym_structs.h"

void sym_copy_system_idt(unsigned char *sym_idt_base);
void sym_set_idtr(unsigned long base, unsigned long bound);
void sym_load_idtr(struct dtr *location);
void sym_store_idt_desc(struct dtr *location);
void sym_set_idt_desc(unsigned char *idt_base, unsigned int idx, union idt_desc *new_desc);
union idt_desc * sym_get_idt_desc(unsigned char *idt_base, unsigned int idx);

void sym_load_addr_from_desc(union idt_desc *desc, union idt_addr *addr);

void sym_load_desc_from_addr(union idt_desc *desc, union idt_addr *addr);

void sym_print_idt_desc(unsigned char *idt, unsigned int idx);
#endif

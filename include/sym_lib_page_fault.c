#include "headers/sym_lib_page_fault.h"
#include "headers/sym_interrupts.h"
#include "headers/sym_lib.h"


// XXX global val
// This is the old handler we jmp to after our interposer.
uint64_t orig_asm_exc_page_fault;

// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
extern uint64_t bs_asm_exc_page_fault;

/* .global bs_asm_exc_page_fault \n\t\ */
asm("\
 .text                         \n\t\
 .align 16                     \n\t\
 bs_asm_exc_page_fault:        \n\t\
 pushq %rsi                    \n\t\
 movq 8(%rsp),%rsi             \n\t\
 orq $0x4, %rsi                \n\t\
 movq %rsi, 8(%rsp)            \n\t\
 popq %rsi                     \n\t\
 jmp *orig_asm_exc_page_fault      \
");

void sym_interpose_on_pg_ft(char * my_idt){
  // Get ptr to pf desc
  union idt_desc *desc_old = sym_get_idt_desc(my_idt, PG_FT_IDX);

  // save old asm_exc_pf ptr
  union idt_addr old_asm_exc_pf;
  sym_load_addr_from_desc(desc_old, &old_asm_exc_pf);

  // swing addr to  bs_asm...
  orig_asm_exc_page_fault = old_asm_exc_pf.raw;

  // New handler
  union idt_addr new_asm_exc_addr;
  new_asm_exc_addr.raw = (uint64_t) &bs_asm_exc_page_fault;

  // Set IDT to point to our new interposer
  sym_load_desc_from_addr(desc_old, &new_asm_exc_addr);
}

typedef void* (*lookup_address_t)(uint64_t address, unsigned int * level);
void sym_make_pg_writable(uint64_t addr){
  sym_elevate();
  // Get PTE
  lookup_address_t my_lookup_address = (lookup_address_t) 0xffffffff8105ce60;
  unsigned int level;
  void* ret = my_lookup_address(addr, &level);

  struct pte* pte_p = (struct pte*) ret;
  pte_p->RW = 1;
  sym_lower();
}

void sym_make_pg_unwritable(uint64_t addr){
  sym_elevate();
  // Get PTE
  lookup_address_t my_lookup_address = (lookup_address_t) 0xffffffff8105ce60;
  unsigned int level;
  void* ret = my_lookup_address(addr, &level);

  struct pte* pte_p = (struct pte*) ret;
  pte_p->RW = 0;
  sym_lower();
}
/* void write_ktext_addr(uint64_t addr, char *s, int len){ */
/*   // TODO if write spans pages, this will fail. */
/*   sym_make_pte_writable(addr); */
/*   sym_elevate(); */
/*   memcpy((void *)addr, s, len); */
/* } */

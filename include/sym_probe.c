#include "headers/sym_probe.h"

#include "headers/sym_interrupts.h"


// This is the old handler we jmp to after our interposer.
uint64_t orig_asm_exc_int3;

// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
extern uint64_t bs_asm_exc_int3;

/* uint64_t int3_ctr = 0; */

asm(" \
 .text                         \n\t\
 .align 16                     \n\t\
 .global bs_asm_exc_int3        \n\t\
 bs_asm_exc_int3:              \n\t\
 movb $0x55, 0xffffffff8107dbf0 \n\t\
 decq (%rsp)                    \n\t\
 iretq                             \
");

void sym_interpose_on_int3_ft(char * my_idt){
  sym_print_idt_desc(my_idt, X86_TRAP_BP);

  // Get ptr to pf desc
  union idt_desc *desc_old;
  desc_old = sym_get_idt_desc(my_idt, X86_TRAP_BP);

  // save old asm_exc_pf ptr
  union idt_addr old_asm_exc_int3;
  sym_load_addr_from_desc(desc_old, &old_asm_exc_int3);

  // swing addr to  bs_asm...
  orig_asm_exc_int3 = old_asm_exc_int3.raw;

  // New handler
  union idt_addr new_asm_exc_addr;
  new_asm_exc_addr.raw = (uint64_t) &bs_asm_exc_int3;

  // Set IDT to point to our new interposer
  sym_load_desc_from_addr(desc_old, &new_asm_exc_addr);

  sym_print_idt_desc(my_idt,  X86_TRAP_BP);

}

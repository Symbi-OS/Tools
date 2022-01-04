#include "L2/sym_probe.h"
#include "L0/sym_lib.h"
#include "L1/sym_interrupts.h"
#include "L2/sym_lib_page_fault.h"

// TODO turn this into a header?
asm(".include \"../arch/x86/arch_x86.S\"");

// This is the old handler we jmp to after our interposer.
uint64_t orig_asm_exc_int3;

uint64_t cr3_reg = 0;


static void (*myprintk)(char *) = (void *)0xffffffff81c34d5b;
/* static void (*myprintki)(char *, int) = (void *)0xffffffff81c34d5b; */

unsigned char reset_byte = 0xf;

static void tu_c_entry(){
  myprintk("hey\n");

  uint64_t my_cr3;
  asm("movq %%cr3,%0" : "=r"(my_cr3));
  if(cr3_reg == my_cr3){
    myprintk("wow, cr3 matches\n");

    myprintk("Need to swing overwritten byte back to 0xf\n");
    unsigned char *ucp = (unsigned char *) 0xffffffff810fbfe0;
    *ucp = reset_byte;
  }else{
    myprintk("Bummer, no match\n");
  }
}

// HACK: why?
static uint64_t my_entry = (uint64_t) &tu_c_entry;

extern uint64_t int3_jmp_to_c;
asm(" \
 .text                          \n\t\
 .align 16                      \n\t\
 .global int3_jmp_to_c          \n\t\
 int3_jmp_to_c:                 \n\t\
 DECREMENT_RETURN_RIP           \n\t\
 PUSH_REGS                      \n\t\
 MY_CALL *my_entry              \n\t\
 POP_REGS                       \n\t\
 MY_IRET                            \
");

// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
extern uint64_t bs_asm_exc_int3;


void sym_interpose_on_int3_ft_c(unsigned char * my_idt){
  /* sym_print_idt_desc(my_idt, X86_TRAP_BP); */

  // Get ptr to pf desc
  union idt_desc *desc_old;
  desc_old = sym_get_idt_desc(my_idt, X86_TRAP_BP);

  // save old asm_exc_pf ptr
  union idt_addr old_asm_exc_int3;
  sym_load_addr_from_desc(desc_old, &old_asm_exc_int3);

  // This is stored, but not used.
  orig_asm_exc_int3 = old_asm_exc_int3.raw;

  // New handler
  union idt_addr new_asm_exc_addr;
  new_asm_exc_addr.raw = (uint64_t) &int3_jmp_to_c;

  // Set IDT to point to our new interposer
  sym_load_desc_from_addr(desc_old, &new_asm_exc_addr);

  /* sym_print_idt_desc(my_idt,  X86_TRAP_BP); */
}

unsigned char sym_set_probe(uint64_t addr){
  // TODO if write spans pages, this will fail.
  sym_elevate();
  unsigned char ret = *(unsigned char *) addr;
  sym_lower();
  sym_make_pg_writable(addr);

  sym_elevate();
  // Magic write int3 instruction.
  *(unsigned char *) addr = 0xcc;
  sym_lower();

  return ret;
}

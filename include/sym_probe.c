#include "headers/sym_probe.h"
#include "headers/sym_lib.h"
#include "headers/sym_interrupts.h"
#include "headers/sym_lib_page_fault.h"


// This is the old handler we jmp to after our interposer.
uint64_t orig_asm_exc_int3;

uint64_t cr3_reg;


static void (*myprintk)(char *) = (void *)0xffffffff81bd6f95;
static void (*myprintki)(char *, int) = (void *)0xffffffff81bd6f95;

static void tu_c_entry(){
  char *p = "hey\n";
  myprintk(p);

  uint64_t my_cr3;
  asm("movq %%cr3,%0" : "=r"(my_cr3));

  myprintki("got %x\n", my_cr3);

  if(cr3_reg == my_cr3){
    char *p = "wow, cr3 matches\n";
    myprintk(p);
  }else{
    char *p = "Bummer, no match\n";
    myprintk(p);
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
 movb $0x55, 0xffffffff8107dbf0 \n\t\
 decq (%rsp)                    \n\t\
 pushq %rbx \n\t\
 pushq %r12 \n\t\
 pushq %r13 \n\t\
 pushq %r14 \n\t\
 pushq %r15 \n\t\
 pushq %rbp \n\t\
 call *my_entry               \n\t\
 popq %rbp \n\t\
 popq %r15 \n\t\
 popq %r14 \n\t\
 popq %r13 \n\t\
 popq %r12 \n\t\
 popq %rbx \n\t\
 iretq                             \
");

/* jmp .                          \n\t\ */

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

void sym_interpose_on_int3_ft_asm(char * my_idt){
  /* sym_print_idt_desc(my_idt, X86_TRAP_BP); */

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

  /* sym_print_idt_desc(my_idt,  X86_TRAP_BP); */
}

void sym_interpose_on_int3_ft_c(char * my_idt){
  /* sym_print_idt_desc(my_idt, X86_TRAP_BP); */

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
}

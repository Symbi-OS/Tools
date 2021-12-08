#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>

#include "../../include/headers/sym_all.h"

#define X86_TRAP_BP		 3	

extern uint64_t get_PTE(uint64_t addr);

// Our version of the idt. Not sure about alignment.
unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

// This is the old handler we jmp to after our interposer.
uint64_t orig_asm_exc_int3;

// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
extern uint64_t bs_asm_exc_int3;

/*

Exception frame 0 -> 0 Interrupt;
-----------------------
SS         (RSP + 40)
RSP        (RSP + 32)
RFLAGS     (RSP + 24)
CS         (RSP + 16)
RIP        (RSP + 8)
ERROR CODE (<- RSP)
-----------------------


Page fault error codes: 
 31              15                             4               0
+---+--  --+---+-----+---+--  --+---+----+----+---+---+---+---+---+
|   Reserved   | SGX |   Reserved   | SS | PK | I | R | U | W | P |
+---+--  --+---+-----+---+--  --+---+----+----+---+---+---+---+---+
Legend: Set (Clear)
0 Present (not)
1 Write (Read)
2 User (Supervisor)
3 Reserved bit set in page directory (not)
4 Inst fetch

Our code for interposing on text fault.
.text                       Lives in text.
.align 16                   All the rest look 16 byte aligned.
bs_asm_exc_page_fault:      Our handler, will be placed in IDT.
pushq %rsi                  Preserve old rsi (Think just a user reg).
movq 8(%rsp),%rsi           Grab error code from HW pushed exception frame.
orq $0x4, %rsi              Set user bit lying that we were in ring 3
movq %rsi, 8(%rsp)          Throw val back in error code on stack.
popq %rsi                   Restore user rsi.
jmp *orig_asm_exc_page_fault
*/
uint64_t int3_ctr = 0;

asm(" \
 .text                         \n\t\
 .align 16                     \n\t\
 .global bs_asm_exc_int3        \n\t\
 bs_asm_exc_int3:              \n\t\
 incq int3_ctr                  \n\t\
 movb $0x55, 0xffffffff8107dbf0 \n\t\
 decq (%rsp)                    \n\t\
 iretq                             \
");

#if 0 
 pushq %rsi                    \n\t\
 movq 8(%rsp),%rsi             \n\t\
 orq $0x4, %rsi                \n\t\
 movq %rsi, 8(%rsp)            \n\t\
 popq %rsi                     \n\t\
 jmp *orig_asm_exc_page_fault      
#endif

void interpose_on_int3_ft(){
  // Copy the system idt to userspace
  sym_copy_system_idt(my_idt);

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

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

struct pte{
uint64_t
  SEL : 1,
  RW : 1,
  US : 1,
  PWT: 1,
  PCD: 1,
  A : 1,
  DIRTY: 1,
  MAPS : 1,
  WHOCARES2 : 4,
  PG_TBL_ADDR : 40,
  RES : 11,
  XD  : 1;
};

/* #define SMALL_PG_SHIFT 12 */
/* void my_get_pte(uint64_t addr){ */

/*   uint64_t cr3_reg; */
/*   asm("movq %%cr3,%0" */
/*       : "=r"(cr3_reg) */
/*       ); */

/*   printf("Cr3 holds %p\n", cr3_reg); */
/*   struct pte* my_pte; */
/*   my_pte = (struct pte *) &cr3_reg; */

/*   uint64_t pml4 = my_pte->PG_TBL_ADDR << SMALL_PG_SHIFT; */
/*   printf("PML4 lives at %llx\n", pml4); */

/*   // now get pdpt */
/*   uint64_t pdpt = pml4 + (addr >> (12 + 9 +9 + 9) ) & ( (1 << 9) - 1 ); */

/*   printf("PDPT lives at %llx\n", pml4 + pdpt); */

/*   /\* nestTableOrFrame(); *\/ */

/*   while(1); */

/* } */

/* void printCommon(struct pte* decompCommon) { */
/*   printf("\t|                       |     1G                                    |                                 |P|     | |M| | |P|P|U|R|S|\n"); */
/*   printf("\t|       Reserved        |               2M          Address of PTE Table              |               |A| IGN |G|A|D|A|C|W|/|/|E|\n"); */
/*   printf("\t|                       |                                  4K                                         |T|     | |P| | |D|T|S|W|L|\n"); */
/*   printf("\t|"); */
/*   printBits(decompCommon.RES,         12); printf("|"); */
/*   printBits(decompCommon.PG_TBL_ADDR, 40); printf("|"); */
/*   printBits(decompCommon.WHOCARES2,   4); printf("|"); */
/*   printBits(decompCommon.MAPS,        1); printf("|"); */
/*   printBits(decompCommon.DIRTY,       1); printf("|"); */
/*   printBits(decompCommon.A,           1); printf("|"); */
/*   printBits(decompCommon.PCD,         1); printf("|"); */
/*   printBits(decompCommon.PWT,         1); printf("|"); */
/*   printBits(decompCommon.US,          1); printf("|"); */
/*   printBits(decompCommon.RW,          1); printf("|"); */
/*   printBits(decompCommon.SEL,         1); printf("|\n"); */
/*   underlineNibbles(); */
/*   printNibblesHex(); */
/* } */
typedef void* (*lookup_address_t)(uint64_t address, unsigned int * level);

void modify_pt(uint64_t addr){

  // GET PTE
  /* get_PTE(addr); */

  lookup_address_t my_lookup_address = (lookup_address_t) 0xffffffff8105ce60;
  unsigned int level;
  void* ret = my_lookup_address(addr, &level);
  printf("got %p for a pte ptr\n", ret);

  struct pte* pte_p = (struct pte*) ret;
  printf("RW bit is %d\n", pte_p->RW);
  pte_p->RW = 1;
  printf("RW bit is %d\n", pte_p->RW);
}

void write_ktext_addr(uint64_t addr, char *s, int len){
  modify_pt(addr);
  sym_elevate();
  memcpy((void *)addr, s, len);
}

void show_int_interposition_works(){

  sym_elevate();

  interpose_on_int3_ft();

  sym_elevate();
  /* ffffffff8107dbf0 t __do_sys_getpid */
  printf("first byte of getpid is %#x \n", *((unsigned char *) 0xffffffff8107dbf0 ));

  char s = 0xcc;
  write_ktext_addr(0xffffffff8107dbf0, &s, 1);

  getpid();
  getpid();

}



uint64_t orig_asm_exc_page_fault;

// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
extern uint64_t bs_asm_exc_page_fault;


// Fixes  #PF: supervisor instruction fetch in kernel mode
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

void interpose_on_pg_ft(){
  int PG_FT_IDX= 14;
  // Copy the system idt to userspace
  sym_copy_system_idt(my_idt);

  sym_print_idt_desc(my_idt, PG_FT_IDX);

  // Get ptr to pf desc
  union idt_desc *desc_old;
  desc_old = sym_get_idt_desc(my_idt, PG_FT_IDX);

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

  sym_print_idt_desc(my_idt, PG_FT_IDX);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

int main(){
  sym_touch_every_page_text();

  printf("Starting main\n");

  // Store system idtr here for later restoration.
  struct dtr system_idtr;
 
  // Store initial system IDTR
  sym_store_idt_desc(&system_idtr);

  /* interpose_on_pg_ft(); */

  show_int_interposition_works();

  printf("int3_ctr is %lld\n",int3_ctr);
  printf("Done main\n");
  // Swing back onto system idtr before exit
  sym_load_idtr(&system_idtr);
}



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

// HACK
extern uint64_t cr3_reg;

// HACK
static void (*myprintk)(char *) = (void *)0xffffffff81bd6f95;
static void (*myprintki)(char *, uint64_t) = (void *)0xffffffff81bd6f95;

static struct excep_frame *ef = NULL;
static void print_ef(){

  myprintki("ef->err  %#llx\n", ef->err);
  myprintki("ef->rip  %#llx\n", ef->rip);
  myprintki("ef->cs   %#llx\n", ef->cs);
  myprintki("ef->flag %#llx\n", ef->flag);
  myprintki("ef->rsp  %#llx\n", ef->rsp);
  myprintki("ef->ss   %#llx\n", ef->ss);
}

static void pg_ft_c_entry(){
  uint64_t my_cr3 = 0;
  // This might be slow, I don't know.
  asm("movq %%cr3,%0" : "=r"(my_cr3));

  if(!cr3_reg){
    myprintk("Error, cr3_reg never set\n");
    while(1);
  }

  if(cr3_reg  != my_cr3){
    char *p = "Bummer, no match\n";
    myprintk(p);
    myprintk("This is unsupported :/ \n");
    while(1);
  }

  // Are we an instruction fetch?
  if(ef->err & INS_FETCH){
    // Are we user code?
    // Could look in cr2, but by def rip caused the fault here.
    // This is modeled after kern:fault_in_kernel_space
    if(ef->rip < ( (1UL << 47) - 4096) ){
      /// Lie that code was running in user mode.
      ef->err |= USER_FT;
      myprintk("swinging err code for\n");
      print_ef();
    }
  }
}

static uint64_t my_entry = (uint64_t) &pg_ft_c_entry;
extern uint64_t c_handler_page_fault;
asm("\
 .text                         \n\t\
 .align 16                     \n\t\
 c_handler_page_fault:        \n\t\
 movq %rsp, ef \n\t\
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
 jmp *orig_asm_exc_page_fault      \
");
/* pushq %rsi                    \n\t\ */
/* movq 8(%rsp),%rsi             \n\t\ */
/*   orq $0x4, %rsi                \n\t\ */
/*   movq %rsi, 8(%rsp)            \n\t\ */
/*   popq %rsi                     \n\t\ */

void sym_interpose_on_pg_ft_c(char * my_idt){
  // Get ptr to pf desc
  union idt_desc *desc_old = sym_get_idt_desc(my_idt, PG_FT_IDX);

  // save old asm_exc_pf ptr
  union idt_addr old_asm_exc_pf;
  sym_load_addr_from_desc(desc_old, &old_asm_exc_pf);

  // swing addr to  bs_asm...
  orig_asm_exc_page_fault = old_asm_exc_pf.raw;

  // New handler
  union idt_addr new_asm_exc_addr;
  new_asm_exc_addr.raw = (uint64_t) &c_handler_page_fault;

  // Set IDT to point to our new interposer
  sym_load_desc_from_addr(desc_old, &new_asm_exc_addr);
}

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

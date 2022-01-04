#include "headers/sym_lib_page_fault.h"
#include "headers/sym_interrupts.h"
#include "headers/sym_lib.h"


asm(".include \"headers/arch_x86.S\"");

// XXX global val
// This is the old handler we jmp to after our interposer.
uint64_t orig_asm_exc_page_fault;

uint64_t orig_asm_exc_double_fault;

// This is the old handler we jmp to after our interposer.
/* uint64_t orig_asm_exc_double_fault; //= 0xffffffff81c3e2b0; */
// NOTE: In df handler, make sure not to jump to new interposed version.
uint64_t my_asm_exc_page_fault = 0xffffffff81e00ac0;

// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
extern uint64_t bs_asm_exc_page_fault;

// HACK
extern uint64_t cr3_reg;

// HACK
static void (*myprintk)(char *) = (void *)0xffffffff81c34d5b;
static void (*myprintki)(char *, uint64_t) = (void *)0xffffffff81c34d5b;

__attribute__((aligned (16)))
static struct excep_frame *ef = NULL;
__attribute__((aligned (16)))
static void print_ef(){

  myprintki("ef->err  %#llx\n", ef->err);
  myprintki("ef->rip  %#llx\n", ef->rip);
  myprintki("ef->cs   %#llx\n", ef->cs);
  myprintki("ef->flag %#llx\n", ef->flag);
  myprintki("ef->rsp  %#llx\n", ef->rsp);
  myprintki("ef->ss   %#llx\n", ef->ss);
}
int my_ctr = 0;
__attribute__((aligned (16)))
static void pg_ft_c_entry(){
  /* myprintk("my handler!\n"); */
  /* print_ef(); */

  /* ef->err |= USER_FT; */
  uint64_t my_cr3;
  /* This might be slow, I don't know. */
  asm("movq %%cr3,%0" : "=r"(my_cr3));

  if(!cr3_reg){
    /* myprintk("Error, cr3_reg never set\n"); */
    while(1);
  }

  if(cr3_reg  != my_cr3){
    char *p = "Bummer, no match\n";
    /* myprintk(p); */
    /* myprintk("This is unsupported :/ \n"); */
    while(1);
  }

  /* Are we an instruction fetch? */
  if(ef->err & INS_FETCH){
    // We don't need to special case when in ring 3.
    if(! (ef->err & USER_FT)  ){
      // Are we user code?
      // Could look in cr2, but by def rip caused the fault here.
      // This is modeled after kern:fault_in_kernel_space
      if(ef->rip < ( (1UL << 47) - 4096) ){
        /// Lie that code was running in user mode.
        ef->err |= USER_FT;
        /* myprintk("swinging err code for\n"); */
        /* print_ef(); */
        /* myprintki("my_ctr %d\n", my_ctr++); */
      }
    }
  }
}

// Preserve caller saved GPRs.
// Want RSP to be 16byte aligned after call.
static uint64_t my_entry = (uint64_t) &pg_ft_c_entry;
extern uint64_t c_handler_page_fault;
asm("\
 NEW_HANDLER c_handler_page_fault       \n\t\
 GET_EXCP_FRAME                 \n\t\
 PUSH_REGS                      \n\t\
 MY_CALL *my_entry              \n\t\
 POP_REGS                       \n\t\
 MY_JUMP *orig_asm_exc_page_fault      \
");

__attribute__((aligned (16)))
static void df_c_entry(){
  // Error code on DF is 0, 
  ef->err = USER_FT | WR_FT;
}

static uint64_t my_df_entry = (uint64_t) &df_c_entry;
// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
extern uint64_t c_df_handler;
// Want this to fix double faults.
// Why is asm_exc_page_fault going to work this time?
// DF runs on known good IST stack unlike pg ft.
// Likely at a performance penalty, should be rare path.
// See kernel mode linux "Stack Starvation".

asm("\
 NEW_HANDLER c_df_handler       \n\t\
 GET_EXCP_FRAME                 \n\t\
 PUSH_REGS                      \n\t\
 MY_CALL *my_df_entry           \n\t\
 POP_REGS                       \n\t\
 MY_JUMP *my_asm_exc_page_fault     \
");

void sym_interpose_on_df_c(unsigned char * my_idt){
  // Get ptr to df desc
  union idt_desc *desc_old;
  desc_old = sym_get_idt_desc(my_idt, DF_IDX);

  // save old asm_exc_pf ptr
  union idt_addr old_asm_exc_df;
  sym_load_addr_from_desc(desc_old, &old_asm_exc_df);

  // Next line breaks

  // swing addr to  bs_asm...
  orig_asm_exc_double_fault = old_asm_exc_df.raw;

  // New handler
  union idt_addr new_asm_exc_addr;
  new_asm_exc_addr.raw = (uint64_t) &c_df_handler;

  // Set IDT to point to our new interposer
  sym_load_desc_from_addr(desc_old, &new_asm_exc_addr);
}

void sym_interpose_on_pg_ft_c(unsigned char * my_idt){
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

void sym_make_pg_ft_use_ist(unsigned char *my_idt){
  union idt_desc *desc_old;
  union idt_desc desc_new;

  // Get ptr to pf desc
  desc_old = sym_get_idt_desc(my_idt, PG_FT_IDX);

  int DF_IST = 1;
  // Copy descriptor to local var
  sym_elevate();
  desc_new = *desc_old;
  sym_lower();

  // Force IST usage
  desc_new.fields.ist = DF_IST;

  // Write into user table
  sym_set_idt_desc(my_idt, PG_FT_IDX, &desc_new);
}


typedef void* (*lookup_address_t)(uint64_t address, unsigned int * level);

struct pte *
sym_get_pte(uint64_t addr, unsigned int *level)
{
  static lookup_address_t my_lookup_address = NULL;
  if (my_lookup_address == NULL) my_lookup_address = (lookup_address_t) 0xffffffff8107f7d0;
  return (struct pte *) my_lookup_address(addr, level);
}

void sym_make_pg_writable(uint64_t addr){
  sym_elevate();
  // Get PTE
  lookup_address_t my_lookup_address = (lookup_address_t) 0xffffffff8107f7d0;
  unsigned int level;
  void* ret = my_lookup_address(addr, &level);

  struct pte* pte_p = (struct pte*) ret;
  pte_p->RW = 1;
  sym_lower();
}

void sym_make_pg_unwritable(uint64_t addr){
  unsigned int level;
  sym_elevate();
  // Get PTE 
  struct pte* pte_p = sym_get_pte(addr, &level);
  pte_p->RW = 0;
  sym_lower();
}
/* void write_ktext_addr(uint64_t addr, char *s, int len){ */
/*   // TODO if write spans pages, this will fail. */
/*   sym_make_pte_writable(addr); */
/*   sym_elevate(); */
/*   memcpy((void *)addr, s, len); */
/* } */

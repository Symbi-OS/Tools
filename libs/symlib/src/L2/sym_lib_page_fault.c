#include "L2/sym_lib_page_fault.h"
#include "L1/sym_interrupts.h"
#include "L0/sym_lib.h"
#include "LIDK/idk.h"


// XXX global val
// This is the old handler we jmp to after our interposer.
uint64_t orig_asm_exc_page_fault;

uint64_t orig_asm_exc_double_fault;

// This is the old handler we jmp to after our interposer.
/* uint64_t orig_asm_exc_double_fault; */
// NOTE: In df handler, make sure not to jump to new interposed version.
uint64_t my_asm_exc_page_fault;

// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
extern uint64_t bs_asm_exc_page_fault;

// HACK
extern uint64_t cr3_reg;

// HACK
static void (*myprintk)(char *);
static void (*myprintki)(char *, uint64_t);

__attribute__((aligned (16)))
static struct excep_frame *ef = NULL;
__attribute__((aligned (16)))

static void print_ef() __attribute__((unused));
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
  GET_CR3(my_cr3)

  if(!cr3_reg){
    /* myprintk("Error, cr3_reg never set\n"); */
    while(1);
  }

  if(cr3_reg  != my_cr3){
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

#define DEFINE_TF_INTERPOSER \
__asm__("\
                  /*prologue*/                                       \
  .text                                                          \n\t\
  .align 16                                                      \n\t\
  .globl \t tf_interposer_asm                                    \n\t\
  tf_interposer_asm:                                             \n\t\
");

// NOTE Define fn in assembly
DEFINE_TF_INTERPOSER

// NOTE Want to pass ef pointer to interposer C code
__asm__(" \
  pushq %rdi                 /*Preserve rdi */ \n\t\
  movq %rsp, %rdi            /*Get rsp for 1st arg to c fn */ \n\t\
  add $8, %rdi               /* Push set us back 8 */ \n\t");

// NOTE Save all regs.
PUSH_REGS

// NOTE Call into C code
__asm__("  call sym_tf_set_user_bit");

// NOTE Restore regs
POP_REGS

__asm__("                     \
  popq %rdi                   /*Done with 1st arg, restore user rdi  */ \n\t\
\
  push   %rax                      \n\t\
  mov    $0xffffffff81e00ac0,%rax  /* asm_exc_page_fault */ \n\t   \
  xor    (%rsp),%rax               /*Arlo's trick*/ \n\t\
  xor    %rax,(%rsp)               \n\t\
  xor    (%rsp),%rax               \n\t\
  ret \
");

#ifdef CTRS
int wr_ctr = 0;
int rd_ctr = 0;
int ins_ctr = 0;

int user_mode_ctr = 0;
int kern_mode_ctr = 0;

int user_area_ctr = 0;
int interpose_ctr = 0;
#endif

// XXX this fn must be on the same page as tf_interposer_asm
void sym_tf_set_user_bit(struct excep_frame * s){

#ifdef CTRS
  // Ins fetch
  if( s->err & INS_FETCH){
    ins_ctr++;
  }
  if( s->err & WR_FT){
    wr_ctr++;
  } else{
    rd_ctr++;
  }
    // We don't need to special case when in ring 3.
  if(! (s->err & USER_FT) )  {
    user_mode_ctr++;
  }else{
    kern_mode_ctr++;
  }

  // User side of addr space
  if( s->rip < ( (1UL << 47) - PG_SZ) ){
    user_area_ctr++;
  }else{
    kern_area_ctr++;
  }
  interpose_ctr++;
#endif
#if 0
  /* Are we a read fault? */
  if( s->err & WR_FT){
    // NYI write fault
    // TODO: implement me

  } else{
    // read fault

    // kern mode
    if(! (s->err & USER_FT) )  {

      // User half
      if( s->rip < ( (1UL << 47) - PG_SZ) ){
        // Lie about it being user mode.
        s->err |= USER_FT;
      }
    }
  }
#endif


  /* Are we an instruction fetch? */
  if( s->err & INS_FETCH){
    // We don't need to special case when in ring 3.
    if(! (s->err & USER_FT) )  {
        // Are we user code?
        // Could look in cr2, but by def rip caused the fault here.
        // This is modeled after kern:fault_in_kernel_space
        if( s->rip < ( (1UL << 47) - PG_SZ) ){
          /// Lie that code was running in user mode.
          s->err |= USER_FT;
          /* myprintk("swinging err code for\n"); */
          /* print_ef(); */
          /* myprintki("my_ctr %d\n", my_ctr++); */
        }
    }
  }

  /* printf("ss %lx\n", ((struct ef *)s)->ss); */
  /* printf("sp %lx\n", ((struct ef *)s)->sp); */
  /* printf("rf %lx\n", ((struct ef *)s)->rf); */
  /* printf("cs %lx\n", ((struct ef *)s)->cs); */
  /* printf("ip %lx\n", ((struct ef *)s)->ip); */
  /* printf("ec %lx\n", ((struct ef *)s)->ec); */
}


void sym_lib_page_fault_init(){
  printf("Init SLPF\n");

  my_asm_exc_page_fault = (uint64_t) sym_get_fn_address("asm_exc_page_fault");

  // Hacks
  myprintk = sym_get_fn_address("printk");
  myprintki = sym_get_fn_address("printk");

}

// Preserve caller saved GPRs.
// Want RSP to be 16byte aligned after call.
static uint64_t __attribute__((unused))my_entry = (uint64_t) &pg_ft_c_entry;
/* extern uint64_t c_handler_page_fault; */

// NOTE: semicolons for editor only.
FINAL_HANDLER(c_handler_page_fault, *my_entry, *orig_asm_exc_page_fault);


FINAL_HANDLER(c_df_handler, *my_df_entry, *my_asm_exc_page_fault);
__attribute__((aligned (16)))
static void df_c_entry_dep(){
  // Error code on DF is 0,
  ef->err = USER_FT | WR_FT;
}

// NOTE: This function is not used in C code, but is used in inline assembly.
// This asks the compiler not to warn about it being unused.
static uint64_t __attribute__((unused)) my_df_entry = (uint64_t) &df_c_entry_dep;
// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.

// NOTE: This function is not used in C code, but is used in inline assembly.
// This asks the compiler not to warn about it being unused.
extern uint64_t __attribute__((unused)) c_df_handler;
// Want this to fix double faults.
// Why is asm_exc_page_fault going to work this time?
// DF runs on known good IST stack unlike pg ft.
// Likely at a performance penalty, should be rare path.
// See kernel mode linux "Stack Starvation".



INTERPOSER_HANDLER(df_jmp_to_c, df_c_entry);
__attribute__((aligned (16)))
static __attribute((unused)) void df_c_entry(struct pt_regs *pt_r){
  // Error code on DF is 0
  pt_r->error_code = USER_FT | WR_FT;
}

INTERPOSER_HANDLER(tf_jmp_to_c, tf_c_entry);
static __attribute((unused)) void tf_c_entry(struct pt_regs *pt_r){
  // Stack or bust.
  /* char ins[] = "ins fetch\n"; */
  /* char wr[] = " wr\n"; */
  /* char str[] = "Stack\n"; */

  /* void (*tu_printk)(char *) = (void *)0xffffffff81c87049 ; */
  /* tu_printk(str); */


  if(pt_r->error_code & WR_FT){
    return;
  }

  pt_r->error_code |= USER_FT;

  /* /\* Are we an instruction fetch? *\/ */
  /* if(pt_r->error_code & INS_FETCH){ */
  /*   // We don't need to special case when in ring 3. */
  /*   if(! (pt_r->error_code & USER_FT)  ){ */
  /*     // Are we user code? */
  /*     // Could look in cr2, but by def rip caused the fault here. */
  /*     // This is modeled after kern:fault_in_kernel_space */
  /*     if(pt_r->rip < ( (1UL << 47) - 4096) ){ */
  /*       /// Lie that code was running in user mode. */
  /*       pt_r->error_code |= USER_FT; */
  /*       /\* tu_printk(ins); *\/ */
  /*       /\* print_ef(); *\/ */
  /*       /\* myprintki("my_ctr %d\n", my_ctr++); *\/ */
  /*     } */
  /*   } */
  /* } */
}

// 6 = user + write
// Rest is to call into 8 byte address without clobbering any registers.
// Push random reg to stack, put addr in that reg & swap w/o dirtying a reg.
// Tell me if there's an easier way!
  __asm__("\
  .text \n\t\
  .align 16 \n\t\
  .globl \t df_asm_handler \n\t\
  df_asm_handler: \n\t\
  movq   $0x6,(%rsp) \n\t\
  push   %rax \n\t\
  mov    $0xffffffff81e00ac0,%rax \n\t\
  xor    (%rsp),%rax /*Arlo's trick*/ \n\t\
  xor    %rax,(%rsp) \n\t\
  xor    (%rsp),%rax \n\t\
  ret \
");

// Text fault handler
// Save rsi
// move ptr to ef into rsi
// set user bit of error code to lie to kernel
// Store error code back in ef
// slide into normal pf handler
__asm__("\
  .text \n\t\
  .align 16 \n\t\
  .globl \t tf_asm_handler \n\t\
  tf_asm_handler: \n\t\
  pushq %rsi                    \n\t\
  movq 8(%rsp),%rsi             \n\t\
  orq $0x4, %rsi                \n\t\
  movq %rsi, 8(%rsp)            \n\t\
  popq %rsi                     \n\t\
  mov    $0xffffffff81e00ac0,%rax \n\t\
  xor    (%rsp),%rax /*Arlo's trick*/ \n\t\
  xor    %rax,(%rsp) \n\t\
  xor    (%rsp),%rax \n\t\
  ret \
");

void sym_interpose_on_df_c(unsigned char * my_idt){
  // Get ptr to df desc
  union idt_desc *desc_old;
  desc_old = sym_get_idt_desc(my_idt, DF_IDX);

  // save old asm_exc_pf ptr
  union idt_addr old_asm_exc_df;
  sym_load_addr_from_desc(desc_old, &old_asm_exc_df);


  // get systm's df handler address
  orig_asm_exc_double_fault = old_asm_exc_df.raw;

  // New handler
  union idt_addr new_asm_exc_addr;
  new_asm_exc_addr.raw = (uint64_t) &c_df_handler;

  // Set IDT to point to our new interposer
  sym_load_desc_from_addr(desc_old, &new_asm_exc_addr);
}

// TODO: clean up this code duplication
// TODO also arg types
void sym_interpose_on_df_asm(unsigned char * my_idt, unsigned char *handler_pg){
  // Get ptr to df desc
  union idt_desc *desc_old;
  desc_old = sym_get_idt_desc(my_idt, DF_IDX);

  // save old asm_exc_pf ptr
  union idt_addr old_asm_exc_df;
  sym_load_addr_from_desc(desc_old, &old_asm_exc_df);


  // get systm's df handler address
  orig_asm_exc_double_fault = old_asm_exc_df.raw;

  // New handler
  union idt_addr new_asm_exc_addr;
  new_asm_exc_addr.raw = (uint64_t) handler_pg;

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


void sym_toggle_pg_ft_ist(unsigned char *my_idt, unsigned int enable){
  union idt_desc *desc_old;
  union idt_desc desc_new;

  assert( (enable == 0) || (enable == 1) );

  // Get ptr to pf desc
  desc_old = sym_get_idt_desc(my_idt, PG_FT_IDX);

  // Copy descriptor to local var
  sym_elevate(); desc_new = *desc_old; sym_lower();

  // Force IST usage
  desc_new.fields.ist = enable;

  // Write into user table
  sym_elevate();
  sym_set_idt_desc(my_idt, PG_FT_IDX, &desc_new);
  sym_lower();
}

// TODO: Depricate this usage
void sym_make_pg_ft_use_ist(unsigned char *my_idt){
  sym_toggle_pg_ft_ist(my_idt, 1);
}

// This is just a type.
typedef void* (*lookup_address_t)(uint64_t address, unsigned int * level);

struct pte *
sym_get_pte(uint64_t addr, unsigned int *level)
{
  // XXX think this only works for kern addresses???
  // Cache this if slow
  lookup_address_t my_lookup_address = sym_get_fn_address("lookup_address");
  sym_elevate();
  struct pte * ret = my_lookup_address(addr, level);
  sym_lower();
  return ret;
}
void sym_print_pte(struct pte *pte){
  sym_elevate(); uint64_t raw_pte = *(uint64_t *)pte; sym_lower();
  printf("PTE at %p contains %lx\n", pte, raw_pte);
};

void sym_make_pg_writable(uint64_t addr){
  sym_elevate();
  // Get PTE
  lookup_address_t my_lookup_address = sym_get_fn_address("lookup_address");
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

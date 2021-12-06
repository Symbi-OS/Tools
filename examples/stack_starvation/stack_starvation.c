#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>

#include "../../include/headers/sym_all.h"

// Our version of the idt. Not sure about alignment.
unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

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

// This is the old handler we jmp to after our interposer.
uint64_t orig_asm_exc_double_fault = 0xffffffff81c3e2b0;
uint64_t asm_exc_page_fault = 0xffffffff81e00ac0;

// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
extern uint64_t bs_asm_exc_double_fault;

// Store system idtr here for later restoration.
struct dtr system_idtr;

// Want this to fix double faults.
// Shot in the dark, what UKL does.
// TODO: probably set U bit (bit 3) of error code saying it is user.
// 0x2 say suser write access in kernel mode. 0x6 says #PF: user write access in kernel mode
asm("\
 .text                         \n\t\
 .align 16                     \n\t\
 bs_asm_exc_double_fault:      \n\t\
 movq    $0x6, (%rsp)         \n\t\
 jmp     *asm_exc_page_fault       \
");


void make_pg_ft_use_ist(){
  // Copy the system idt to userspace
  sym_copy_system_idt(my_idt);

  union idt_desc *desc_old;
  union idt_desc desc_new;

  int PG_FT_IDX = 14;
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

  // Swing idtr to new modified table
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

void show_process_stk_ft_works(){
  sym_touch_stack();
}

void show_naive_elevation_DFs(){
  printf("about to elevate and touch stack\n");

  sym_elevate();

  sym_touch_stack();

  sym_lower();
}

void show_prefault_solves_DF(){
  sym_touch_stack();

  sym_elevate();

  sym_touch_stack();

  sym_lower();
}

void show_using_ist_solves_DF(){
  make_pg_ft_use_ist();
  sym_elevate();
  sym_touch_stack();
  sym_lower();
}


void idt_interpose(){
  printf("interpose nyi\n");

  int DF_IDX= 8;

  // We've already copied and modified the idt, now we change it further.
  /* sym_copy_system_idt(my_idt); */

  sym_print_idt_desc(my_idt, DF_IDX);

  // Get ptr to pf desc
  union idt_desc *desc_old;
  desc_old = sym_get_idt_desc(my_idt, DF_IDX);

  // save old asm_exc_pf ptr
  union idt_addr old_asm_exc_df;
  sym_load_addr_from_desc(desc_old, &old_asm_exc_df);

  // swing addr to  bs_asm...
  orig_asm_exc_double_fault = old_asm_exc_df.raw;

  // New handler
  union idt_addr new_asm_exc_addr;
  new_asm_exc_addr.raw = (uint64_t) &bs_asm_exc_double_fault;

  // Set IDT to point to our new interposer
  sym_load_desc_from_addr(desc_old, &new_asm_exc_addr);

  sym_print_idt_desc(my_idt, DF_IDX);
  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

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

void show_using_idt_interpose_solves_DF(){
  // make sure we don't fault on text faults
  sym_elevate();
  interpose_on_pg_ft();

  sym_elevate();
  idt_interpose();
  sym_lower();

  sym_elevate();

  sym_touch_stack();
  sym_lower();
}

/* #define NORMAL_PROCESS 1 */
/* #define NAIVE_ELEVATION 1 */
// #define PREFAULT_ELEVATION 1
/* #define IST_ELEVATION 1 */

#define IDT_INTERPOSE 1

int main(){
  printf("Starting main\n");

  // Store initial system IDTR
  sym_store_idt_desc(&system_idtr);

#ifdef NORMAL_PROCESS
  printf("NORMAL_PROCESS\n");
  show_process_stk_ft_works();
#endif

#ifdef NAIVE_ELEVATION
  printf("NAIVE_ELEVATION\n");
  show_naive_elevation_DFs();
#endif

#ifdef  PREFAULT_ELEVATION
  printf("PREFAULT_ELEVATION\n");
  show_prefault_solves_DF();
#endif

#ifdef IST_ELEVATION
  printf("IST_ELEVATION\n");
  show_using_ist_solves_DF();
#endif

#ifdef IDT_INTERPOSE
  printf("IDT_INTERPOSE\n");
  show_using_idt_interpose_solves_DF();
#endif

  printf("Done main\n");

  // Restore system IDTR
  sym_load_idtr(&system_idtr);
}


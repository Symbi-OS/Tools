#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>

#include "../../include/headers/sym_all.h"

// Our version of the idt. Not sure about alignment.
unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

// This is the old handler we jmp to after our interposer.
uint64_t orig_asm_exc_double_fault; //= 0xffffffff81c3e2b0;
uint64_t my_asm_exc_page_fault = 0xffffffff81e00ac0;

// Store system idtr here for later restoration.
struct dtr system_idtr;



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

int my_ctr = 0;
static void df_c_entry(){
  // Error code on DF is 0, 
  ef->err = USER_FT | WR_FT;
}

static uint64_t my_entry = (uint64_t) &df_c_entry;
// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
extern uint64_t c_df_handler;

// Want this to fix double faults.
asm("\
 .text                         \n\t\
 .align 16                     \n\t\
 c_df_handler:      \n\t\
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
 jmp     *my_asm_exc_page_fault       \
");

void make_pg_ft_use_ist(){
  // Copy the system idt to userspace
  sym_copy_system_idt(my_idt);

  union idt_desc *desc_old;
  union idt_desc desc_new;

  /* int PG_FT_IDX = 14; */
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

  // We've already copied and modified the idt, now we change it further.
  sym_copy_system_idt(my_idt);
  sym_elevate();
  sym_print_idt_desc(my_idt, DF_IDX);


  // Get ptr to df desc
  union idt_desc *desc_old;
  desc_old = sym_get_idt_desc(my_idt, DF_IDX);


  // save old asm_exc_pf ptr
  union idt_addr old_asm_exc_df;
  sym_load_addr_from_desc(desc_old, &old_asm_exc_df);
  printf("old_asm_exc_df.raw %llx\n", old_asm_exc_df.raw );
  // Next line breaks

  // swing addr to  bs_asm...
  orig_asm_exc_double_fault = old_asm_exc_df.raw;

  // New handler
  union idt_addr new_asm_exc_addr;
  new_asm_exc_addr.raw = (uint64_t) &c_df_handler;

  // Set IDT to point to our new interposer
  sym_load_desc_from_addr(desc_old, &new_asm_exc_addr);

  sym_print_idt_desc(my_idt, DF_IDX);
  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}
extern uint64_t cr3_reg;
void interpose_on_pg_ft(){

  sym_elevate();
  asm("movq %%cr3,%0" : "=r"(cr3_reg));
  sym_lower();

  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt);{
    printf("copying the idt for pf\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(my_idt);
  }

  sym_interpose_on_pg_ft_c(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

void show_using_idt_interpose_solves_DF(){
  // make sure we don't fault on text faults
  /* interpose_on_pg_ft(); */
  sym_touch_every_page_text();

  sym_elevate();
  idt_interpose();
  sym_lower();

  sym_elevate();
  sym_touch_stack();
  sym_lower();
}

/* #define NORMAL_PROCESS 1 */
/* #define NAIVE_ELEVATION 1 */
/* #define PREFAULT_ELEVATION 1 */
/* #define IST_ELEVATION 1 */

#define IDT_INTERPOSE 1

int main(){
  printf("Starting main\n");

#ifndef NORMAL_PROCESS
  // Store initial system IDTR
  sym_store_idt_desc(&system_idtr);
#endif

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
#ifndef NORMAL_PROCESS
  sym_load_idtr(&system_idtr);
#endif
}


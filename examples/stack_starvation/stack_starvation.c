#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>

#include "LINF/sym_all.h"

// Our version of the idt. Not sure about alignment.
unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

// Store system idtr here for later restoration.
struct dtr system_idtr;


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
  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for ist\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(my_idt);
  }

  sym_make_pg_ft_use_ist(my_idt);

  // If already swung, don't do anything.
  sym_store_idt_desc(&check_idtr);
  if(check_idtr.base != (uint64_t) &my_idt){
    // Swing idtr to new modified table
    sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
  }

  sym_elevate();
  sym_touch_stack();
  sym_lower();
}

void interpose_on_df(){
  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for DOUBLE F\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(my_idt);
  }

  sym_interpose_on_df_c(my_idt);

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

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for PAGE F\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(my_idt);
  }

  sym_interpose_on_pg_ft_c(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

void show_using_idt_interpose_solves_DF(){
  // make sure we don't fault on text faults

  interpose_on_pg_ft();
  interpose_on_df();

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
  sym_lib_init();

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


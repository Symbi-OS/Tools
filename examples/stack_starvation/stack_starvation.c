#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>

#include "../../include/sym_lib_syscall.h"
#include "../../include/sym_structs.h"
#include "../../include/sym_interrupts.h"


// TODO Rename to generalize

unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

void stack_test(){
  int i = 0;
  // Let's do a bunch of pushes and see if we can trigger the stack bug.
  intptr_t sp;
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Rsp is %lx\n", sp);
  for(; i < (1<<12); i++){
    asm("pushq $42");
  }
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Rsp is %lx\n", sp);

}

void touch_stack(){
  intptr_t sp;
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Touching stack Rsp is %lx\n", sp);

  int i = 0;
  // Push a bunch of values
  for(; i < (1<<12); i++){
    asm("pushq $42");
  }

  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Touching stack Rsp is %lx\n", sp);
  i = 0;

  // Pop them all off
  for(; i < (1<<12); i++){
    asm("popq %rax");
  }
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Touching stack Rsp is %lx\n", sp);
}

void make_pg_ft_use_ist(){
  /* printf("user idt at %p\n", my_idt); */

  // Copy the system idt to userspace
  sym_copy_system_idt(my_idt);

  // Take a look at the page fault handler descriptor
  /* sym_print_idt_desc(my_idt, 14); */

  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);

  union idt_desc *desc_old;
  union idt_desc desc_new;

  desc_old = sym_get_idt_desc(my_idt, 14);


  desc_new = *desc_old;
  desc_new.fields.ist = 3;

  sym_set_idt_desc(my_idt, 14, &desc_new);

  /* sym_print_idt_desc(my_idt, 14); */
}

void show_process_stk_ft_works(){
  sym_touch_stack();
}

void show_naive_elevation_DFs(){
  // Elevate
  sym_elevate();

  sym_touch_stack();

  sym_lower();
}

void show_prefault_solves(){
  sym_touch_stack();

  // Elevate
  sym_elevate();

  sym_touch_stack();

  sym_lower();
}

void show_using_ist_solves(){
  // Elevate
  sym_elevate();

  make_pg_ft_use_ist();

  sym_lower();

  sym_touch_stack();
}

/* #define NORMAL_PROCESS 1 */
/* #define NAIVE_ELEVATION 1 */
/* #define PREFAULT_ELEVATION 1 */
#define IST_ELEVATION 1


int main(){
  // This prevents double faults when kern uses def stack for pg ft.
  /* sym_touch_stack(); */
  printf("Starting main\n");

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
  show_prefault_solves();
#endif

#ifdef IST_ELEVATION
  printf("IST_ELEVATION\n");
  show_using_ist_solves();
#endif

  printf("Done main\n");

  while(1);
  return 0;

}

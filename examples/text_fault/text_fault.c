#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>

#include "../../include/headers/sym_all.h"

// Our version of the idt. Not sure about alignment.
unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

void interpose_on_pg_ft(){

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
/* void interpose_on_pg_ft(){ */
/*   int PG_FT_IDX= 14; */
/*   // Copy the system idt to userspace */
/*   sym_copy_system_idt(my_idt); */

/*   // Modify it to inject a shim that flips a  */
/*   sym_interpose_on_pg_ft(my_idt); */

/*   // Make our user IDT live! */
/*   sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1); */
/* } */

extern void foo(void);
/*
  1) Process works
  2) Naive elevation fails
  3) Prefault works
  4) Interposition works.
*/

/* #define NORMAL_PROCESS 1 */
/* #define NAIVE_ELEVATION 1 */
/* #define PREFAULT_ELEVATION 1 */
#define INT_INTERPOSITION 1
/* #define NOP_SLIDE 1 */

void show_process_text_ft_works(){
  foo();
}

void show_naive_elevation_breaks(){
  sym_elevate();
  foo();
  sym_lower();
}

void show_int_interposition_works(){
  sym_elevate();
  interpose_on_pg_ft();
  sym_elevate();
  foo();
  sym_lower();
}

void make_nop_slide(){
  printf("write nops over kernel\n");
  sym_elevate();
  memset((void *)0xffffffff810588da, 0x90, 48);
  sym_lower();
}

void show_nop_slide_works(){
  // Access the first byte of page we'll fault on.
  make_nop_slide();
  printf("char at addr is %x\n", *((unsigned char*) 0xffffffff810588da) );
  while(1);
  asm("WBINVD");
  sym_elevate();
  foo();
  sym_lower();
}

void show_prefault_works(){
  // Access the first byte of page we'll fault on.
  char *c;
  c = (char *) 0x404000; // Addr first text fault
  printf("byte at %p is %c\n", c, *c);
  sym_elevate();
  foo();
  sym_lower();
}

extern uint64_t cr3_reg;
int main(){
  printf("Starting main\n");

  // Store system idtr here for later restoration.
  struct dtr system_idtr;
 
  // Store initial system IDTR
  sym_store_idt_desc(&system_idtr);

  sym_elevate();
  asm("movq %%cr3,%0" : "=r"(cr3_reg));
  sym_lower();


#ifdef NORMAL_PROCESS
  printf("NORMAL_PROCESS\n");
  show_process_text_ft_works();
#endif

#ifdef NAIVE_ELEVATION
  printf("NAIVE_ELEVATION\n");
  show_naive_elevation_breaks();
#endif

#ifdef  PREFAULT_ELEVATION
  printf("PREFAULT_ELEVATION\n");
  show_prefault_works();
#endif

#ifdef NOP_SLIDE
  printf("NOP_SLIDE\n");
  printf("currently doesn't work\n");
  show_nop_slide_works();
#endif

#ifdef INT_INTERPOSITION
  printf("IST_ELEVATION\n");
  show_int_interposition_works();
#endif

  printf("Done main\n");

  // Swing back onto system idtr before exit
  sym_load_idtr(&system_idtr);
}



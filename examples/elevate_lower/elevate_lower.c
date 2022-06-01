#include <stdio.h> // for kallsymlib... bug.
#include <assert.h>
#include <stdlib.h>
#include "LINF/sym_all.h"

void push_a_lot(){
  // HACK don't know why seg faults without register usage.
  register int count = 1 << 13;
  for(register int i=0; i< count; i++){
    __asm__("pushq $42":::"memory");
  }
  /* printf("pushing a lot \n"); */
  for(register int i=0; i< count; i++){
    __asm__("popq %%rax": /*no*/: /*no*/ : "rax");
  }

}
/* #define USE_MODE_SHIFT */

int main(__attribute((unused))int argc, char *argv[]){
  // Assumes some mitigation is used.
#ifdef USE_MODE_SHIFT
  uint64_t kern_gs;
#endif
  /* uint64_t zero_gs = 0; */
  int count = atoi(argv[1]);
  assert(count >= 1);

  /* push_a_lot(); */
#ifdef USE_MODE_SHIFT
  sym_mode_shift(SYM_ELEVATE_FLAG | SYM_INT_DISABLE_FLAG | SYM_INT_DISABLE_FLAG );
  // NOTE: interrupts disabled
  asm("swapgs"); // get onto kern
  asm("rdgsbase %0" : "=r"(kern_gs));
  asm("swapgs"); // get onto user
  asm("wrgsbase %0" : : "r"(kern_gs));
  sym_mode_shift(SYM_LOWER_FLAG);
#else
  sym_elevate();
  sym_lower();
#endif

  for(int i = 0; i<count; i++ ){
#ifdef USE_MODE_SHIFT
    /* sym_mode_shift(SYM_ELEVATE_FLAG | SYM_INT_DISABLE_FLAG | SYM_DEBUG_FLAG); */
    sym_mode_shift(SYM_ELEVATE_FLAG );
#else
    sym_elevate();
#endif
    /* asm("nop"); */
    /* asm("movq %%cr3,%0" */
    /*     : "=r"(cr3_reg) */
    /*     ); */
    /* asm("nop"); */
#ifdef USE_MODE_SHIFT
    /* sym_mode_shift(SYM_LOWER_FLAG | SYM_DEBUG_FLAG); */
    sym_mode_shift(SYM_LOWER_FLAG);
#else
    sym_lower();
#endif

  }
  /* printf("prove elevation worked by printing cr3 %lx\n", cr3_reg); */
}

#include "./headers/sym_lib_hacks.h"

extern char __executable_start;
extern char __etext;

void sym_touch_every_page_text(){
  unsigned char *p;
  p = (unsigned char *) &__executable_start;

  unsigned char c;
  for(; p < (unsigned char *)&__etext; p+= (1<<12))
    c = *p;
}

void sym_touch_stack(){
  // Num times to push
  int count = 1<<16;

  int i;
  // Touch a bunch of stack pages
  for(i=0; i < count; i++){
    __asm__("pushq $42":::"memory");
  }

  // Pop them all off
  for(i=0; i < count; i++){
    __asm__("popq %%rax": /*no*/: /*no*/ : "rax");
  }
}

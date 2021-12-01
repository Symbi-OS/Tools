#include <stdio.h>
#include "elevate.h"

void sym_elevate(){
  register int    rax  asm("rax") = 448; // elevate
  register int    rdi        asm("rdi") = 1; // direction
  __asm__ __volatile__ (
                        "syscall"
                        : "+r" (rax)
                        : "r" (rdi)
                        : "rcx", "r11", "memory"
                        );

}

void sym_lower(){
  // This write syscall is understood by the modified linux to request lowering
  register int    rax  asm("rax") = 448; // elevate
  register int    rdi        asm("rdi") = -1; // direction
  __asm__ __volatile__ (
                        "syscall"
                        : "+r" (rax)
                        : "r" (rdi)
                        : "rcx", "r11", "memory"
                        );
}

void touch_stack(){
  // Num times to push
  int count = 1<<16;

  int i;
  // Touch a bunch of stack pages
  for(i=0; i < count; i++){
    __asm__("pushq $42":::"memory");
  }

  // Pop them all off
  for(i=0; i < count; i++){
    __asm__("popq %%rax":::"rax");
  }
  fprintf(stderr, "Push %d times, %d bytes, %d pages\n", count, count*7, (count*8) / (1<<12) );
}

void __attribute__ ((constructor)) initLibrary(void) {
  //
  // Function that is called when the library is loaded
  //

  // Let's touch stack pages before elevating to try to avoid starvation.
  touch_stack();
  sym_elevate();
}

void __attribute__ ((destructor)) cleanUpLibrary(void) {
  //
  // Function that is called when the library is »closed«.
  //
  sym_lower();
}

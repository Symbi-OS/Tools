#include <stdio.h>
#include "elevate.h"

void sym_elevate(){
  // This write syscall is understood by the modified linux to request elevation

  register int    rax  asm("rax") = 1; // write
  register int    rdi        asm("rdi") = 1; // file des
  register const void*  rsi        asm("rsi") = "elevate";
  register size_t   rdx        asm("rdx") = 7;
  __asm__ __volatile__ (
                        "syscall"
                        : "+r" (rax)
                        : "r" (rdi), "r" (rsi), "r" (rdx)
                        : "rcx", "r11", "memory"
                        );


}

void sym_lower(){
  // This write syscall is understood by the modified linux to request lowering

  register int    rax  asm("rax") = 1; // write
  register int    rdi        asm("rdi") = 1; // file des
  register const void*  rsi        asm("rsi") = "lower";
  register int    rdx        asm("rdx") = 5;
  __asm__ __volatile__ (
                        "syscall"
                        : "+r" (rax)
                        : "r" (rdi), "r" (rsi), "r" (rdx)
                        : "rcx", "r11", "memory"
                        );
}

void touch_stack(){
  /* register int    syscall_no  asm("rax") = 1; // write */
  /* register int    arg1        asm("rdi") = 1; // file des */
  /* register char*  arg2        asm("rsi") = "Touch Stack"; */
  /* register int    arg3        asm("rdx") = 11; */
  /* asm("syscall"); */

  // Num times to push
  int count = 1<<16;


  int i;
  // Touch a bunch of stack pages
  for(i=0; i < count; i++){
    asm("pushq $42");
  }

  // Pop them all off
  for(i=0; i < count; i++){
    asm("popq %rax");
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

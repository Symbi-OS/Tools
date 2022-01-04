#include <stdio.h>

// void sym_elevate();
// void sym_lower();

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

  printf("Push %d times, %d bytes, %d pages\n", count, count*8, (count*8) / (1<<12) );
}

void sym_elevate(){
  touch_stack();

  register int    syscall_no  asm("rax") = 1; // write
  register int    arg1        asm("rdi") = 1; // file des
  register char*  arg2        asm("rsi") = "elevate";
  register int    arg3        asm("rdx") = 7;
  asm("syscall");
}
void sym_lower(){
  register int    syscall_no  asm("rax") = 1; // write
  register int    arg1        asm("rdi") = 1; // file des
  register char*  arg2        asm("rsi") = "lower";
  register int    arg3        asm("rdx") = 5;
  asm("syscall");
}

#include <stdio.h>
#include "elevate.h"

void hi() {
  printf("hi\n");
}

void sym_elevate(){
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

void __attribute__ ((constructor)) initLibrary(void) {
  //
  // Function that is called when the library is loaded
  //
  sym_elevate();
}
void __attribute__ ((destructor)) cleanUpLibrary(void) {
  //
  // Function that is called when the library is »closed«.
  //
  sym_lower();
}

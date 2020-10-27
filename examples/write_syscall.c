
#include <stdio.h>
#include "include/sym_lib.h"

int main(){
  sym_elevate();

  /* Can even call a syscall from ring 0 */
  register int    syscall_no  asm("rax") = 1; // write
  register int    arg1        asm("rdi") = 1; // file des
  register char*  arg2        asm("rsi") = "Tommy";
  register int    arg3        asm("rdx") = 5;
  asm("syscall");

  sym_lower();
  return 0;
}

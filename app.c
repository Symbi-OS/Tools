#include<stdio.h>
int main(){
  printf("START\n");

  register int    syscall_no  asm("rax") = 1;
  register int    arg1        asm("rdi") = 1;
  register char*  arg2        asm("rsi") = "hello, world!\n";
  register int    arg3        asm("rdx") = 14;
  asm("syscall");

  syscall_no = 1;
  arg1 = 1;
  arg2 = "elevate";
  arg3 = 7;
  asm("syscall");

  syscall_no = 1;
  arg1 = 1;
  arg2 = "lower";
  arg3 = 5;
  asm("syscall");

  printf("DONE\n");
  return 0;
}

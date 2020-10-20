#include<stdio.h>
void secret_fn(){
  /* asm("mov    %cr3, %rax"); */
	while(1);

  void (*printk)(char*) = ( void(*)(char*) ) 0xffffffff810c1659;

  char *my_str = "hi tommy\n";
  (*printk)(my_str);

	while(1);
}

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
  while(1);

  syscall_no = 1;
  arg1 = 1;
  arg2 = "Tommy";
  arg3 = 5;
  asm("syscall");

  while(1);

  syscall_no = 1;
  arg1 = 1;
  arg2 = "lower";
  arg3 = 5;
  asm("syscall");

  printf("DONE\n");
  return 0;
}

#include <stdint.h>
#include <stdio.h>
#include "../../include/sym_lib_syscall.h"

extern char __executable_start;
extern char __etext;

void touch_every_page_text(){
  unsigned char *p;
  p = (unsigned char *) &__executable_start;

  unsigned char c;
  for(; p < (unsigned char *)&__etext; p+= (1<<12))
    c = *p;
}

int main(){
  touch_every_page_text();
  sym_touch_stack();

  printf("Can we read cr3?\n");

  sym_elevate();
  uint64_t cr3_reg;
  asm("movq %%cr3,%0" : "=r"(cr3_reg));
  printf("Cr3 holds %p\n", cr3_reg);

  register int    syscall_no  asm("rax") = 1; // write
  register int    arg1        asm("rdi") = 1; // file des
  register char*  arg2        asm("rsi") = "Tommy\n";
  register int    arg3        asm("rdx") = 6;

#if 0
  asm("syscall");
#else

  /* asm("jmp 0xffffffff81c00010"); */
  /* my_entry_SYSCALL_64(); */
  int (*my_ksys_write)(unsigned int fd, const char *buf, size_t count) =
    ( int(*)(unsigned int fd, const char *buf, size_t count) ) 0xffffffff81206ff0;
  my_ksys_write(1, "Tommy\n", 6);
#endif
  sym_lower();
  printf("Done\n");
  return 0;
}

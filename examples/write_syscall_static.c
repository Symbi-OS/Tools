#include <stdint.h>
#include <stdio.h>

#include "../include/sym_lib.h"

int main(){

  sym_elevate();

  /* printf("Can we read cr3?\n"); */
  /* uint64_t cr3_reg; */
  /* asm("movq %%cr3,%0" */
  /*     : "=r"(cr3_reg) */
  /*     ); */
  /* printf("Cr3 holds %p\n", cr3_reg); */

#if 0
  while(1){
    /* Can even call a syscall from ring 0 */
    register int    syscall_no  asm("rax") = 1; // write
    register int    arg1        asm("rdi") = 1; // file des
    register char*  arg2        asm("rsi") = "Tommy\n";
    register int    arg3        asm("rdx") = 6;
    asm("syscall");
  }

#elif 0
  /* void (*my_entry_SYSCALL_64)() = */
  /*   ( void(*)() ) 0xffffffff81c00010; */
  /* my_entry_SYSCALL_64(); */
  while(1){
  register int    syscall_no  asm("rax") = 1; // write
  register int    arg1        asm("rdi") = 1; // file des
  register char*  arg2        asm("rsi") = "Tommy\n";
  register int    arg3        asm("rdx") = 6;

  // Get r11 setup
  asm("pushfq");
  asm("popq %r11");

  // Get rflags setup
  asm("cli");

  // Get RCX setup
  asm("mov $0x401de7, %rcx");

  asm("jmp 0xffffffff81c00010");
  }
#else
  /* int (*my_ksys_write)(unsigned int fd, const char *buf, size_t count) = */
  /*   ( int(*)(unsigned int fd, const char *buf, size_t count) ) 0xffffffff8133e990; */
  /* my_ksys_write(1, "Tommy\n", 6); */

  int (*my___x64_sys_write)(unsigned int fd, const char *buf, size_t count) =
    ( int(*)(unsigned int fd, const char *buf, size_t count) ) 0xffffffff8133ebd0;
  my___x64_sys_write(1, "Tommy\n", 6);

#endif

  sym_lower();

  return 0;
}

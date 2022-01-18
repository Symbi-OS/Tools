#ifndef __ARCH_X86_64_SYM_LIB__
#define __ARCH_X86_64_SYM_LIB__

#include "L0/sym_lib.h"
static long sym_do_syscall(int work){
  register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL;
  register uint64_t    arg1        __asm__("rdi") = work;
  if(!is_sticky){
    __asm__ __volatile__ (
                          "syscall"
                          : "+r" (syscall_no)
                          : "r" (arg1), "r" (syscall_no)
                          : "rcx", "r11", "memory"
                          );
  }
  // HACK
  // Return rax
  return syscall_no;
}

#endif

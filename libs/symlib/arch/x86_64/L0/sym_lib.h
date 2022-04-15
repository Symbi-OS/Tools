#ifndef __ARCH_X86_64_SYM_LIB__
#define __ARCH_X86_64_SYM_LIB__

#include <unistd.h>
#include <sys/syscall.h>

#include "L0/sym_lib.h"
static long sym_do_syscall(int work){

  // XXX The inline assembly implementation below led to subtle
  // breakage. Elevating then lowering in a loop 10^8 times
  // led to ~10 "Already Elevated???" error cases, suggesting
  // perhaps that a lower failed silently?

  /* register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL; */
  /* register uint64_t    arg1        __asm__("rdi") = work; */
  /* if(!is_sticky){ */
  /*   __asm__ __volatile__ ( */
  /*                         "syscall" */
  /*                         : "+r" (syscall_no) */
  /*                         : "r" (arg1), "r" (syscall_no) */
  /*                         : "rcx", "r11", "memory" */
  /*                         ); */
  /* } */
  /* // HACK */
  /* // Return rax */
  /* return syscall_no; */

  if(!is_sticky){
    return syscall(NR_ELEVATE_SYSCALL, work);
  }
  // XXX obviously
  return 42;
}

#endif

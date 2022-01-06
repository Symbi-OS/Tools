#ifndef __ARCH_X86_64_SYM_LIB__
#define __ARCH_X86_64_SYM_LIB__

#include "L0/sym_lib.h"

long sym_elevate(){
  register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL;
  register uint64_t    arg1        __asm__("rdi") = SYSCALL_ELEVATE;
  if(!is_sticky){
    __asm__ __volatile__ (
                          "syscall"
                          : "+r" (syscall_no)
                          : "r" (arg1), "r" (syscall_no)
                          : "rcx", "r11", "memory"
                          );
  }
    return -1;
}



long sym_lower(){
register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL;
  register uint64_t    arg1        __asm__("rdi") = SYSCALL_LOWER;
  if(!is_sticky){
    __asm__ __volatile__ (
                          "syscall"
                          : "+r" (syscall_no)
                          : "r" (arg1), "r" (syscall_no)
                          : "rcx", "r11", "memory"
                          );
  }
  return -1;
}

long sym_check_elevate(){
register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL;
register uint64_t    arg1        __asm__("rdi") = SYSCALL_CHECK_ELEVATE_STATUS;
__asm__ __volatile__ (
                      "syscall"
                      : "+r" (syscall_no)
                      : "r" (arg1), "r" (syscall_no)
                      : "rcx", "r11", "memory"
                      );
return syscall_no;
}

#endif

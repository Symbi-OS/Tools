// License C 2021-
// Author: Thomas Unger
// Level: 0
#define _GNU_SOURCE

#include <stdint.h>
#include "L0/sym_lib.h"

// This gives 1 level of toggle prevention.
__thread int is_sticky = 0;

long sym_elevate(){
  register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL; // write
  register uint64_t    arg1        __asm__("rdi") = SYSCALL_ELEVATE; //

  // After syscall should be in elevated state
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

// After syscall should be in lowered state
long sym_lower(){
  register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL; // write
  register uint64_t    arg1        __asm__("rdi") = SYSCALL_LOWER; //
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
  register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL; // write
  register uint64_t    arg1        __asm__("rdi") = SYSCALL_CHECK_ELEVATE_STATUS; //
  __asm__ __volatile__ (
                        "syscall"
                        : "+r" (syscall_no)
                        : "r" (arg1), "r" (syscall_no) // TODO: are these actually true inputs?
                        : "rcx", "r11", "memory" // TODO: is memory volatile?
                        );
  // XXX: Result should be stored in rcx. IDK if this is safe in general.
  return syscall_no;
}

#define _GNU_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/syscall.h>
#include "headers/sym_lib_syscall.h"

__thread int is_sticky = 0;

void sym_touch_stack(){
  // Num times to push
  int count = 1<<16;

  int i;
  // Touch a bunch of stack pages
  for(i=0; i < count; i++){
    __asm__("pushq $42":::"memory");
  }

  // Pop them all off
  for(i=0; i < count; i++){
    __asm__("popq %%rax": /*no*/: /*no*/ : "rax");
  }
}

long sym_check_elevate(){
    return syscall(NR_ELEVATE_SYSCALL, SYSCALL_CHECK_ELEVATE_STATUS);
}

long sym_elevate(){
  // After syscall should be in elevated state
  if(!is_sticky){
    return syscall(NR_ELEVATE_SYSCALL, SYSCALL_ELEVATE);
  }
  return -1;
}

// After syscall should be in lowered state
long sym_lower(){
  if(!is_sticky){
    return syscall(NR_ELEVATE_SYSCALL, SYSCALL_LOWER);
  }
  return -1;
}

void sym_elevate_syscall(){
  register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL; // write
  register uint64_t    arg1        __asm__("rdi") = SYSCALL_ELEVATE; //
  __asm__ __volatile__ (
                        "syscall"
                        : "+r" (syscall_no)
                        : "r" (arg1), "r" (syscall_no)
                        : "rcx", "r11", "memory"
                        );
}

void sym_lower_syscall(){
  register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL; // write
  register uint64_t    arg1        __asm__("rdi") = SYSCALL_LOWER; //
  __asm__ __volatile__ (
                        "syscall"
                        : "+r" (syscall_no)
                        : "r" (arg1), "r" (syscall_no)
                        : "rcx", "r11", "memory"
                        );
}

void sym_check_syscall(){
  register uint64_t    syscall_no  __asm__("rax") = NR_ELEVATE_SYSCALL; // write
  register uint64_t    arg1        __asm__("rdi") = SYSCALL_CHECK_ELEVATE_STATUS; //
  __asm__ __volatile__ (
                        "syscall"
                        : "+r" (syscall_no)
                        : "r" (arg1), "r" (syscall_no)
                        : "rcx", "r11", "memory"
                        );
}

#include <stdio.h>
#include <unistd.h>

#include <sys/syscall.h>
#include "sym_lib_syscall.h"

__thread int is_sticky = 0;

void sym_touch_stack(){
  // Num times to push
  int count = 1<<16;

  int i;
  // Touch a bunch of stack pages
  for(i=0; i < count; i++){
    asm("pushq $42");
  }

  // Pop them all off
  for(i=0; i < count; i++){
    asm("popq %rax");
  }
}

long sym_check_elevate(){
    return syscall(448, SYSCALL_CHECK_ELEVATE_STATUS);
}

long sym_elevate(){
  // After syscall should be in elevated state
  if(!is_sticky){
    return syscall(448, SYSCALL_ELEVATE);
  }
}

// After syscall should be in lowered state
long sym_lower(){
  if(!is_sticky){
    return syscall(448, SYSCALL_LOWER);
  }
}

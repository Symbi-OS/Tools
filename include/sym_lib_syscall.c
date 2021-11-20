#include <stdio.h>
#include <unistd.h>

#include <sys/syscall.h>

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

  printf("Push %d times, %d bytes, %d pages\n", count, count*8, (count*8) / (1<<12) );
}

long sym_check_elevate(){
  return syscall(448,1);
}

void sym_elevate(){
  syscall(448,1);
}

void sym_lower(){
  syscall(448,-1);
}

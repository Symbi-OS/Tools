#include <unistd.h>
#include <stdio.h>

#include "include/sym_lib.h"

void stack_test(){
  int i = 0;
  // Let's do a bunch of pushes and see if we can trigger the stack bug.
  intptr_t sp;
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Rsp is %lx\n", sp);
  for(; i < (1<<12); i++){
    asm("pushq $42");
  }
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Rsp is %lx\n", sp);

}

void touch_stack(){
  intptr_t sp;
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Touching stack Rsp is %lx\n", sp);

  int i = 0;
  // Push a bunch of values
  for(; i < (1<<12); i++){
    asm("pushq $42");
  }

  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Touching stack Rsp is %lx\n", sp);
  i = 0;

  // Pop them all off
  for(; i < (1<<12); i++){
    asm("popq %rax");
  }
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Touching stack Rsp is %lx\n", sp);
}

int main(){
  touch_stack();

  sym_elevate();

  stack_test();

  sym_lower();
  return 0;
}

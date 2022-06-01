#include <stdio.h>
#include "LINF/sym_all.h"
#include "../../libs/kallsymlib/kallsymlib.h"
void check_on_probe(uint64_t addr){
  sym_elevate();
  printf("1st byte getpid is %#x \n", *((unsigned char *) addr));
  sym_lower();
}

extern uint64_t int3_count;
extern uint64_t int3_rdi;
extern uint64_t int3_rsi;
extern uint64_t int3_rdx;

void test(){

  __asm__("                     \
  popq %rdi                   /*Done with 1st arg, restore user rdi  */ \n\t\
\
  mov    $0x0,%r15   \n\t   \
  mov    $0x1,%r14     \n\t   \
  mov    $0x2,%r13     \n\t   \
  mov    $0x3,%r12     \n\t   \
  mov    $0x4,%rbp     \n\t   \
  mov    $0x5,%rbx     \n\t   \
  mov    $0x6,% r11;    \n\t   \
  mov    $0x7,% r10;    \n\t   \
  mov    $0x8,% r9;     \n\t   \
  mov    $0x9,% r8;     \n\t   \
  mov    $0x10,% rax;    \n\t   \
  mov    $0x11,% rcx;    \n\t   \
  mov    $0x12,% rdx;    \n\t   \
  mov    $0x13,% rsi;    \n\t   \
  mov    $0x14,% rdi;    \n\t   \
  int3 \
");

}

int main(){

  sym_lib_init();

  void *addr__do_sys_getpid = sym_get_fn_address("__do_sys_getpid");

  sym_set_probe((uint64_t)addr__do_sys_getpid);

  check_on_probe((uint64_t)addr__do_sys_getpid);

  sym_elevate();

  getpid();

  check_on_probe((uint64_t)addr__do_sys_getpid);

  getpid();

  check_on_probe((uint64_t)addr__do_sys_getpid);

}

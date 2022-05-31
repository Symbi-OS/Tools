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

int main(){

  sym_lib_init();

  void *addr__do_sys_getpid = sym_get_fn_address("__do_sys_getpid");
  sym_set_probe((uint64_t)addr__do_sys_getpid);
  check_on_probe((uint64_t)addr__do_sys_getpid);

  getpid();

  printf("int3_count:%#lx\n",int3_count );
  printf("int3_rdi  :%#lx\n",int3_rdi   );
  printf("int3_rsi  :%#lx\n",int3_rsi   );
  printf("int3_rdx  :%#lx\n",int3_rdx   );

  getpid();

  check_on_probe((uint64_t)addr__do_sys_getpid);

}

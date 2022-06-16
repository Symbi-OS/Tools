#include <stdlib.h>
#include <stdio.h>
#include "LINF/sym_all.h"

#include "../../libs/kallsymlib/kallsymlib.h"

int f(){
  printf("PERFORMING ARBITRARY TASK\n");
  int x = 100;
  int y = 0, i = 0;
  for(; i < x; i++)
    y+=i;
  return y;
}

int main(){
  int core = 0;
  void * f_ptr = &f;
  uint64_t db_reg = 0;
  struct scratchpad * sp = (struct scratchpad *)get_scratch_pg(core);
  
  sym_lib_init();
  sym_probe_init();
  printf("SETTING TRIGGER AT %p\n", f_ptr);
  sym_set_db_probe((uint64_t)f_ptr, db_reg, DB_LOCAL); 
 
  sym_elevate();
  sp->debug = 1;
  sym_lower();
  
  if(fork() == 0)
    printf("RUNNING IN CHILD PROCESS\n");
  else
    printf("RUNNING IN PARENT PROCESS\n");
  
  f();

  printf("PROCESS COMPLETE\n");

  return 0;
}

#define _GNU_SOURCE
#include <sched.h>
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

int main(int argc, char * argv[]){
  int core = 0;
  void * f_ptr = &f;
  uint64_t db_reg = 0;
  struct scratchpad * sp = (struct scratchpad *)get_scratch_pg(core);
  int locality;

  if(argc > 1)
    locality = atoi(argv[1]);
  else
    locality = DB_LOCAL;

  //g = locality;

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);
  sched_setaffinity(0, sizeof(mask), &mask);
  
  sym_lib_init();
  sym_probe_init();
  printf("SETTING TRIGGER AT %p\n", f_ptr);
  sym_set_db_probe((uint64_t)f_ptr, db_reg, locality); 

  if(!locality)
    printf("TESTING LOCAL FLAG\n");
  else
    printf("TESTING GLOBAL FLAG\n");
 
  sym_elevate();
  sp->debug = 1;
  sp->cnt = 0;
  sym_lower();
  
  if(fork() == 0){
    printf("RUNNING IN CHILD PROCESS\n\n");

    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    sched_setaffinity(0, sizeof(mask), &mask);
  } else {
    printf("RUNNING IN PARENT PROCESS\n\n");
  }
  
  f();

  sym_elevate();
  int cnt = (int)sp->cnt;
  int hit = (int)sp->get.dr_hit;
  sym_lower();

  printf("PROCESS COMPLETE\n\n%d HIT(S) DETECTED ON DR%d\n", cnt, hit);

  return 0;
}

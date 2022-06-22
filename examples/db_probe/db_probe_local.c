#define _GNU_SOURCE
#include <sys/wait.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include "LINF/sym_all.h"

#include "../../libs/kallsymlib/kallsymlib.h"

#define RESET "\033[0m"
#define RED "\033[31m"     /* Red */
#define GREEN "\033[32m"   /* Green */
#define YELLOW "\033[33m"  /* Yellow */
#define BLUE "\033[34m"    /* Blue */
#define MAGENTA "\033[35m" /* Magenta */
#define CYAN "\033[36m"    /* Cyan */

int locality;

uint64_t get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
    while(1);
  }
  return info->addr;
}

int main(int argc, char * argv[]){
  int core = 0;
  void * f_ptr;
  uint64_t db_reg = 0;
  struct scratchpad * sp = (struct scratchpad *)get_scratch_pg(core);

  if(argc > 1)
    locality = atoi(argv[1]);
  else
    locality = DB_LOCAL;

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);
  sched_setaffinity(0, sizeof(mask), &mask);
  
  sym_lib_init();
  sym_probe_init();
  f_ptr = (void *)get_fn_address("__do_sys_getpid");
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
  int pid;
  if((pid = fork()) == 0){
    printf("\nRUNNING IN CHILD PROCESS\n\n");
    char * param_list[] = {"./db_probe/test", NULL};
    execvp(param_list[0], param_list);
  } else {
    waitpid(-1, NULL, 0);
    printf("\nRUNNING IN PARENT PROCESS\n\n");
  }
  

  sym_elevate();
  int cnt = (int)sp->cnt;
  int hit = (int)sp->get.dr_hit;
  sym_lower();

  if(pid != 0){
    if((cnt > 0 && locality == 0)||(cnt < 1 && locality == 1)){
      printf(RED);
      printf("LOCALITY TEST FAILED\n\n");
    } else {
      printf(GREEN);
      printf("LOCALITY TEST PASSED\n\n");
    }
    printf("%d HIT(S) DETECTED ON DR%d\n", cnt, hit);
  }
  printf(RESET);
  return 0;
}

#define _GNU_SOURCE
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include "LINF/sym_all.h"

// Some colors for printf
#define RESET "\033[0m"
#define RED "\033[31m"     /* Red */
#define GREEN "\033[32m"   /* Green */
#define YELLOW "\033[33m"  /* Yellow */
#define BLUE "\033[34m"    /* Blue */
#define MAGENTA "\033[35m" /* Magenta */
#define CYAN "\033[36m"    /* Cyan */

int f(){
//	printf("PERFORMING ARBITRARY TASK\n");
	int x = 1000000;
	int y = 0, i = 0;
	for(; i < x; i++)
		y+=i;
	return y;
}

int g(){
//	printf("PERFORMING ARBITRARY TASK\n");
	int x = 2000000;
	int y = 0, i = 0;
	for(; i < x; i++)
		y+=i;
	return y;
}

int main(int argc, char* argv[]) {
  if(argc != 3)
    return -1;
  int core = atoi(argv[1]);
  int db_reg = atoi(argv[2]);
  void * f_ptr;
  if(core)
    f_ptr = &f;
  else
    f_ptr = &g;

  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(core, &mask);
  sched_setaffinity(0, sizeof(mask), &mask);

  struct scratchpad * sp = (struct scratchpad *)get_scratch_pg(core);
  
  /*
  sym_elevate();
  sp->debug = 1;
  sym_lower();
  */

  uint64_t x = 9;
  sym_lib_init();
  //printf("SETTING TRIGGER AT %p\n", f_ptr);
  sym_set_db_probe((uint64_t)f_ptr, db_reg, DB_GLOBAL); 
  
  if(core)
    f();
  else
    g();

  sym_elevate();
  x = sp->get.dr_hit;
  sym_lower();

  //printf("\nDEBUG REGISTER %ld HIT ON CORE %d\n", x, core);
  if((int)x == db_reg){
    printf(GREEN);
    printf("TEST PASSED\n");
    printf(RESET);
    return 0;
  } else {
    printf(RED);
    printf("TEST FAILED\n");
    printf(RESET);
    return 1;
  }
    
  //printf("\nDONE MAIN\n");
}

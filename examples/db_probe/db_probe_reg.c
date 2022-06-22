#include <unistd.h>
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

uint64_t get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
    while(1);
  }
  return info->addr;
}

int main(/*int argc, char * argv[]*/){
  int core = 0;
  void * f_ptr;
  uint64_t db_reg = 0;
  struct scratchpad * sp = (struct scratchpad *)get_scratch_pg(core);
  int locality = 1;
  uint64_t rdi, rsi, rdx;
  
  sym_lib_init();
  sym_probe_init();
  f_ptr = (void *)get_fn_address("ksys_write");
  printf("SETTING TRIGGER AT %p\n", f_ptr);
  sym_set_db_probe((uint64_t)f_ptr, db_reg, locality); 
  
  char* buf = "test\n";

  write(1, buf, 5);

  sym_elevate();
  rdi = sp->get.pt_r.rdi;
  rsi = sp->get.pt_r.rsi;
  rdx = sp->get.pt_r.rdx;
  sym_lower();

  printf("INPUTS: %d %p %d\nREGISTERS: %ld 0x%lx %ld\n", 1, buf, 5, rdi, rsi, rdx);

  if(rdi == 1 && rsi == (uint64_t)buf && rdx == 5){
    printf(GREEN);
    printf("TEST PASSED\n");
    printf(RESET);
  }
  
  return 0;
}

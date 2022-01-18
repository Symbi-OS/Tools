#include <stdio.h>
#include "LIDK/idk.h"
#include <stdint.h>

#include "L2/sym_lib_page_fault.h"
#include "L2/sym_probe.h"

void* sym_get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
    while(1);
  }
  return (void *) info->addr;
}

void sym_l2_init(){
  printf("sym_l2_init\n");
  sym_lib_page_fault_init();
  sym_probe_init();
}

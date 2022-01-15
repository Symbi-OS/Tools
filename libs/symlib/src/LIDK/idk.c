#include <stdio.h>
#include "LIDK/idk.h"
#include <stdint.h>
uint64_t sym_get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
    while(1);
  }
  return info->addr;
}

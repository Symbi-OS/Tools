#include <stdio.h>
#include "L2/init.h"

void sym_l2_init(){
  printf("sym_l2_init\n");
  sym_lib_page_fault_init();
}

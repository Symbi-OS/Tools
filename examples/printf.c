#include <stdio.h>
#include "include/sym_lib.h"

int main(){
  sym_elevate();
  printf("Printf works in kernel mode.\n");
  sym_lower();
  return 0;
}
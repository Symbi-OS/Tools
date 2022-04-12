#include <stdio.h> // for kallsymlib... bug.
#include <assert.h>
#include <stdlib.h>
#include "LINF/sym_all.h"

int main(int argc, char *argv[]){
  int count = atoi(argv[1]);
  assert(count >= 1);
  for(int i = 0; i<count; i++ ){
    sym_elevate();
    sym_lower();
  }
}

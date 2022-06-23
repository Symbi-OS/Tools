#include <stdio.h> // for kallsymlib... bug.
#include <assert.h>
#include <stdlib.h>
#include "LINF/sym_all.h"

#define USE_MODE_SHIFT

int main(__attribute((unused))int argc, char *argv[]){
  // Assumes some mitigation is used.
  int count = atoi(argv[1]);
  assert(count >= 1);

  for(int i = 0; i<count; i++ ){
    sym_elevate();
    sym_lower();
  }
}

#include <stdio.h>
//#include "include/sym_lib.h"
//#include "examples/constructor_elevate.h"
#include "elevate.h"

int main(int argc, char* argv[]) {
  printf("Printing in kernel mode\n");
  hi();
  return 0;
}

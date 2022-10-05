#include <stdio.h>
#include "helper.h"
void expected(){
  printf("in hijacked lib, expectations upended\n");
  helper();
}

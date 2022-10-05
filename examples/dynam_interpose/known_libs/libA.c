#include <stdio.h>
#include "libB.h"
void fn_A(){
  printf("in %s\n", __func__);
  fn_B();
}

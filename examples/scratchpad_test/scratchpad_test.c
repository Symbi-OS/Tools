#include <stdlib.h>
#include <stdio.h>

#include "LINF/sym_all.h"

#include "../../libs/kallsymlib/kallsymlib.h"

int main() {
  struct scratchpad * sp = (struct scratchpad *)get_scratch_pg(0);
  uint64_t dr_hit = 9;
  sym_elevate();
  dr_hit = sp->s0.dr_hit;
  sym_lower();

  printf("SCRATCHPAD LOCATION: %p\n", sp);
  printf("REGISTER %ld HIT\n", dr_hit);

  return 0;
}

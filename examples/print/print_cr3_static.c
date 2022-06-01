#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../libs/symlib/include/LINF/sym_all.h"

int main(__attribute((unused)) int argc, char *argv[]){
  int count = atoi(argv[1]);
  assert(count >= 1);
  uint64_t cr3_reg;
  /* asm asm-qualifiers ( AssemblerTemplate  */
  /*                      : OutputOperands  */
  /*                        [ : InputOperands */
  /*                          [ : Clobbers ] ]) */

  // cr3: virtual address of the page table.
  sym_elevate();

  for(int i = 0; i<count; i++ ){
    asm("movq %%cr3,%0"
        : "=r"(cr3_reg)
        );
  }

  sym_lower();
  printf("Cr3 holds %lx\n", cr3_reg);

  return 0;
}

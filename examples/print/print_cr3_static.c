#include <stdint.h>
#include <stdio.h>
#include "../../include/headers/sym_all.h"

int main(){

  uint64_t cr3_reg;
  /* asm asm-qualifiers ( AssemblerTemplate  */
  /*                      : OutputOperands  */
  /*                        [ : InputOperands */
  /*                          [ : Clobbers ] ]) */

  // cr3: virtual address of the page table.
  sym_elevate();
  asm("movq %%cr3,%0"
      : "=r"(cr3_reg)
      );
  sym_lower();
  printf("Cr3 holds %p\n", cr3_reg);

  return 0;
}

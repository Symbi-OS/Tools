#include <stdint.h>
#include <stdio.h>
int main(){

  uint64_t cr3_reg;
  asm("movq %%cr3,%0"
      : "=r"(cr3_reg)
      );

  printf("Cr3 holds %p\n", cr3_reg);

  return 0;
}

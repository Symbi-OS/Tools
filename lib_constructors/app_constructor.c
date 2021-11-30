#include <stdio.h>
#include <stdint.h>

int main(){
  uint64_t cr3_reg;

  printf("This prints in kernel mode\n");
  asm("movq %%cr3,%0"
      : "=r"(cr3_reg)
      );
  printf("Cr3 holds %p\n", cr3_reg);
  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>

#include "../libs/symlib/include/LINF/sym_all.h"
#include "snapshot_tool.h"

uint64_t get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
  }
  return (uint64_t) info->addr;
}

int main(int argc, char * argv[]){
  uint64_t cr3, db_reg = 0, core = 0;
  void * pg_tbl = NULL;
  char * fn_name;
  uint64_t fn_addr;
  struct scratchpad * sp; 

  if(argc < 3)
    return 1;

  fn_name = argv[1];
  db_reg = atoi(argv[2]);
  core = atoi(argv[3]);
  
  sp = (struct scratchpad *)get_scratch_pg(core);
  sym_lib_init();
  fn_addr = get_fn_address(fn_name);
  printf("SETTING TRIGGER AT 0x%lx\n", fn_addr);
  sym_set_db_probe((uint64_t)fn_addr, db_reg, DB_GLOBAL); 

  printf("IDLE UNTIL TRIGGER HIT\n");

  sym_elevate();
  sp->get.dr_hit = 9;
  //sym_lower();

  while(sp->get.dr_hit > 3);

  //sym_elevate();
  cr3 = sp->get.cr3;
  sym_lower();

  uint64_t phys = (cr3 >> 12) << 12;
  uint64_t virt = 0xffff888000000000;
  pg_tbl = (void *)(phys + virt);

  printf("CR3: 0x%lx\nPG_TBL ADDR: %p\n", cr3, pg_tbl);

  return 0;
}

#define _GNU_SOURCE
#include <sched.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "../../Symlib/include/LINF/sym_all.h"

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
  char * fn_name;// * target;
  uint64_t fn_addr;
  struct scratchpad * sp; 

  if(argc < 3)
    return 1;


  fn_name = argv[1];
  db_reg = atoi(argv[2]);
  core = atoi(argv[3]);
  
  // set probe on core
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(core, &mask);
  sched_setaffinity(0, sizeof(mask), &mask);

  sp = (struct scratchpad *)get_scratch_pg(core);
  sym_lib_init();
  fn_addr = get_fn_address(fn_name);
  printf("SETTING TRIGGER AT 0x%lx\n", fn_addr);
  sym_set_db_probe(fn_addr, db_reg, DB_GLOBAL); 
  
  // launch target

  printf("PID: %d\n", getpid());
  
  sym_elevate();
  cr3 = sp->get.cr3;
  sym_lower();

  pg_tbl = (void *)phys_to_virt(cr3);

/* 
  int fd = open("/dev/mem", O_RDWR);
  if(fd == -1){
    printf("%s\n", strerror(errno));
    return 1;
  }

  uint64_t phys = (cr3 >> 12) << 12;
  pg_tbl = mmap(NULL, PG_SZ, PROT_READ, MAP_PRIVATE, fd, phys);
  if(pg_tbl == MAP_FAILED){
    printf("%s\n", strerror(errno));
    return 1;
  }
*/
  printf("CR3: 0x%lx\nPG_TBL ADDR: %p\nPG_SZ: 0x%llx\n", cr3, pg_tbl, PG_SZ);

  char fname[100];
  strcpy(fname, fn_name);
  strcat(fname, ".snapshot");
  int snapshot = open(fname, O_CREAT);
  if(snapshot < 0)
    printf("%s\n", strerror(errno));
  printf("FILE DESC: %d\n", snapshot);
  char * buf = "TEST";

  ssize_t wr = write(snapshot, buf, 4);
  printf("WRITE LEN: %ld\n", wr);
  if(wr < 0)
    printf("%s\n", strerror(errno));

  return 0;
}

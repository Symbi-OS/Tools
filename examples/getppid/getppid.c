#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include "LINF/sym_all.h"

int bench_time(int count){
  clock_t start, end;
  double cpu_time_used;
  start = clock();
  /* Do the work. */
  int i;
  for(i=0; i<count; i++)
    getppid();

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Time used: %f\n", cpu_time_used);

  return 0;
}

int bench_time_elevated(int count){
  clock_t start, end;
  double cpu_time_used;
  start = clock();
  /* Do the work. */
  int i;
  sym_elevate();
  for(i=0; i<count; i++)
    getppid();
  sym_lower();

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Time used: %f\n", cpu_time_used);

  return 0;
}

typedef void (*void_fn_ptr)(unsigned long);
void_fn_ptr get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
  }
  return (void_fn_ptr) info->addr;
}

typedef int (*getppid_t)(void);
int bench_time_internal(int count){
  clock_t start, end;
  double cpu_time_used;

  getppid_t getppid_elevated = (getppid_t) get_fn_address("__x64_sys_getppid");

  start = clock();
  /* Do the work. */
  int i;
  sym_elevate();
  for(i=0; i<count; i++)
    getppid_elevated();
  sym_lower();

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Time used: %f\n", cpu_time_used);

  return 0;
}

int main(int argc, char *argv[]){
  int count = atoi(argv[1]);
  assert(count >= 1);
  assert(argc == 2);
	for (int  i=0; i <count; i++) {
    //  clock_gettime(CLOCK_MONOTONIC, &outer.start);
  }

  printf("starting getppid syscall\n");
  bench_time(count);

  sym_lib_init();
  sym_touch_every_page_text();

  printf("starting elevated syscall\n");
  bench_time_elevated(count);

  printf("starting shortcutted syscall\n");
  bench_time_internal(count);

  return 0;
}

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "LINF/sym_all.h"

#define ITERATIONS 1<<24

int bench_time(){
  clock_t start, end;
  double cpu_time_used;
  start = clock();
  /* Do the work. */
  int i;
  for(i=0; i< ITERATIONS; i++)
    getppid();

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Time used: %f\n", cpu_time_used);

  return 0;
}

int bench_time_internal(){
  clock_t start, end;
  double cpu_time_used;

  int (*getppid_elevated)() = ( int(*)() ) 0xffffffff810fc0c0;

  start = clock();
  /* Do the work. */
  int i;
  for(i=0; i< ITERATIONS; i++)
    getppid_elevated();

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Time used: %f\n", cpu_time_used);

  return 0;
}

int main(){
  printf("starting main\n");

  int (*getppid_elevated)() = ( int(*)() ) 0xffffffff810f62b0;

  bench_time();

  sym_elevate();
  bench_time_internal();
  sym_lower();

  return 0;
}

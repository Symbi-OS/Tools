#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "../include/sym_lib.h"

int bench_time(){
  clock_t start, end;
  double cpu_time_used;
  start = clock();
  /* Do the work. */
  int i;
  for(i=0; i< 1<<21; i++)
    getppid();

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; 
  printf("Time used: %f\n", cpu_time_used);

  return 0;
}

int bench_time_internal(){
  clock_t start, end;
  double cpu_time_used;

  /* int (*getppid_elevated)() = ( int(*)() ) 0xffffffff8107f460; */
  int (*getppid_elevated)() = ( int(*)() ) 0xffffffff810f62b0;
  start = clock();
  /* Do the work. */
  int i;
  for(i=0; i< 1<<21; i++)
    getppid_elevated();

  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; 
  printf("Time used: %f\n", cpu_time_used);

  return 0;
}

int main(){
  printf("starting main\n");

  // Need generic way to get this.
  /* tommyu@don:~/Symbi-OS/linux$ nm vmlinux  | grep sys_getppid */
  /*   ffffffff8107f460 t __do_sys_getppid */
  /*   ffffffff826ad980 t _eil_addr___ia32_sys_getppid */
  /*   ffffffff826ad990 t _eil_addr___x64_sys_getppid */
  /*   ffffffff8107f460 T __ia32_sys_getppid */
  /*   ffffffff8107f460 T __x64_sys_getppid */
  int (*getppid_elevated)() = ( int(*)() ) 0xffffffff810f62b0;

  sym_elevate();

  /* int ppid=getppid_elevated(); */

  /* bench_time(); */
  bench_time_internal();

  sym_lower();

  /* printf("elevated ppid is %d \n", ppid); */

  /* printf("ppid is %d \n", getppid()); */
  return 0;
}

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

int main(int argc, char *argv[]){
  if(argc != 2){
    printf("usage ./unroll <app_work>\n");
    exit(1);
  }

  int work_pwr_2 = atoi(argv[1]);
  int work_total = 1 << work_pwr_2;

  while(1){
    // app work
    register int work_ctr = work_total;

    while (work_ctr--) {
      asm("nop");
    }
    // system work
    getppid();
  }

}

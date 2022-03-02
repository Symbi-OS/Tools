#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include <unistd.h>

// call this function to start a nanosecond-resolution timer
struct timespec timer_start() {
  struct timespec start_time;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
  return start_time;
}

// call this function to end a timer, returning nanoseconds elapsed as a long
long timer_end(struct timespec start_time) {
  struct timespec end_time;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
  long diffInNanos = (end_time.tv_sec - start_time.tv_sec) * (long)1e9 +
                     (end_time.tv_nsec - start_time.tv_nsec);
  return diffInNanos;
}

int main(int argc, char *argv[]){
  if(argc != 3){
    printf("usage ./unroll <app_work>\n");
    exit(1);
  }

  int work_pwr_2 = atoi(argv[1]);
  int work_total = 1 << work_pwr_2;

  int app_work_pwr_2 = atoi(argv[2]);
  int app_work_total = 1 << app_work_pwr_2;

  // get cycles and instructions

  // get time high resolution

  struct timespec vartime = timer_start();
  while(work_total--){

    // app work
    //    register int work_ctr = work_total;
#ifdef USER
    int app_work_total_tmp = app_work_total;

    while (app_work_total_tmp--) {
      asm("nop");
    }
#endif

    // system work
#ifdef SYSTEM
    getppid();
#endif

  }
  long time_elapsed_nanos = timer_end(vartime);
  printf("Time taken (micro seconds): %ld\n", time_elapsed_nanos/1000);
  // get cycles and instructions

}

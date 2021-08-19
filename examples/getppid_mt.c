#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>

#include "../include/sym_lib.h"
int work;
int work_total;

// Concerned this is being shared.
union{
  long turn;
  char cache_line_sz[64];
} padded_turn, padded_app_ctr, padded_sys_ctr __attribute__((aligned(64)));


int app_work_total;
// System worker thread
pthread_t sys_work_thread;

// p1

// call this function to start a nanosecond-resolution timer
struct timespec timer_start() {
  struct timespec start_time;
  /* clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time); */
  clock_gettime(CLOCK_MONOTONIC, &start_time);
  return start_time;
}

// call this function to end a timer, returning nanoseconds elapsed as a long
long timer_end(struct timespec start_time) {
  struct timespec end_time;
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  long diffInNanos = (end_time.tv_sec - start_time.tv_sec) * (long)1e9 +
                     (end_time.tv_nsec - start_time.tv_nsec);
  return diffInNanos;
}

#ifndef NOPINNING
int stick_this_thread_to_core(int core_id) {
  int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
  if (core_id < 0 || core_id >= num_cores)
    return EINVAL;

  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);

  pthread_t current_thread = pthread_self();
  return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}
#endif

#ifndef NOFUNCALIGN
void *system_work(void *arg) __attribute__((aligned(1 << 12)));
void *app_work(void *arg) __attribute__((aligned(1 << 12)));
#endif

void *system_work(void *arg) {

  int (*getppid_elevated)() = ( int(*)() ) 0xffffffff810f62b0;

#ifndef NOPINNING
  stick_this_thread_to_core(2);
#endif

    // give plenty of time for other thread to get up and pinned.
    /* usleep(1000 * 100); */
  sym_elevate();

  struct timespec vartime = timer_start();
#ifdef DOPRINTOUT
#endif
  // System thread work.
  while (work_total--) {

    // Spin until app signals
    while (padded_turn.turn == 1){
      padded_sys_ctr.turn++;
    };
    /* printf("sys work\n"); */

    // If doing ping pong test, remove this code.
/* #ifndef PINGPONG */
    /* getppid(); */
    getppid_elevated();
/* #endif */
    padded_turn.turn = 1;
  }
#ifdef DOPRINTOUT
  printf("Time taken (micro seconds): %ld\n", time_elapsed_nanos/1000);
#endif
  long time_elapsed_nanos = timer_end(vartime);

  printf("%d", (padded_app_ctr.turn + padded_sys_ctr.turn) / work );
  exit(0);
  return NULL;
}

void *app_work(void *arg){

    while (1) {
      int app_work_total_tmp = app_work_total;

      // Spin until system signals.
      while (padded_turn.turn == 0) {
        padded_app_ctr.turn++;
      };
      /* printf("App work\n"); */

      // If doing ping pong test, remove this code.
#ifndef PINGPONG
    // The actual work
    while (app_work_total_tmp--) {
      asm("nop");
    }
#endif

    padded_turn.turn = 0;
  }


  return NULL;
  }

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("usage ./unroll <total_work> <app_work>\n");
    exit(1);
  }

  padded_app_ctr.turn = 0;
  padded_sys_ctr.turn = 0;

  padded_turn.turn = 0;

  // 2^<input> many nops executed by app
  work_total = 1 << atoi(argv[1]);
  work = work_total;

  // How many nops to run per outer loop
  app_work_total = 1 << atoi(argv[2]);

  // Create system thread
  pthread_create(&sys_work_thread, NULL, system_work, NULL);

  // Do app work on main thread
#ifndef NOPINNING
  stick_this_thread_to_core(0);
#endif

  app_work(NULL);

#ifdef DOPRINTOUT
  pthread_cancel(sys_work_thread);
  printf("App spins:\t%d\n", padded_app_ctr.turn);
  printf("Sys spins:\t%d\n", padded_sys_ctr.turn);
  printf("Work total:\t%d\n", work);
#endif

  /* printf("spins per RT pingpong\n"); */

  /* pthread_join(sys_work_thread, NULL); */
}

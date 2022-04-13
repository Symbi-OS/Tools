#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "LINF/sym_all.h"

/* #define ONE_TIMER */
/* #define SYM_ELEVATE */

FILE *fp;

void calc_diff(struct timespec *diff, struct timespec *bigger, struct timespec *smaller)
{
	if (smaller->tv_nsec > bigger->tv_nsec)
    {
      diff->tv_nsec = 1000000000 + bigger->tv_nsec - smaller->tv_nsec;
      diff->tv_sec = bigger->tv_sec - 1 - smaller->tv_sec;
    }
	else
    {
      diff->tv_nsec = bigger->tv_nsec - smaller->tv_nsec;
      diff->tv_sec = bigger->tv_sec - smaller->tv_sec;
    }
}

struct Record
{
	size_t size;
	struct timespec start;
	struct timespec end;
};

void clock_bench(int loop) {

	struct timespec diff = {0, 0};
	int l;

#ifndef ONE_TIMER
	struct Record *runs;

	runs = mmap(NULL, sizeof(struct Record) * loop, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	memset(runs, 0, sizeof(struct Record) * loop);
#endif

#ifdef SYM_ELEVATE
  sym_elevate();
#endif

#ifdef ONE_TIMER
  struct Record stacker;
#endif

  struct Record outer;
  clock_gettime(CLOCK_MONOTONIC, &outer.start);

	for (l = 0; l < loop; l++) {
#ifdef ONE_TIMER
    clock_gettime(CLOCK_MONOTONIC, &stacker.start);
    clock_gettime(CLOCK_MONOTONIC, &stacker.end);
#else
    clock_gettime(CLOCK_MONOTONIC, &runs[l].start);
    clock_gettime(CLOCK_MONOTONIC, &runs[l].end);
#endif
  }

  clock_gettime(CLOCK_MONOTONIC, &outer.end);

#ifdef SYM_ELEVATE
  sym_lower();
#endif


  calc_diff(&diff, &outer.end, &outer.start);
  printf("Runtime %ld.%09ld\n", diff.tv_sec, diff.tv_nsec);

	munmap(runs, sizeof(struct Record) * loop);

	return;
}

void elevated_clock(int count){
  struct Record outer;

  sym_elevate();
	for (int  i=0; i < count; i++) {
    clock_gettime(CLOCK_MONOTONIC, &outer.start);
  }
  sym_lower();
}

int main(int argc, char *argv[]){
  int count = atoi(argv[1]);
  assert(count >= 1);
  /* clock_bench(); */
  elevated_clock(count);
}

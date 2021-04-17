#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include <string.h>

#include "../include/sym_lib.h"

int NUM_REPS = 1<<0;
/* int NUM_REPS = 1<<30; */

void handle_sigint(int sig)
{
  fprintf(stderr, "Caught signal %d\n", sig);
  fprintf(stderr, "About to do lower in sig handler\n");
  sym_lower();
  fprintf(stderr, "Done with lower\n");
  exit(0);
}


#define RD_SYSCALL_NUM 0
void safer_syscall_loop(int reps){
  assert(reps > 0);

  /* int fd = open("out.txt", O_RDONLY); */
  int fd = open("out.txt", O_RDONLY);

  /* printf("fd = %d \n", fd); */

  if (fd ==-1)
    {
      // print which type of error have in a code
      printf("Error Number % d\n", errno);

      // print program detail "Success or failure"
      perror("Program");
    }
  int sz = 7;
  const char buf[sz]; // = "Tommy!\n";
  size_t size = sz;

  while(reps--){
    register int64_t rax __asm__ ("rax") = RD_SYSCALL_NUM;
    register int rdi __asm__ ("rdi") = fd;
    register const void *rsi __asm__ ("rsi") = buf;
    register size_t rdx __asm__ ("rdx") = size;
    __asm__ __volatile__ (
                          "syscall"
                          : "+r" (rax)
                          : "r" (rdi), "r" (rsi), "r" (rdx)
                          : "rcx", "r11", "memory"
                          );
  }

    /* fprintf(stderr, "read: %s\n", buf); */
    // Using close system Call
    if (close(fd) < 0)
      {
        perror("c1");
        exit(1);
      }
    printf("closed the fd.\n");
}
void standard_syscall_loop(int reps){
  printf("Not yet adapted from write\n");
  while(1);
  assert(reps > 0);

  while(reps--){
    /* Can even call a syscall from ring 0 */
    register int    syscall_no  asm("rax") = 0; // read
    register int    arg1        asm("rdi") = 1; // file des
    register char*  arg2        asm("rsi") = "Tommy\n";
    register int    arg3        asm("rdx") = 6;
    asm("syscall");
  }
}

void skipping_syscall_instruction(int reps){
  printf("Not yet adapted from write\n");
  while(1);
  assert(reps > 0);

  while(reps--){
    register int    syscall_no  asm("rax") = 0; // write
    register int    arg1        asm("rdi") = 1; // file des
    register char*  arg2        asm("rsi") = "Tommy\n";
    register int    arg3        asm("rdx") = 6;

    // Get r11 setup
    asm("pushfq");
    asm("popq %r11"); // let compiler know we're clobbering r11

    // Get rflags setup
    asm("cli");

    // Get RCX setup this is the retrun instruction.
    asm("mov $0x401de7, %rcx"); // specify clobber here

    asm("jmp 0xffffffff81c00010"); // lstar pointer to system call handler
  }
}

void ksys_read_shortcut(int reps){
  assert(reps > 0);

  int fd = open("out.txt", O_RDONLY);


  int (*my_ksys_read)(unsigned int fd, const char *buf, size_t count) =
    ( int(*)(unsigned int fd, const char *buf, size_t count) ) 0xffffffff8133e880;

  int sz = 7;
  const char buf[sz]; // = "Tommy!\n";

	while(reps--){
    //		if( (count % (1<<10)) == 0) {
    /* write(1, "Opportunity to catch a signal\n", 30); */
    //		} else {
    my_ksys_read(fd, buf, sz);
    fprintf(stderr, "read: %s\n", buf);
    //		}
	}
}
int main(){
  signal(SIGINT, handle_sigint);

  sym_elevate();

  clock_t start, end;
  double cpu_time_used;

  start = clock();

  ksys_read_shortcut(NUM_REPS);
 /* standard_syscall_loop(NUM_REPS); */
  /* safer_syscall_loop(NUM_REPS); */

  end = clock();

  sym_lower();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  fprintf(stderr, "Time used: %f\n", cpu_time_used);

  return 0;
}

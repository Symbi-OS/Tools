#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include "../include/sym_lib.h"

int NUM_REPS = 1<<23;
/* int NUM_REPS = 1<<30; */

void handle_sigint(int sig)
{
  fprintf(stderr, "Caught signal %d\n", sig);
  fprintf(stderr, "About to do lower in sig handler\n");
  sym_lower();
  fprintf(stderr, "Done with lower\n");
  exit(0);
}
#define WR_SYSCALL_NUM 1
void safer_syscall_loop(int reps){
  assert(reps > 0);

  int fd = 1;
  const void *buf = "Tommy!\n";
  size_t size = strlen(buf);

  while(reps--){
      register int64_t rax __asm__ ("rax") = WR_SYSCALL_NUM;
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

}
void standard_syscall_loop(int reps){
  assert(reps > 0);

  while(reps--){
    /* Can even call a syscall from ring 0 */
    register int    syscall_no  asm("rax") = 1; // write
    register int    arg1        asm("rdi") = 1; // file des
    register char*  arg2        asm("rsi") = "Tommy\n";
    register int    arg3        asm("rdx") = 6;
    asm("syscall");
  }
}

void skipping_syscall_instruction(int reps){
  assert(reps > 0);

  while(reps--){
    register int    syscall_no  asm("rax") = 1; // write
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

void ksys_write_shortcut(int reps){
  assert(reps > 0);

  int (*my_ksys_write)(unsigned int fd, const char *buf, size_t count) =
    ( int(*)(unsigned int fd, const char *buf, size_t count) ) 0xffffffff8133e990;

	while(reps--){
    //		if( (count % (1<<10)) == 0) {
    /* write(1, "Opportunity to catch a signal\n", 30); */
    //		} else {
    my_ksys_write(1, "Tommy\n", 6);
    /* fprintf("Tommy\n"); */
    //		}
	}
}
int main(){
  signal(SIGINT, handle_sigint);

  sym_elevate(); 

  clock_t start, end;
  double cpu_time_used;

  start = clock();

   ksys_write_shortcut(NUM_REPS); 
 /* standard_syscall_loop(NUM_REPS); */
//  safer_syscall_loop(NUM_REPS);

  end = clock();

  sym_lower(); 
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC; 
  fprintf(stderr, "Time used: %f\n", cpu_time_used);

  return 0;
}

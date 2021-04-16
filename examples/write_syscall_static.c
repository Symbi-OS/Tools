#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "../include/sym_lib.h"
void handle_sigint(int sig)
{
  printf("Caught signal %d\n", sig);
  printf("About to do lower in sig handler\n");
  sym_lower();
  printf("Done with lower\n");
  exit(0);
}


int main(){
  signal(SIGINT, handle_sigint);

  sym_elevate();

  /* printf("Can we read cr3?\n"); */
  /* uint64_t cr3_reg; */
  /* asm("movq %%cr3,%0" */
  /*     : "=r"(cr3_reg) */
  /*     ); */
  /* printf("Cr3 holds %p\n", cr3_reg); */

#if 0
  while(1){
    /* Can even call a syscall from ring 0 */
    register int    syscall_no  asm("rax") = 1; // write
    register int    arg1        asm("rdi") = 1; // file des
    register char*  arg2        asm("rsi") = "Tommy\n";
    register int    arg3        asm("rdx") = 6;
    asm("syscall");
  }

  /* void (*my_entry_SYSCALL_64)() = */
  /*   ( void(*)() ) 0xffffffff81c00010; */
  /* my_entry_SYSCALL_64(); */
  while(1){
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
#else
  int (*my_ksys_write)(unsigned int fd, const char *buf, size_t count) =
    ( int(*)(unsigned int fd, const char *buf, size_t count) ) 0xffffffff8133e990;

	int count = 1<<16;
	while(count--){
		if( (count % (1<<10)) == 0) {
			write(1, "Opportunity to catch a signal\n", 30);
		} else {
			my_ksys_write(1, "Tommy\n", 6);
      /* printf("Tommy\n"); */
		}
	}

  /* int (*my___x64_sys_write)(unsigned int fd, const char *buf, size_t count) = */
  /*   ( int(*)(unsigned int fd, const char *buf, size_t count) ) 0xffffffff8133ebd0; */
  /* my___x64_sys_write(1, "Tommy\n", 6); */

#endif

  sym_lower();

  return 0;
}

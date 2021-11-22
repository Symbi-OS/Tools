#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "include/sym_lib.h"

int main(){
  int output_fd;
  output_fd = open("/tmp/foo", O_WRONLY | O_CREAT, 0644);
  write(output_fd, "test", 4);

  sym_elevate();


  // Can write to app fd from libOS code.
  // Think user / group crediential donated to kernel.
  // Setup prior to entering kernel mode.

  register int    syscall_no  asm("rax") = 1; // write
  register int    arg1        asm("rdi") = output_fd; // file des
  register char*  arg2        asm("rsi") = "TEST";
  register int    arg3        asm("rdx") = 4;
  asm("syscall");

  sym_lower();
  return 0;
}

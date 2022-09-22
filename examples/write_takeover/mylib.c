#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include "sym_lib.h"

__thread int is_sticky = 0;

void __attribute__ ((constructor)) initLibrary(void) {
  //
  // Function that is called when the library is loaded
  //
  // Let's touch stack pages before elevating to try to avoid starvation.
  sym_elevate();
}

void __attribute__ ((destructor)) cleanUpLibrary(void) {
  //
  // Function that is called when the library is »closed«.
  //
  sym_lower();
}

static long sym_do_syscall(int work){
  if(!is_sticky){
    return syscall(NR_ELEVATE_SYSCALL, work);
  }
  // XXX obviously
  return 42;
}

long sym_mode_shift(uint64_t flags){
  return sym_do_syscall(flags);
}

long sym_elevate(){
  // XXX HACK This assumes we can just clobber user's gsbase...
  // I believe it also assumes no core migration...
  uint64_t kern_gs;

  long ret = sym_mode_shift( SYM_ELEVATE_FLAG | SYM_INT_DISABLE_FLAG );
  GET_KERN_GS_CLOBBER_USER_GS
  return ret;
}

long sym_lower(){
  /* return sym_do_syscall(SYSCALL_LOWER); */
  return sym_mode_shift(SYM_LOWER_FLAG );

}

long sym_check_elevate(){
  /* return sym_do_syscall(SYSCALL_CHECK_ELEVATE_STATUS); */
  return sym_mode_shift(SYM_QUERY_FLAG);
}

ssize_t write(int fd, const void* buf, size_t len){
  /*shortcutted ksyswrite call*/
  my_ksys_write_t my_ksys_write = 0xffffffff813670f0;
  return my_ksys_write(fd, buf, len);
  /* normal write syscall */
  /* return syscall(SYS_write, fd, buf, len); */
}

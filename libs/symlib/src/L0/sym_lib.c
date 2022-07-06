// License C 2021-2022
// Author: Thomas Unger
#include <unistd.h>
#include <sys/syscall.h>

#include <stdint.h>
#include "L0/sym_lib.h"

// This gives 1 level of toggle prevention.
__thread int is_sticky = 0;

/* Implementation is almost entirely archetecture specific, check arch dir. */
#ifdef CONFIG_X86_64
#include "../../arch/x86_64/L0/sym_lib.h"
#endif

#ifdef DYNAM
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
#endif


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


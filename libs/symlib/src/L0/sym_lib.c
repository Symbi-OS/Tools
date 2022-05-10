// License C 2021-2022
// Author: Thomas Unger

#include <stdint.h>
#include "L0/sym_lib.h"

// This gives 1 level of toggle prevention.
__thread int is_sticky = 0;

/* Implementation is almost entirely archetecture specific, check arch dir. */
#ifdef CONFIG_X86_64
#include "../../arch/x86_64/L0/sym_lib.h"
#endif

long sym_mode_shift(uint64_t flags){
  return sym_do_syscall(flags);
}

long sym_elevate(){
  // XXX HACK This assumes we can just clobber user's gsbase...
  // I believe it also assumes no core migration...
  uint64_t kern_gs;

  long ret = sym_mode_shift( SYM_ELEVATE_FLAG | SYM_INT_DISABLE_FLAG );
  // XXX make this architecture agnostic
  asm("swapgs"); // get onto kern gs
  asm("rdgsbase %0" : "=rm"(kern_gs) :: ); // Store kern gs
  asm("swapgs"); // get onto user gs
  asm("wrgsbase %0" :: "r"(kern_gs) ); // Overwrite user gs with kern gs
  asm("sti"); // make interruptable
  return ret;
}

long sym_lower(){
  /* return sym_do_syscall(SYSCALL_LOWER); */
  return sym_mode_shift(SYM_LOWER_FLAG);

}

long sym_check_elevate(){
  /* return sym_do_syscall(SYSCALL_CHECK_ELEVATE_STATUS); */
  return sym_mode_shift(SYM_QUERY_FLAG);
}


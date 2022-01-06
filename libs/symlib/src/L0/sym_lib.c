// License C 2021-
// Author: Thomas Unger
// Level: 0
#define _GNU_SOURCE

#include <stdint.h>
#include "L0/sym_lib.h"
#include "../../arch/x86/arch_x86.h"

// This gives 1 level of toggle prevention.
__thread int is_sticky = 0;

long sym_elevate(){
  ELEVATE
  return -1;
}

// After syscall should be in lowered state
long sym_lower(){
  LOWER
  return -1;
}

long sym_check_elevate(){
  SYM_CHECK
}

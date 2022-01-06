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

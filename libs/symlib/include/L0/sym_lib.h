#include <stdint.h>

#ifndef __SYM_LIB_SYSCALL__
#define __SYM_LIB_SYSCALL__

#define NR_ELEVATE_SYSCALL 448
#define SYSCALL_ELEVATE 1
#define SYSCALL_LOWER -1
#define SYSCALL_CHECK_ELEVATE_STATUS 0

// Stickyness ignores elevate and lower calls until unset.
#define CHECK_STICKY -1
#define STICKY 1
#define NOT_STICKY 0



/*
Symbiote flags

Page fault error codes: 
31              15                             4    3   2   1   0
+-------------------------------------------------+---+---+---+---+
|                                                 | DB|ID | Q | E |
+-------------------------------------------------+---+---+---+---+

Legend: Set (Clear)
0 Elevate (lower)
1 Query (don't)
2 Iret with Interrupts Disabled (Enabled)
3 Debug printks for elevate syscall
*/

#define SYM_LOWER_FLAG 0

#define SYM_ELEVATE_FLAG 1
#define SYM_QUERY_FLAG (1<<1)
#define SYM_INT_DISABLE_FLAG (1<<2)
#define SYM_DEBUG_FLAG (1<<3)

// 'Q'uery, 'D'isable interrupts, 'E'levate
extern long sym_mode_shift(uint64_t flags);

// Execute syscall setting elevated bit. Calls glibc syscall fn.
extern long sym_elevate();

// Execute syscall clearing elevated bit
extern long sym_lower();

// Query elevation status
extern long sym_check_elevate();

// Lock current elevation status until unset
extern int set_sticky(int is_sticky);

#endif

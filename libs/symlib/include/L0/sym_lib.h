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
31              15              8   7   6   5   4   3   2   1   0
+-------------------------------------------------+---+---+---+---+
|                             |FL |R\I|SMA|SME|  DB   |ID | Q | E |
+-------------------------------------------------+---+---+---+---+

Legend: Set (Clear)
0 Elevate (Lower)
1 Query elevation status (don't)
2 Iret with Interrupts Disabled (Enabled)
3-4 Debug printks for elevate syscall 0 none, 3 most
5 noSMEP (SMEP)
6 noSMAP (SMAP)
7 Return from elevate using RET (IRET)
8 Fast lower via SYSRET (SYSCALL -> SYSRET)
*/

#define SYM_LOWER_FLAG 0
// 0 Elevate (Lower)
#define SYM_ELEVATE_FLAG 1
// 1 Query (Don't)
#define SYM_QUERY_FLAG (1<<1)
// 2 Return with Int Disabled (Enabled)
#define SYM_INT_DISABLE_FLAG (1<<2)
// 3-4 Debug Level (0 for quiet)
#define SYM_DEBUG_LOW_FLAG (1<<3)
#define SYM_DEBUG_MED_FLAG (2<<3)
#define SYM_DEBUG_HGH_FLAG (3<<3)
// 5 Disable SMEP (Enabled)
#define SYM_NOSMEP_FLAG (1<<5)
// 6 Disable SMAP (Enabled)
#define SYM_NOSMAP_FLAG (1<<6)
// 7 Return with Ret (Iret)
#define SYM_RET_FLAG (1<<7)
// 8 Don't do syscall, just SYSRET to self.
#define SYM_FAST_LOWER_FLAG (1<<8)


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

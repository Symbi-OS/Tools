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

// Execute syscall setting elevated bit. Calls glibc syscall fn.
long sym_elevate();

// Execute syscall clearing elevated bit
long sym_lower();

// Query elevation status
long sym_check_elevate();

// Lock current elevation status until unset
extern int set_sticky(int is_sticky);

#endif

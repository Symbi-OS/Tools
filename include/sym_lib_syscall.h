#ifndef __SYM_LIB_SYSCALL__
#define __SYM_LIB_SYSCALL__

#define SYSCALL_ELEVATE 1
#define SYSCALL_LOWER -1
#define SYSCALL_CHECK_ELEVATE_STATUS 0

// Stickyness ignores elevate and lower calls until unset.
#define CHECK_STICKY -1
#define STICKY 1
#define NOT_STICKY 0

// Execute syscall setting elevated bit
extern long sym_elevate();
// Execute syscall clearing elevated bit
extern long sym_lower();

// Touch some stack pages
extern void sym_touch_stack();

// Query elevation status
extern long sym_check_elevate();

// Lock current elevation status until unset
extern int set_sticky(int is_sticky);

// Restore system IDT
void sym_restore_system_idt();

#endif

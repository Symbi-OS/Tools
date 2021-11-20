#ifndef __SYM_LIB_SYSCALL__
#define __SYM_LIB_SYSCALL__
extern void sym_elevate();
extern void sym_lower();
extern void sym_touch_stack();
extern long sym_check_elevate();
#endif

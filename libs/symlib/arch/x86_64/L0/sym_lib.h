#ifndef __ARCH_X86_64_SYM_LIB__
#define __ARCH_X86_64_SYM_LIB__

/* #include <unistd.h> */
/* #include <sys/syscall.h> */

/* #include "L0/sym_lib.h" */

// get onto kern gs
// Store kern gs
// get onto user gs
// Overwrite user gs with kern gs
// make interruptable
#define GET_KERN_GS_CLOBBER_USER_GS                           \
  asm("swapgs");                                              \
  asm("rdgsbase %0" : "=rm"(kern_gs) :: );                    \
  asm("swapgs");                                              \
  asm("wrgsbase %0" :: "r"(kern_gs) );                        \
  asm("sti");

#endif

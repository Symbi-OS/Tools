#include "sym_lib_syscall.h"
void __attribute__ ((constructor)) initLibrary(void) {
  //
  // Function that is called when the library is loaded
  //

  // Let's touch stack pages before elevating to try to avoid starvation.
  sym_touch_stack();
  sym_elevate();
}

void __attribute__ ((destructor)) cleanUpLibrary(void) {
  //
  // Function that is called when the library is »closed«.
  //
  sym_lower();
}

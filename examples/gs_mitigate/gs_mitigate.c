#include <stdio.h> // for kallsymlib... bug.
#include <assert.h>
#include <stdlib.h>
#include "LINF/sym_all.h"

// compiler fence.
/* asm ("" ::: "memory") */

int main(__attribute((unused))int argc, __attribute((unused))char *argv[]){
  // Assumes some mitigation is used.
  uint64_t symbiote_flags = SYM_ELEVATE_FLAG | SYM_INT_DISABLE_FLAG;

  uint64_t b_kern_gs;
  uint64_t a_kern_gs;

  uint64_t b_user_gs;
  uint64_t a_user_gs;


    sym_mode_shift( symbiote_flags );

    // Return to user mode with interrupts disabled
    // User gs is active, swap onto kern
    asm("rdgsbase %0" : "=rm"(b_user_gs) : : "memory" );
    asm("swapgs"); // get onto kern
    asm("rdgsbase %0" : "=rm"(b_kern_gs) : : "memory" );
    asm("swapgs"); // get onto user

    // Modify user gs_base
    /* asm("wrgsbase %0" :: "r" (&gsdata)); */

    asm("wrgsbase %0" : : "r"(b_kern_gs) : );


    asm("rdgsbase %0" : "=rm"(a_user_gs) : : "memory" );
    asm("swapgs"); // get onto kern
    asm("rdgsbase %0" : "=rm"(a_kern_gs) : : "memory" );
    asm("swapgs"); // get onto user

    sym_mode_shift(SYM_LOWER_FLAG);

    printf("B User gs is %lx\n", b_user_gs);
    printf("B Kern gs is %lx\n\n", b_kern_gs);

    printf("A User gs is %lx\n", a_user_gs);
    printf("A Kern gs is %lx\n\n", a_kern_gs);

    sym_mode_shift( symbiote_flags );
    asm("wrgsbase %0" : : "r"(b_user_gs) : );
    asm("rdgsbase %0" : "=rm"(a_user_gs) : : "memory" );
    sym_mode_shift(SYM_LOWER_FLAG);
    printf("A User gs is %lx\n", a_user_gs);

}

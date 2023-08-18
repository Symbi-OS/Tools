#include "page_table_util.h"
#include <LINF/sym_all.h>

int main() {
    /*
    Next example:
        - allocate an integer and set its value to 42 (global user variable PAGE ALIGNED! make sure nothing else is there)
        - create a temporary local (user stack) int variable and set it to the value of global variable
        - allocate a kernel page
        - change the PTE of global variable to point to the kernel page
        - write the temp int variable into the global
        - read from the kernel page address to see if it got copied :D

        *Notes* might need to be on the kernel stack
    */

    return 0;
}

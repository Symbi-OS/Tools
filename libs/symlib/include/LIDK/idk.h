#ifndef __IDK__
#define __IDK__
#include <stdint.h>
// TODO: Make this general.
#include "../../../kallsymlib/kallsymlib.h"
void * sym_get_fn_address(char *symbol);
extern void sym_l2_init();
#endif

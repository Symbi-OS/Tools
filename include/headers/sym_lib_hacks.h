#ifndef __SYM_LIB_HACKS__
#define __SYM_LIB_HACKS__

void sym_touch_every_page_text();

// Touch some stack pages
extern void sym_touch_stack();
#endif

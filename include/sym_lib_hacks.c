#include "./headers/sym_lib_hacks.h"

extern char __executable_start;
extern char __etext;

void sym_touch_every_page_text(){
  unsigned char *p;
  p = (unsigned char *) &__executable_start;

  unsigned char c;
  for(; p < (unsigned char *)&__etext; p+= (1<<12))
    c = *p;
}

#include <stdio.h>
#include "include/sym_lib.h"

int main(){
  sym_elevate();

  /* tommyu@don:~/Symbi-OS/linux$ nm vmlinux  | grep "T printk" */
  /*   ffffffff810c74fe T printk */
  void (*printk)(char*) = ( void(*)(char*) ) 0xffffffff810c74fe;
  // In the user addr space half.
  char *my_str = "We can printk from the elevated mode\n";
  (*printk)(my_str);

  sym_lower();
  return 0;
}

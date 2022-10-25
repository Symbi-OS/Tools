#include <stdio.h>
#include <unistd.h>
#include "../../Symlib/include/LINF/sym_all.h"
//#include "../../../Symlib/include/L0/sym_lib.h"

typedef void (*void_fn_ptr)(unsigned long);
typedef int (*ksys_write_t)(unsigned int fd, const char *buf, size_t count);
typedef int (*ksys_read_t)(unsigned int fd, const char *buf, size_t count);
ksys_write_t ksys_wr_sc;

void __attribute__ ((constructor)) init(void) {
  // function that is called when the library is loaded
  // printf("FUNC_ADDR_RET: %p\n", (void *)kallsyms_get_name("ksys_write")); 
  ksys_wr_sc = (ksys_write_t)sym_get_fn_address("ksys_write");
  sym_elevate();
}

void __attribute__ ((destructor)) cleanUp(void) {
  // function that is called when the library is »closed«.
  sym_lower();
}

ssize_t write(int fd, const void* buf, size_t len){
  // interpose shortcutted function
  return ksys_wr_sc(fd, buf, len);
}


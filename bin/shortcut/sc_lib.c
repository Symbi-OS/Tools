#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "../../Symlib/include/LINF/sym_all.h"
//#include "../../../Symlib/include/L0/sym_lib.h"

#define BUFSIZE 80

typedef void (*void_fn_ptr)(unsigned long);
typedef int (*ksys_write_t)(unsigned int fd, const char *buf, size_t count);
typedef int (*ksys_read_t)(unsigned int fd, const char *buf, size_t count);
ksys_write_t ksys_wr_sc;
ksys_read_t ksys_rd_sc;
uint32_t opt = 0;

void __attribute__ ((constructor)) init(void) {
  // function that is called when the library is loaded
  char buf[BUFSIZE];
  char* envvar = "SHORTCUT_OPTIONS";
  
  if(getenv(envvar)){
    if(snprintf(buf, BUFSIZE, "%s", getenv(envvar)) >= BUFSIZE){
      fprintf(stderr, "BUFSIZE OF %d TOO SMALL. ABORTING\n", BUFSIZE);
      exit(1);
    } else {
      opt = atoi(buf);
      ksys_wr_sc = (ksys_write_t)sym_get_fn_address("ksys_write");
      ksys_rd_sc = (ksys_read_t)sym_get_fn_address("ksys_read");
      sym_elevate();
    }
  }
}

void __attribute__ ((destructor)) cleanUp(void) {
  // function that is called when the library is »closed«.
  sym_lower();
}

ssize_t read(int fd, void* buf, size_t len){
  // interpose shortcutted function
  if(opt & 1)
    return ksys_rd_sc(fd, buf, len);
  else
    return syscall(SYS_read, fd, buf, len);
}

ssize_t write(int fd, const void* buf, size_t len){
  // interpose shortcutted function
  if(opt & 4)
    return ksys_wr_sc(fd, buf, len);
  else
    return syscall(SYS_write, fd, buf, len);
}

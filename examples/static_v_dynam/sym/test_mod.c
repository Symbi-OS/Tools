#include <fcntl.h>
#include <unistd.h>
#include "LINF/sym_all.h"

typedef int (*ksys_write_t)(unsigned int fd, const char *buf, size_t count);
typedef int (*ksys_read_t)(unsigned int fd, const char *buf, size_t count);

int main(){
  int i,fd;
  char* buf = "\0";
  fd = open("/dev/null", O_WRONLY);

  ksys_write_t ksys_wr_sc;
  ksys_read_t ksys_rd_sc;
  ksys_wr_sc = (ksys_write_t)sym_get_fn_address("ksys_write");
  ksys_rd_sc = (ksys_read_t)sym_get_fn_address("ksys_read");

  sym_elevate();
  for(i = 0; i < 20000000; i++){
    ksys_wr_sc(fd,buf,1);
  }
  for(i = 0; i < 20000000; i++){
    ksys_rd_sc(fd,buf,1);
  }
  sym_lower();
}

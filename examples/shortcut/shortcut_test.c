#include <fcntl.h>
#include <unistd.h>

int main(){
  int i,fd;
  char* buf = "\0";
  fd = open("/dev/null", O_WRONLY);

  for(i = 0; i < 10000000; i++){
    write(fd,buf,1);
  }
}

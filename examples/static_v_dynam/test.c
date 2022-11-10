#include <fcntl.h>
#include <unistd.h>

int main(){
  int i,fd;
  char* buf = "\0";
  fd = open("/dev/null", O_WRONLY);

  for(i = 0; i < 20000000; i++){
    write(fd,buf,1);
  }
  for(i = 0; i < 20000000; i++){
    read(fd,buf,1);
  }
}

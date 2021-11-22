#include <stdio.h>

#ifndef LINUX
#include "elevate.h"
#endif

#include <sys/types.h>
#include <unistd.h>
int main(int argc, char* argv[]) {

  printf("main pid is %d \n", getpid());


  int forkId = fork();
  if (forkId == 0) {
    printf("child process, id is %d\n", forkId);
  } else if (forkId > 0) {
    printf("parent process, id is %d\n", forkId);
  }
  return 0;
}

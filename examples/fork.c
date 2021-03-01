#include <stdio.h>
#include "elevate.h"
#include <sys/types.h>
#include <unistd.h>
int main(int argc, char* argv[]) {
  printf("Printing in kernel mode\n");
  int forkId = fork();
  if (forkId == 0) {
    printf("child process\n");
  } else if (forkId > 0) {
    printf("parent process\n");
  }
  return 0;
}

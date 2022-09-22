#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char** argv)
  {
    if (argc < 2)
    {
      printf("tell me how many times to loop please\n");
      return 1;
    }
    /* printf("top of the main to ya\n"); */
    int fd;
    long long count = atoi(argv[1]);
    fd = open("loop.txt", O_RDWR | O_CREAT, 0777);
    char * string = "deadbeef\n";
    for (int long long i=0; i < count; i++)
    {
      write(fd, string, 9);
    }
    /* printf("end of the main\n"); */
    return 0;
  }

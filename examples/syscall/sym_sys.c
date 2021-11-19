#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

int main(void) {
  int i;
  long j = 1;


  for(i=0; i < (j<<20); i++){
    syscall(448, 1);
    syscall(448, -1);
  }

 /* /\* Do elevate syscall with 0 for argument to check status *\/ */
 /* printf("are we elevated %d\n", syscall(448, 0)); */

 /* printf("request elevate %d\n", syscall(448, 1)); */

 /* asm("mov %cr3, %rdi"); */

 /* /\* Do elevate syscall with 0 for argument to check status *\/ */
 /* printf("are we elevated %d\n", syscall(448, 0)); */

 /* printf("request lowering %d\n", syscall(448, -1)); */

 /* /\* Do elevate syscall with 0 for argument to check status *\/ */
 /* printf("are we elevated %d\n", syscall(448, 0)); */

 printf("Done!");
  return 0;
}

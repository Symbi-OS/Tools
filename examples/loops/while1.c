#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int ctr = 0;

void do_work(int alpha){
  /* printf("ctr is %d, alpha is %d\n",ctr, alpha); */

  int reps = 1 << alpha;
  /* int reps = (alpha - 4 >= 0) ? 1 << (alpha - 4) : 1; */

  /* printf("ctr reps is %d, will execute %d nops\n", ctr, ctr*16); */

  // Trying to get some unrolling here.
  while (reps--) {
    asm("nop");
    // 1 =  0.82 ins/cyc at ./while1 17 10

    asm("nop");
    // 2 =  1.01 ins/cyc at ./while1 17 10

    asm("nop");
    asm("nop");
    // 4 = 1.31 ins/cyc at ./while1 17 10

    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    // 8 = 2.14 ins/cyc at ./while1 17 10

    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
    // 16 = 3.49 ins/cyc at ./while1 15 10

    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    // 32 = 3.70 ins/cyc at ./while1 15 10

    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    // 64 = 3.83 ins/cyc at ./while1 15 10

    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    // 128 = 3.91 ins/cyc at ./while1 15 10

    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    /* asm("nop"); */
    //256 = 3.96 ins/cyc at ./while1 15 10
  }
}

int main(int argc, char *argv[]){

  if (argc !=3) {
    printf("%d args given, not 3\n", argc);
    printf("expected ./while1 <app_work_pwr2> <system_work_pwr2>\n");
    exit(1);
  }

  int alpha = atoi(argv[1]);
  int beta = atoi(argv[2]);

  printf("alpha is %d beta is %d\n", alpha, beta);

  printf("\nNOPs:\n");
  printf("16 * %d nops will run each loop for a total of %d\n", 1<<alpha, 1<<(alpha + 4));
  if (alpha < 4) {
    printf("WARNING: You asked for %d nops per loop, but we're doing 16, sorry\n",
           1 << alpha);
  }

  printf("\nSYSCALLS:\n");
  printf("1 system call will be made every %d loops\n", 1 << beta);

  ctr = 0;

  // Try to keep total runtime aprox equal.
  /* int kill_ctr = 1 << (25 - ( (4 * alpha) / 5)); */
  int kill_ctr = 1<<13;
  /* int kill_ctr = 1 << 23; */

  /* kill = 23 */
  /* alpha = 4 */

	while(++ctr) {
    /* printf("\nouter loop, ctr is %d\n", ctr); */
    do_work(alpha);
    // Periodically relinquish core to os.

		if( ( ctr % (1 << beta) ) == 0 ){
      /* printf("doing system work ctr is %d\n", ctr); */
      getpid();
      ctr = 0;
    }

    if (kill_ctr-- == 0){
      exit(0);
    }

	}
}

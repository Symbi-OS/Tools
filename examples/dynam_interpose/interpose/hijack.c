#include <stdlib.h>
#include <stdio.h>
#include "helper.h"

#define BUFSIZE 80

// Show example reading envt var
void __attribute__ ((constructor)) setup(void) {
  char buf[BUFSIZE];
  char *envvar = "MY_ENV_VAR";

  // Make sure envar actually exists
  if(!getenv(envvar)){
    fprintf(stderr, "The environment variable %s was not found.\n", envvar);
    fprintf(stderr, "So I won't touch the flag\n");
  } else {
    if(snprintf(buf, BUFSIZE, "%s", getenv(envvar)) >= BUFSIZE){
      fprintf(stderr, "BUFSIZE of %d was too small. Aborting\n", BUFSIZE);
      exit(1);
    }
    printf("MY_ENV_VAR: %s\n", buf);
    fprintf(stderr, "So I will set the flag\n");

  }


}


void expected(){
  printf("in hijacked lib, expectations upended\n");
  helper();
}

#include <stdio.h>
#include "include/sym_lib.h"

/* Apply the constructor attribute to startupfun() so that it is executed before main() */
void startupfun(void) __attribute__((constructor));

/* Apply the destructor attribute to cleanupfun() so that it is executed after
 * main() */
void cleanupfun(void) __attribute__((destructor));

void startupfun(void) {
  sym_elevate();
  printf("Now elevated\n");
}

void cleanupfun(void) {
  sym_lower();
  printf("Now lowered\n");
}

int main() { printf("Main fn\n"); }

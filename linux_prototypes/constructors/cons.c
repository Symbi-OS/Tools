#include <stdio.h>

// Assigning functions to be executed before and
// after main()
void __attribute__((constructor)) calledFirst();
void __attribute__((destructor)) calledLast();

// This function is assigned to execute before
// main using __attribute__((constructor))
void calledFirst() { printf("I am called first\n"); }

// This function is assigned to execute after
// main using __attribute__((destructor))
void calledLast() { printf("I am called last\n"); }

void main() {
  printf("I am in main\n");
}


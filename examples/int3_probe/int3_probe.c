#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "../../include/headers/sym_all.h"

#include "../../include/kallsymlib/kallsymlib.h"
// Our version of the idt. Not sure about alignment.
unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

uint64_t get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
    while(1);
  }
  return info->addr;
}

void init_kallsym(){
  char *path = "./System.map";
  if (path) {
    // alternative path specified for kallsyms file
    // manually initialize library with this path
    if (!kallsymlib_init(path)) {
      fprintf(stderr, "ERROR: kallsymlib failed to initialize\n");
      exit(-1);
    }
  }
}
void interpose_on_int3_ft(){
  // Copy the system idt to userspace
  sym_copy_system_idt(my_idt);

  sym_interpose_on_int3_ft(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

void show_int_interposition_works(){
  // Elevation is used somewhat carefully here.

  uint64_t addr__do_sys_getpid = get_fn_address("__do_sys_getpid");
  printf("the fn lives at %llx\n", addr__do_sys_getpid);

  interpose_on_int3_ft();

  sym_elevate();
  printf("Before modification 1st byte getpid is %#x \n", *((unsigned char *) addr__do_sys_getpid));
  sym_lower();

  sym_set_probe(addr__do_sys_getpid);

  sym_elevate();
  printf("After inserting int3, 1st byte of getpid is %#x \n", *((unsigned char *) addr__do_sys_getpid));
  sym_lower();

  // This invocation will result in triggering the 
  getpid();

  /* sym_make_pg_unwritable(addr__do_sys_getpid); */

  sym_elevate();
  printf("After encountering and running handler, 1st byte of getpid is restored %#x \n", *((unsigned char *) addr__do_sys_getpid));
  sym_lower();

  // Totally normal invocation
  getpid();

  sym_elevate();
  printf("once more for fun, 1st byte of getpid is restored %#x \n", *((unsigned char *) addr__do_sys_getpid));
  sym_lower();
}



int main(){
  init_kallsym();

  printf("Starting main\n");

  // Store system idtr here for later restoration.
  struct dtr system_idtr;
  sym_store_idt_desc(&system_idtr);

  // Where all the real work happens.
  show_int_interposition_works();

  kallsymlib_cleanup();

  // Swing back onto system idtr before exit
  sym_load_idtr(&system_idtr);

  // Make sure we lower.
  if(sym_check_elevate())
    sym_lower();

  printf("Done main\n");
}

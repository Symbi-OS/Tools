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

  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt);{
    printf("copying the idt for int3\n");
      // Copy the system idt to userspace if we haven't already.
      sym_copy_system_idt(my_idt);
  }

  sym_interpose_on_int3_ft_c(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}


void check_on_probe(uint64_t addr){
  sym_elevate();
  printf("1st byte getpid is %#x \n", *((unsigned char *) addr));
  sym_lower();
}

void show_int_interposition_works(){
  // Elevation is used somewhat carefully here.

  uint64_t addr__do_sys_getpid = get_fn_address("__do_sys_getpid");
  printf("the fn lives at %llx\n", addr__do_sys_getpid);

  interpose_on_int3_ft();

  check_on_probe(addr__do_sys_getpid);

  // Inject probe on first byte of fn.
  sym_set_probe(addr__do_sys_getpid);

  check_on_probe(addr__do_sys_getpid);

  // This invocation will result in triggering the
  sym_make_pg_unwritable(addr__do_sys_getpid);

  // NOTE: Hmm, shouldn't this fault as text is not writable?
  getpid();

  check_on_probe(addr__do_sys_getpid);

  // Totally normal invocation
  getpid();

  check_on_probe(addr__do_sys_getpid);
}

extern uint64_t cr3_reg;

// Store system idtr here for later restoration.
struct dtr system_idtr;

void setup(){
  printf("Starting setup\n");

  sym_elevate();
  asm("movq %%cr3,%0" : "=r"(cr3_reg));
  sym_lower();

  init_kallsym();

  sym_store_idt_desc(&system_idtr);
}

void cleanup(){
  kallsymlib_cleanup();

  // Swing back onto system idtr before exit
  // XXX why are you broken?
  sym_load_idtr(&system_idtr);

  // Make sure we lower.
  if(sym_check_elevate()){
    printf("Didn't expect to be elevated\n");
    sym_lower();
  }
}

int main(){
  printf("Starting main\n");
  printf("Don't really get why not preparing the stack or text still works\n");

  setup();

  // Where all the real work happens.
  show_int_interposition_works();

  cleanup();

  printf("Done main\n");
}

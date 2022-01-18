#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "LINF/sym_all.h"

#include "../../libs/kallsymlib/kallsymlib.h"

// Some colors for printf
#define RESET "\033[0m"
#define RED "\033[31m"     /* Red */
#define GREEN "\033[32m"   /* Green */
#define YELLOW "\033[33m"  /* Yellow */
#define BLUE "\033[34m"    /* Blue */
#define MAGENTA "\033[35m" /* Magenta */
#define CYAN "\033[36m"    /* Cyan */

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
  char *path = "/boot/System.map-5.14.0-symbiote+";
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

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for int3\n");
      // Copy the system idt to userspace if we haven't already.
      sym_copy_system_idt(my_idt);
  } else{
    printf("no copy made int3\n");
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

extern uint64_t cr3_reg;
// Store system idtr here for later restoration.
struct dtr system_idtr;
void show_int_interposition_works(){
  // Elevation is used somewhat carefully here.

  uint64_t addr__do_sys_getpid = get_fn_address("__do_sys_getpid");
  interpose_on_int3_ft();

  check_on_probe(addr__do_sys_getpid);

  // Inject probe on first byte of fn.
  sym_set_probe(addr__do_sys_getpid);

  check_on_probe(addr__do_sys_getpid);

  // This invocation will result in triggering the
  /* sym_make_pg_unwritable(addr__do_sys_getpid); */

  // NOTE: Hmm, shouldn't this fault as text is not writable?
  getpid();

  check_on_probe(addr__do_sys_getpid);

  // Totally normal invocation
  getpid();

  check_on_probe(addr__do_sys_getpid);
}


void interpose_on_pg_ft(){
  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for pf\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(my_idt);
  } else{
    printf("no copy made pg_ft\n");
  }

  sym_interpose_on_pg_ft_c(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

void setup(){
  printf("Starting setup\n");

  sym_elevate();
  asm("movq %%cr3,%0" : "=r"(cr3_reg));
  sym_lower();

  init_kallsym();

  sym_store_idt_desc(&system_idtr);

  interpose_on_pg_ft();
}

void cleanup(){
  kallsymlib_cleanup();

  // Swing back onto system idtr before exit
  sym_load_idtr(&system_idtr);

  // Make sure we lower.
  if(sym_check_elevate()){
    printf("Didn't expect to be elevated\n");
    sym_lower();
  }
}

int main(){
  printf("Starting main\n");
  sym_lib_init();

  setup();

  // Where all the real work happens.
  show_int_interposition_works();

  cleanup();

  printf("Done main\n");
}

#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>

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
  char l[80];
  int f=open("/home/sym/Symbi-OS/Apps/examples/int3_probe/test",O_RDONLY);
  printf("l lives at %p\n", l);
  int n;

  // Elevation is used somewhat carefully here.

  /* uint64_t addr__do_sys_getpid = get_fn_address("__do_sys_getpid"); */
  uint64_t addr__do_sys_getpid = get_fn_address("ksys_read");
  interpose_on_int3_ft();

  check_on_probe(addr__do_sys_getpid);

  // Inject probe on first byte of fn.
  sym_set_probe(addr__do_sys_getpid);

  check_on_probe(addr__do_sys_getpid);

  // This invocation will result in triggering the
  /* sym_make_pg_unwritable(addr__do_sys_getpid); */

  // NOTE: Hmm, shouldn't this fault as text is not writable?


  n=read(f,l,80);
  /* getpid(); */

  check_on_probe(addr__do_sys_getpid);

  // Totally normal invocation
  /* getpid(); */
  n=read(f,l,80);

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

extern uint64_t int3_count;
extern uint64_t int3_rdi;
extern uint64_t int3_rsi;
extern uint64_t int3_rdx;
int main(){
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  printf("Starting main\n");

  printf("int3_count:%#lx\n",int3_count );
  printf("int3_rdi  :%#lx\n",int3_rdi   );
  printf("int3_rsi  :%#lx\n",int3_rsi   );
  printf("int3_rdx  :%#lx\n",int3_rdx   );


  sym_lib_init();

  /* sym_mitigate_pf(); */

  // Where all the real work happens.
  show_int_interposition_works();

  /* sym_mitigate_pf_cleanup(); */

  printf("int3_count:%#lx\n",int3_count );
  printf("int3_rdi  :%#lx\n",int3_rdi   );
  printf("int3_rsi  :%#lx\n",int3_rsi   );
  printf("int3_rdx  :%#lx\n",int3_rdx   );

  printf("Done main\n");


  sym_set_idtr((unsigned long)check_idtr.base, IDT_SZ_BYTES - 1);
}

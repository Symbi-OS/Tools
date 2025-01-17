#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>

#include "LINF/sym_all.h"

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

void interpose_on_int3_ft(){

  sym_copy_system_idt(my_idt);

  sym_interpose_on_int3_ft_c(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);

  fprintf(stderr, "idtr set\n");
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
  /* char l[80]; */
  /* int f=open("/home/sym/Symbi-OS/Apps/examples/int3_probe/test",O_RDONLY); */
  /* printf("l lives at %p\n", l); */
  /* int n; */

  // Elevation is used somewhat carefully here.

  uint64_t addr__do_sys_getpid = (uint64_t)sym_get_fn_address("__do_sys_getpid");
  interpose_on_int3_ft();

  check_on_probe(addr__do_sys_getpid);

  // Inject probe on first byte of fn.
  sym_set_probe(addr__do_sys_getpid);

  while(1)
    getpid();


  check_on_probe(addr__do_sys_getpid);



  // This invocation will result in triggering the
  /* sym_make_pg_unwritable(addr__do_sys_getpid); */

  // NOTE: Hmm, shouldn't this fault as text is not writable?


  /* n=read(f,l,80); */
  getpid();

  check_on_probe(addr__do_sys_getpid);

  // Totally normal invocation
  getpid();
  /* n=read(f,l,80); */

  check_on_probe(addr__do_sys_getpid);
}

#if 0
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
#endif

extern uint64_t int3_count;
extern uint64_t int3_rdi;
extern uint64_t int3_rsi;
extern uint64_t int3_rdx;
int main(){
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);
  printf("system idt is%ld\n", check_idtr.base);

  printf("Starting main\n");

  printf("int3_count:%#lx\n",int3_count );
  printf("int3_rdi  :%#lx\n",int3_rdi   );
  printf("int3_rsi  :%#lx\n",int3_rsi   );
  printf("int3_rdx  :%#lx\n",int3_rdx   );


  sym_lib_init();

  /* sym_mitigate_pf(); */

  // Where all the real work happens.
  show_int_interposition_works();
  struct dtr interposer_idtr;
  sym_store_idt_desc(&interposer_idtr);
  printf("interposer idt is %lx\n", interposer_idtr.base);

  /* sym_mitigate_pf_cleanup(); */

  printf("int3_count:%#lx\n",int3_count );
  printf("int3_rdi  :%#lx\n",int3_rdi   );
  printf("int3_rsi  :%#lx\n",int3_rsi   );
  printf("int3_rdx  :%#lx\n",int3_rdx   );

  printf("Done main\n");


  sym_set_idtr((unsigned long)check_idtr.base, IDT_SZ_BYTES - 1);
  struct dtr exit_idtr;
  sym_store_idt_desc(&exit_idtr);
  printf("exit idt is %lx\n", exit_idtr.base);
  while(1)
    getpid();

}

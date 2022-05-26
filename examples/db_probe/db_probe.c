#include <stdlib.h>
#include <stdio.h>
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

void interpose_on_db_ft(){

  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for debug\n");
      // Copy the system idt to userspace if we haven't already.
      sym_copy_system_idt(my_idt);
  } else{
    printf("no copy made debug\n");
  }

  sym_interpose_on_db_ft_c(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

int func(){
	printf("PERFORMING ARBITRARY TASK\n");
	int x = 1000000;
	int y = 0, i = 0;
	for(; i < x; i++)
		y+=i;
	return y;
}

int main() {

  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  sym_lib_init();
  sym_probe_init();
  interpose_on_db_ft();
  sym_set_db_probe((uint64_t)&func);
  
  printf("SET TRIGGER TO: %p\n", &func); 

  uint64_t addr;
  sym_elevate();
  asm("\t mov %%db0,%0" : "=rm"(addr));
  sym_lower();
  printf("DR0: %p\n", (void *)addr);

  func();

  printf("DONE MAIN\n");
  sym_set_idtr((unsigned long)check_idtr.base, IDT_SZ_BYTES - 1);
}

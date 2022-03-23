#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "../libs/symlib/include/LINF/sym_all.h"

enum pg_level {
  MOD_IST_ENABLE,
  MOD_IST_DISABLE
};
#define eprintf(...) fprintf (stderr, __VA_ARGS__)

void help(){
  eprintf("./idt_tool hi");
  eprintf("options: \n");
  eprintf("\t-h help: this help menu\n");
  eprintf("\t-g get current idt:  print current idt \n");

  eprintf("given no a, will be relative to current\n");
  /* eprintf("\t-i idt:  print current idt \n"); */

  /* eprintf("\nparameters\n"); */
}

void get_current_idtr(struct dtr * idt){
  sym_store_idt_desc(idt);
}

void print_idtr(){
  struct dtr my_idt;
  get_current_idtr(&my_idt);

  printf("IDT base:  %lx\n", my_idt.base);
  printf("IDT limit: %#x\n", my_idt.limit);
}
void print_desc(struct dtr *idt, int vector){
  assert(idt->base != 0);
  assert(vector >= 0);

  sym_print_idt_desc((unsigned char *) idt->base, (unsigned int)vector);
}

typedef void (*void_fn_ptr)(unsigned long);
void_fn_ptr get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
  }
  return (void_fn_ptr) info->addr;
}

typedef void * (*vzalloc_t)(unsigned long);
typedef void (*kvfree_t)(void *);

void * get_aligned_kern_pg(){

  // TODO: is there a right way to do this?
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
  vzalloc_t vzalloc = (vzalloc_t) get_fn_address("vzalloc");
#pragma GCC diagnostic pop 

  sym_elevate();
  void * p = vzalloc(IDT_SZ_BYTES);
  // Mem leak need this to be page aligned 
  sym_lower();

  /* printf("got kern page ptr %p\n", p); */
  assert( ((long unsigned )p % IDT_SZ_BYTES ) == 0);
  return p;
}

void * copy_idt(struct dtr* src_idt){
  // Allocate kern page
  void *idt_cp = get_aligned_kern_pg();

  // Copy idt
  sym_memcpy(idt_cp, (void *) src_idt->base, IDT_SZ_BYTES );

  // Return pointer to page
  return idt_cp;
}

void * modify_idt(struct dtr *idt, int vector, unsigned int mod_option){
  assert(idt != NULL);
  assert(vector >= 0);

  if(mod_option == MOD_IST_ENABLE){
    unsigned int enable = 1;
    sym_toggle_pg_ft_ist((unsigned char *)idt->base, enable);
  }else if(mod_option == MOD_IST_DISABLE){
    unsigned int disable = 0;
    sym_toggle_pg_ft_ist((unsigned char *)idt->base, disable);
  }
  return (void *)idt->base;
}

void install_idt(struct dtr *idt){
  sym_load_idtr(idt);
}

int main(int argc, char *argv[]){
  sym_lib_init();
  /* sym_touch_every_page_text(); */

  /* copy_and_install_idk_kern_page(); */
  // NOTE assumes the existance of at least idt_maps/sys_default.idt
  struct dtr idt;

  // User supplied idt
  void * input_idt = NULL;

  int vector = -1;
  bool print_flag = 0;
  bool cp_flag = 0;
  bool install_flag = 0;

  bool mod_flag = 0;
  unsigned int mod_option;

  int c;
  // Ugly, has to be in order {d e p c} {n, v } {i, s}
  // Or something like that.
  while ((c = getopt (argc, argv, "a:cdghilm:pv:")) != -1)
    switch (c)
      {
      case 'a':
        // Address
        input_idt = (void *) strtoull(optarg, NULL, 16) ;
        break;
      case 'c':
        // Copy
        cp_flag = 1;
        break;
      case 'd':
        // Delete
        break;
      case 'g':
        // get idtr
        /* printf("%lx\n", get_current_idtr(&current_dtr)); */
        get_current_idtr(&idt);
        printf("%lx\n", idt.base);
        return 0;
      case 'h':
        // help
        help();
        return 0;
        break;
      case 'i':
        // Install
        install_flag = 1;
        break;
      case 'l':
        // List maybe only for shell script
        break;
      case 'm':
        mod_flag = 1;
        // Modify
        if(!strcmp(optarg, "ist_enable")){
          mod_option = MOD_IST_ENABLE;
        }else if (!strcmp(optarg, "ist_disable")){
          mod_option = MOD_IST_DISABLE;
        }else {
          eprintf("Don't know that mod option\n");
          return -1;
        }
        break;
      case 'p':
        // Print
        print_flag = 1;
        break;
      case 'v':
        // Entry

        vector = strtoull(optarg, NULL, 10) ;
        // Specify offset in dec
        /* int vector = strtoull(optarg, NULL, 10); */
        /* d_print_desc(vector); */
        break;

      case '?':
        if (optopt == 'c')
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        /* else if (isprint (optopt)) */
        /*   fprintf (stderr, "Unknown option `-%c'.\n", optopt); */
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }

  // Printer.
  if(print_flag && (vector != -1)){
    // If no a flag, use current, otherwise use
    if(input_idt){
      idt.base= (uint64_t) input_idt;
      idt.limit = IDT_SZ_BYTES - 1;
    }else{
      // No user supplied idt
      get_current_idtr(&idt);
    }
    print_desc(&idt, vector);
  }

  void * copied_idt = NULL;
  // Copier
  if(cp_flag){
    // If no a flag, use current, otherwise use
    if(input_idt){
      idt.base= (uint64_t) input_idt;
      idt.limit = IDT_SZ_BYTES - 1;
    }else{
      // No user supplied idt
      get_current_idtr(&idt);
    }
    copied_idt = copy_idt(&idt);
    printf("copied_idt %p\n", copied_idt);
  }

  if(mod_flag && (vector != -1)){
    // If no a flag, use current, otherwise use
    if(input_idt){
      idt.base= (uint64_t) input_idt;
      idt.limit = IDT_SZ_BYTES - 1;
    }else{
      // No user supplied idt
      get_current_idtr(&idt);
    }
    modify_idt(&idt, vector, mod_option);
  }

  if(install_flag){
    struct dtr old_idt;
    get_current_idtr(&old_idt);

    // If no a flag, use current, otherwise use
    if(input_idt){
      idt.base= (uint64_t) input_idt;
      idt.limit = IDT_SZ_BYTES - 1;
    }else{
      // No user supplied idt
      idt = old_idt;
      /* get_current_idtr(&idt); */
    }
    install_idt(&idt);
    printf("%lx\n", old_idt.base);
  }

}

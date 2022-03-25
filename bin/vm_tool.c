#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "../libs/symlib/include/LINF/sym_all.h"
#include "vm_tool.h"

void_fn_ptr get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
  }
  return (void_fn_ptr) info->addr;
}

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
void help(){
  printf("Help NYI");
}
void parse_args(int argc, char *argv[], struct params *p){
  int c;
  // Ugly, has to be in order {d e p c} {n, v } {i, s}
  // Or something like that.
  while ((c = getopt (argc, argv, "a:hp")) != -1)
    switch (c)
      {
      case 'a':
        // Address
        p->input_addr = (void *) strtoull(optarg, NULL, 16) ;
        break;
      case 'h':
        // help
        help();
        exit(0);
        break;
      case 'p':
        // Print
        p->print_flag = 1;
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
        exit(1);
      default:
        abort ();
      }
}
void init_params(struct params *p){
  memset(p, 0, sizeof(struct params));
}
int main(int argc, char *argv[]){
  sym_lib_init();
  // Don't operate on this directly, use pointer
  struct params params;

  // For consistency, always operate on pointer
  struct params *p = &params;

  init_params(p);
  parse_args(argc, argv, p);

  // PTE Printer
  if(p->print_flag && p->input_addr){
    printf("Supposed to print pte\n");

    unsigned int level;
    struct pte *pte = sym_get_pte((uint64_t) p->input_addr, &level);
    sym_print_pte(pte);
    return 0;
  }

  return 0;
}

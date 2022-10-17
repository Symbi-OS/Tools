#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "../../Symlib/include/LINF/sym_all.h"

#include "cr_tool.h"

#define eprintf(...) fprintf (stderr, __VA_ARGS__)

void help(){
  eprintf("cr_tool\n");
  eprintf("This tool is built to run before other mitigations.\n");
  eprintf("It should be able to prepare cr4 possibly by removing\n");
  eprintf("SMEP and SMAP protections.\n");
  eprintf("options:\n");
  eprintf("\tg:         get current cr4\n");
  eprintf("\ts: <val>:  set cr4\n");
  eprintf("\th:         print this help msg\n");
}

void toggle_smep(){
  eprintf("nyi\n");
}

void toggle_smap(){}

void parse_args(int argc, char *argv[], struct params *p){
  int c;
  // Ugly, has to be in order {d e p c} {n, v } {i, s}
  // Or something like that.
  while ((c = getopt (argc, argv, "deghs:")) != -1)
    switch (c)
      {
      case 'd':
        // Address
        p->disable_smap_smep = true;
        break;
      case 'e':
        // Address
        eprintf("enable smap smep case\n");
        p->enable_smap_smep = true;
        break;
      case 'g':
        // Address
        eprintf("get case\n");
        p->get_cr4_flag = true;
        break;
      case 's':
        // Address
        eprintf("set case\n");
        p->set_cr4_flag = true;
        p->set_val = strtoull(optarg, NULL, 16) ;
        break;
      case 'h':
        // help
        help();
        exit(0);
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
void init_params(struct params * p){
  // Zero struct
  memset(p, 0, sizeof(struct params));
}

uint64_t get_cr4(){
  assert(false);
  uint64_t cr4;
  /* sym_elevate(); */
  /* sym_syscall_flags_get_gs(SYM_ELEVATE_FLAG | SYM_INT_DISABLE_FLAG */
  /*                          | SYM_TOGGLE_SMEP_FLAG | SYM_TOGGLE_SMAP_FLAG */
  /*                          | SYM_NOSMEP_FLAG | SYM_NOSMAP_FLAG); */
  asm volatile ("mov %%cr4, %0" : "=r"(cr4) : : "memory");

  // Implicitly flipping SMEP and SMAP protections back on.
  sym_mode_shift(SYM_LOWER_FLAG | SYM_TOGGLE_SMEP_FLAG | SYM_TOGGLE_SMAP_FLAG);

  return cr4;
}

void set_cr4(uint64_t val){
  sym_elevate();
	asm volatile("mov %0,%%cr4": "+r" (val) : : "memory");
  sym_lower();
}

int main(int argc, char *argv[]){ 
  // Don't operate on this directly, use pointer
  struct params params;

  // For consistency, always operate on pointer
  struct params *p = &params;

  init_params(p);
  parse_args(argc, argv, p);

  if(p->get_cr4_flag && p->set_cr4_flag){
    eprintf("Can't both get and set\n");
    exit(-1);
  }
  if(p->disable_smap_smep && p->enable_smap_smep){
    eprintf("Can't both disable and enable\n");
    exit(-1);
  }

  if(p->get_cr4_flag){
    eprintf("about to get cr4\n");
    uint64_t ret = get_cr4();
    printf("%#lx\n", ret);
    exit(0);
  }

  if(p->set_cr4_flag){
    /* set_cr4(p->set_val); */
    uint64_t ret = get_cr4();
    eprintf("cr4 is %#lx\n", ret);
    eprintf("set it to %#lx\n", p->set_val);
    eprintf("NYI\n");
  }

  if(p->enable_smap_smep){
    eprintf("Enable smap and smep\n");
    // Implies no smap / smep
    sym_mode_shift(SYM_TOGGLE_SMEP_FLAG | SYM_TOGGLE_SMAP_FLAG);
    exit(0);
  }

  if(p->disable_smap_smep){
    eprintf("Disable smap and smep\n");
    sym_mode_shift(SYM_TOGGLE_SMEP_FLAG | SYM_TOGGLE_SMAP_FLAG
                   | SYM_NOSMEP_FLAG | SYM_NOSMAP_FLAG);
    exit(0);
  }

  /* if(p->modify_nosmep_flag){ */
  /*   toggle_smap(); */
  /* } */

  /* if(p->modify_nosmap_flag){ */
  /*   toggle_smep(); */
  /* } */

}

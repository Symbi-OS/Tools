#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "syscall_tool.h"
#include "../libs/symlib/include/LINF/sym_all.h"

void sym_flush_tlb(){
  // XXX TEST ME
  sym_elevate();
  __asm__("movq %%cr3, %%rax" :: : "%rax");
  __asm__("movq %rax, %cr3"); // does the flush
  sym_lower();
}

void sym_toggle_page_exe_disable(void * addr, bool disable){
  // When set, XD bit in PTE makes referenced page
  // execute disabled.

  // TODO: We think this only works for kern addrs.
  {
    uint64_t tmp = (uint64_t) addr;
    assert((tmp >> 63) == 1);
  }

  // make it executable
  unsigned int level;
  struct pte *pte = sym_get_pte((uint64_t) addr, &level);

  sym_elevate(); pte->XD = disable; sym_lower();

  // TODO Still need to invalidate
  sym_flush_tlb();
}

void * get_aligned_kern_pg(){

  // TODO: is there a right way to do this?
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
  vzalloc_t vzalloc = (vzalloc_t) sym_get_fn_address("vzalloc");
#pragma GCC diagnostic pop

  sym_elevate();
  void * p = vzalloc(PG_SZ);
  sym_lower();

  assert( ((long unsigned )p % PG_SZ) == 0);
  return p;
}

void parse_args(int argc, char *argv[], struct params *p){

  int c;
  while ((c = getopt (argc, argv, "gm:i")) != -1)
    switch (c)
      {
      case 'g':
        p->get_flag = 1;
        break;
      case 'i':
        p->insert_flag = 1;
        break;
      case 'm':
        p->mod_flag = 1;
        if(! strncmp(optarg, "addr:", 5)){
          // If first 5 chars are "addr:"
          char *c = optarg;
          c += 5;
          p->mod_addr = (void *) strtoull(c, NULL, 16);
        }else {
          printf("Don't know that mod option\n");
          exit(-1);
        }
        break;
      default:
        abort ();
      }
}

void* read_lstar(){
  sym_elevate();
  void* lstar_addr;
  PUSH_REGS
  asm (
       "movl	$0xc0000082,%%ecx;"
       "rdmsr;"
       "shl	$0x20,%%rdx;"
       "or	%%rax,%%rdx;"
       "movq	%%rdx,%0;"
       :"=r"(lstar_addr)
       :
       :);
  POP_REGS
  sym_lower();

  printf("address read from LSTAR is %p\n", lstar_addr);
  return lstar_addr;
}

void write_lstar(struct params *p){
  sym_elevate();
  uint64_t new_addr = (uint64_t) p->mod_addr;
  uint64_t lower = (new_addr & 0xFFFFFFFF);
  uint64_t upper = ((new_addr >>  32) & 0xFFFFFFFF);
  PUSH_REGS
  asm (
       "movq	%0,%%rax;"
       "movq	%1,%%rdx;"
       "movl	$0xc0000082,%%ecx;"
       "wrmsr;"
       :
       :"r"(lower), "r"(upper)
       :);
  POP_REGS
  sym_lower();
  printf("wrote %lx to LSTAR\n",new_addr);
}

#define CHECK_SYSCALL_NO                        \
	asm (                                         \
       ".global _interposer;"                   \
       "_interposer:	pushfq;"                  \
       "pushq	%rax;"                            \
       "pushq	%rdi;"                            \
       "cmp	$0x1c0,%rax;"                       \
       "jne	_bail;"                             \
       );

#define CHECK_FAST_ELEVATE	\
  asm (                     \
       "movq	%rdi,%rax;"   \
       "and	$0x200,%eax;"   \
       "test	%rax,%rax;"   \
       "je		_bail;"       \
       );
/*check that interrupts are actually disabled*/
#define SCALPEL                                 \
  asm (                                         \
       "pop		%rdi;"                            \
       "pop		%rax;"                            \
       "popfq;"                                 \
       "pushq	%rcx;"                            \
       "ret;"                                   \
       );

#define JMP_SYSCALL                             \
  asm (                                         \
       ".global _bail;"                         \
       "_bail: pop	%rdi;"                      \
       "pop		%rax;"                            \
       "popfq;"                                 \
       "pushq	%rax;"                            \
       "pushfq;"                                \
       "movq	$0xffffffff81e00000,%rax;"        \
       "xorq	%rax,0x8(%rsp);"                  \
       "xorq	0x8(%rsp),%rax;"                  \
       "xorq	%rax,0x8(%rsp);"                  \
       "popfq;"                                 \
       "ret;"                                   \
       );

#define MY_INTERPOSER	\
  CHECK_SYSCALL_NO	\
  CHECK_FAST_ELEVATE	\
  SCALPEL	\
  JMP_SYSCALL

MY_INTERPOSER

/* long myfunc(register long syscallno, register long fastelevate){ */
/*   if (syscallno == 448){ */
/*     if (fastelevate & (0b1 << 9)){ */
/*       syscallno = 42; */
/*       return syscallno; */
/*     } */
/*   } */
/*   syscallno = 666; */
/*   return syscallno; */
/* } */

void* interpose_syscalls(struct params *p){
  void* page = (void*)get_aligned_kern_pg();
  void * src = NULL;
  int sz = 0;
  src = &_interposer;
  sz = PG_SZ;

  assert(src != NULL);
  assert(sz != 0);
  assert(sz <= (int) PG_SZ);

  sym_memcpy(page, src, sz);

  int disable = 0;
  sym_toggle_page_exe_disable(page, disable);

  read_lstar();
  p->mod_addr = page;
  write_lstar(p);
  return page;
}

void init_params(struct params * p){
  memset(p, 0, sizeof(struct params));
}

int main(int argc, char *argv[]){
  sym_lib_init();

  struct params params;
  struct params *p = &params;
  init_params(p);

  parse_args(argc, argv, p);

  if(p->get_flag){
    read_lstar();
    return 0;
  }

  if(p->insert_flag){
    interpose_syscalls(p);
    return 0;
  }

  if(p->mod_flag && (p->mod_addr != 0)){
    write_lstar(p);
    return 0;
  }
}

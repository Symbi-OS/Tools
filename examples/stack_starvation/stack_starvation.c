#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>

#include "LINF/sym_all.h"

// when we add support for kernel source get this
// from arch/x86/include/asm/pgtable_types.h
enum pg_level {
  PG_LEVEL_NONE,
  PG_LEVEL_4K,
  PG_LEVEL_2M,
  PG_LEVEL_1G,
  PG_LEVEL_512G,
  PG_LEVEL_NUM
};

typedef void (*void_fn_ptr)(unsigned long);
void_fn_ptr get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
  }
  return (void_fn_ptr) info->addr;
}

// Our version of the idt. Not sure about alignment.
unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

unsigned char *kern_pg_for_idt = NULL;
unsigned char *kern_pg_for_df_handler = NULL;

// Store system idtr here for later restoration.
struct dtr system_idtr;

void push_a_lot(){
  /* uint64_t my_rsp; */
  /* asm("movq %"); */
  // HACK don't know why seg faults without register usage.
  register int count = 1 << 13;
  /* printf("pushing a lot \n"); */
  for(register int i=0; i< count; i++){
    __asm__("pushq $42":::"memory");
  }
  /* printf("pushing a lot \n"); */
  for(register int i=0; i< count; i++){
    __asm__("popq %%rax": /*no*/: /*no*/ : "rax");
  }
  /* printf("pushing a lot \n"); */

}

void show_process_stk_ft_works(){
  /* sym_touch_stack(); */
  push_a_lot();
}


void show_naive_elevation_DFs(){
  /* sym_touch_every_page_text(); */

  /* printf("about to elevate and touch stack\n"); */

  /* uint64_t my_cr3_reg; */
  /* sym_elevate(); */

  /* asm("movq %%cr3,%0" : "=r"(my_cr3_reg)); */
  /* printf("%lx\n", my_cr3_reg); */

  /* sym_touch_stack(); */
  sym_elevate(); push_a_lot(); sym_lower();
}

void show_prefault_solves_DF(){
  sym_touch_stack();

  sym_elevate();

  sym_touch_stack();

  sym_lower();
}

void show_using_ist_solves_DF(){
  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for ist\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(my_idt);
  }

  sym_make_pg_ft_use_ist(my_idt);

  // If already swung, don't do anything.
  sym_store_idt_desc(&check_idtr);
  if(check_idtr.base != (uint64_t) &my_idt){
    // Swing idtr to new modified table
    sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
  }

  sym_elevate();
  sym_touch_stack();
  sym_lower();
}

static inline void
setPageWriteable(void *desc)
{
  struct pte *pte = desc;
  sym_set_pte_writeable(pte);
}

static inline bool
isPageWriteable(void *desc)
{
  struct pte * pte = desc;
  return sym_is_pte_writeable(*pte);
}

bool updateWriteIfNeeded(void *desc)
{
  printf("Is page writable?\n");
  if (isPageWriteable(desc)) return false;

  printf("If we're here, page shouldnt be writable yet\n");
  setPageWriteable(desc);
  printf("Now it should be writable\n");
  return true;
}

uint64_t
getVPNMask(unsigned int level)
{
  uint64_t mask = 0; 
  switch (level) {
  case PG_LEVEL_4K:
    mask = ~(((1ULL<<12)-1));
    break;
  case PG_LEVEL_2M:
    mask = ~(((1ULL<<20)-1));
    break;
  case PG_LEVEL_1G:
    mask =  ~(((1ULL<<29)-1));
    break;
  case PG_LEVEL_512G:
    mask =  ~(((1ULL<<38)-1));
    break;
  default:
    assert(0);
  }
  fprintf(stderr, "%s(%d) : %lx\n", __func__, level, mask);
  return mask;
}

void *
getPageDesc(uint64_t addr, unsigned int *level)
{
  //  returns a copy of the page descriptor
  sym_elevate();
  void *rc = sym_get_pte(addr, level);
  sym_lower();
  fprintf(stderr, "%s(%lx) : %p pglvl: %d\n", __func__, addr, rc, *level);
  return  rc;
}

typedef void * (*vzalloc_t)(unsigned long);
typedef void (*kvfree_t)(void *);

void * get_aligned_kern_pg(){
  vzalloc_t vzalloc = sym_get_fn_address("vzalloc");
  /* printf("About to alloc kern page\n"); */

  sym_elevate();
  void * p = vzalloc(1 << 12);
  // Mem leak need this to be page aligned 
  sym_lower();

  /* printf("got kern page ptr %p\n", p); */
  assert( ((long unsigned )p % (1<<12) ) == 0);
  return p;
}

void free_kern_mem(void *p){
  // Free page vzalloc'd
  kvfree_t kvfree  = sym_get_fn_address("kvfree");
  sym_elevate(); kvfree(p); sym_lower();
}

void show_using_system_ist_solves_DF(){
  // This actually worked.

  // Let's not think about text faults
  /* printf("show_using_system_ist_solves_DF()\n"); */
  sym_touch_every_page_text();

  /* printf("Allocate kern page\n"); */
  void *p = get_aligned_kern_pg();

  /* printf("Check system descriptor for page fault\n"); */
  /* sym_print_idt_desc((void *)system_idtr.base, PG_FT_IDX); */

  sym_copy_system_idt(p);

  /* printf("Sanity check copied descriptor matches\n"); */
  /* sym_print_idt_desc(p, PG_FT_IDX); */

  // modify to use ist
  /* printf("Sanity check copied descriptor matches\n"); */
  sym_set_idtr((unsigned long) p, IDT_SZ_BYTES - 1);

  /* printf("Switch onto copied kern page\n"); */
  sym_make_pg_ft_use_ist((unsigned char *) p);

  /* printf("Verify ist bit is set\n"); */
  /* sym_print_idt_desc(p, PG_FT_IDX); */

  /* printf("Try to force a starvation\n"); */
  sym_elevate(); push_a_lot(); sym_lower();

  // Swing idtr back
  /* printf("Switch back onto system idtr\n"); */
  sym_load_idtr(&system_idtr);

  /* printf("Check system descriptor for page fault\n"); */
  /* sym_print_idt_desc((void *) system_idtr.base, PG_FT_IDX); */

  /* printf("Free kern page\n"); */
  free_kern_mem(p);
}
#if 0
void interpose_on_df(){
  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for DOUBLE F\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(my_idt);
  }

  sym_interpose_on_df_c(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

void system_interpose_on_df(){
  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  printf("kern_pg_for_idt is %p\n", kern_pg_for_idt);
  if(check_idtr.base != (uint64_t) kern_pg_for_idt){
    printf("copying the idt for DOUBLE F system\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(kern_pg_for_idt);
  }
  printf("About to interpose\n");
  // TODO: sink these into sym lib
  sym_elevate();
  sym_interpose_on_df_asm(kern_pg_for_idt, kern_pg_for_df_handler);
  sym_lower();
  printf("About to swing idtr\n");

  // Make our user IDT live!
  sym_set_idtr((unsigned long)kern_pg_for_idt, IDT_SZ_BYTES - 1);
}
#endif

extern uint64_t cr3_reg;
#if 0
void interpose_on_pg_ft(){

  sym_elevate();
  asm("movq %%cr3,%0" : "=r"(cr3_reg));
  sym_lower();

  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for PAGE F\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(my_idt);
  }

  sym_interpose_on_pg_ft_c(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}
#endif

#if 0
void system_interpose_on_pg_ft(){

  sym_elevate();
  asm("movq %%cr3,%0" : "=r"(cr3_reg));
  sym_lower();

  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for PAGE F\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(my_idt);
  }

  sym_interpose_on_pg_ft_c(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}
#endif

#if 0
void show_using_idt_interpose_solves_DF(){
  // make sure we don't fault on text faults

  interpose_on_pg_ft();
  interpose_on_df();

  sym_elevate();
  push_a_lot();
  // TODO figure out why below seems unreliable?
  /* sym_touch_stack(); */
  sym_lower();
}
#endif



// Untested here, used to be in symlib
__asm__("\
  .text \n\t\
  .align 16 \n\t\
  .globl \t df_asm_handler \n\t\
  df_asm_handler: \n\t\
  movq   $0x6,(%rsp) \n\t\
  push   %rax \n\t\
  mov    $0xffffffff81e00ac0,%rax \n\t\
  xor    (%rsp),%rax /*Arlo's trick*/ \n\t\
  xor    %rax,(%rsp) \n\t\
  xor    (%rsp),%rax \n\t\
  ret \
");

#if 0
void show_using_system_idt_interpose_solves_DF(){
  // This has only been poorly tested.

  printf("About to touch text\n");
  sym_touch_every_page_text();

  /* system_interpose_on_pg_ft(); */

  // Init kallsym lib
  /* sym_lib_init(); */

  kern_pg_for_idt = get_aligned_kern_pg();
  // The PTE for this page may not have execute enabled.
  kern_pg_for_df_handler = get_aligned_kern_pg();

  unsigned int level;
  struct pte *handler_pte = sym_get_pte((uint64_t) kern_pg_for_df_handler, &level);

  sym_print_pte(handler_pte);

  /* printf("Try to get pte\n"); */
  if(sym_is_pte_execute_disable(handler_pte)){
    printf("Pte is XD\n");
    sym_clear_pte_execute_disable(handler_pte);
    printf("PTE should now not be XD\n");
    sym_print_pte(handler_pte);
    // TODO Still need to invalidate
    sym_elevate();
    __asm__("movq %%cr3, %%rax" :: : "%rax");
    __asm__("movq %rax, %cr3"); // does the flush
    sym_lower();
  }
  printf("pte should be good\n");

  sym_memcpy(kern_pg_for_df_handler, &df_asm_handler, 0x1d );

  system_interpose_on_df();

  // Test that DF doesn't occur
  sym_elevate(); push_a_lot(); sym_lower();

  // Get back on system idtr
  sym_load_idtr(&system_idtr);

  free_kern_mem(kern_pg_for_idt);
  free_kern_mem(kern_pg_for_df_handler);

}
#endif

void check_idt_and_pf(){
  sym_touch_every_page_text();
  struct dtr check_idtr;
  sym_elevate();
  sym_store_idt_desc(&check_idtr);
  sym_lower();

  printf("idtr is %lx\n", check_idtr.base);

  sym_elevate();
  sym_print_idt_desc((void *)check_idtr.base, DF_IDX); //PG_FT_IDX);
  sym_lower();

}

/* #define NORMAL_PROCESS 1 */
#define NAIVE_ELEVATION 1
/* #define PREFAULT_ELEVATION 1 */
/* #define IST_ELEVATION 1 */
/* #define SYSTEM_IST_ELEVATION 1 */
/* #define IDT_INTERPOSE 1 */
/* #define SYSTEM_IDT_INTERPOSE 1 */

int main(){
  printf("Starting main\n");
  sym_lib_init();

  /* sym_elevate(); */
  /* push_a_lot(); */
  /* sym_lower(); */

  /* check_idt_and_pf(); */
  /* return 0; */

#ifndef NORMAL_PROCESS
  // Store initial system IDTR
  sym_store_idt_desc(&system_idtr);
#endif

#ifdef NORMAL_PROCESS
  printf("NORMAL_PROCESS\n");
  show_process_stk_ft_works();
#endif

#ifdef NAIVE_ELEVATION
  printf("NAIVE_ELEVATION\n");
  show_naive_elevation_DFs();
#endif

#ifdef  PREFAULT_ELEVATION
  printf("PREFAULT_ELEVATION\n");
  show_prefault_solves_DF();
#endif

#ifdef IST_ELEVATION
  printf("IST_ELEVATION\n");
  show_using_ist_solves_DF();
#endif

#ifdef SYSTEM_IST_ELEVATION
  printf("SYSTEM_IST_ELEVATION\n");
  show_using_system_ist_solves_DF();
#endif

#ifdef IDT_INTERPOSE
  printf("IDT_INTERPOSE\n");
  show_using_idt_interpose_solves_DF();
#endif

#ifdef SYSTEM_IDT_INTERPOSE
  printf("SYSTEM_IDT_INTERPOSE\n");
  show_using_system_idt_interpose_solves_DF();
#endif

  printf("Done main\n");

#ifndef NORMAL_PROCESS
  // Restore system IDTR
  sym_load_idtr(&system_idtr);
#endif
}


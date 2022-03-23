#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>
#include <sys/mman.h>

#include <errno.h>


#include "LINF/sym_all.h"

#define eprintf(...) fprintf (stderr, __VA_ARGS__)

// Our version of the idt. Not sure about alignment.
unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

struct dtr system_idtr;

typedef void (*void_fn_ptr)(unsigned long);
void_fn_ptr get_fn_address(char *symbol){
  struct kallsymlib_info *info;

  if (!kallsymlib_lookup(symbol, &info)) {
    fprintf(stderr, "%s : not found\n", symbol);
  }
  return (void_fn_ptr) info->addr;
}

void interpose_on_pg_ft(){

  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &my_idt){
    printf("copying the idt for pf\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(my_idt);
  }

  sym_interpose_on_pg_ft_c(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);

}
/* void interpose_on_pg_ft(){ */
/*   int PG_FT_IDX= 14; */
/*   // Copy the system idt to userspace */
/*   sym_copy_system_idt(my_idt); */

/*   // Modify it to inject a shim that flips a  */
/*   sym_interpose_on_pg_ft(my_idt); */

/*   // Make our user IDT live! */
/*   sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1); */
/* } */

extern void foo(void);

void show_process_text_ft_works(){
  foo();
}

void show_naive_elevation_breaks(){
  /* fprintf(stderr, "Getting in foo\n"); */

  /* int res = madvise((void *)&foo, (size_t)5*(1<<12), MADV_REMOVE); */
  /* printf("Want to call madvise(%p, %lx, %d)\n", &foo, 5 *(1<<12), MADV_DONTNEED); */

  /* uint64_t my_foo = (uint64_t) &foo; */
  /* printf("foo at %lx\n", my_foo); */

  /* my_foo = my_foo >> 12; */
  /* my_foo = my_foo << 12; */

  /* printf("lo bound pg at %lx\n", my_foo); */

  /* my_foo += 1<<12; */
  /* printf("next pg %lx\n", my_foo); */


  /* if (res == -1) */
  /*   err(1, NULL); */
    /* printf("memadvise errno %d\n", errno); */



  sym_elevate(); foo(); sym_lower();
  /* fprintf(stderr, "done foo\n"); */
  /* sym_elevate(); printf("hi %d\n", 42); sym_lower(); */
}

void show_int_interposition_works(){
  sym_elevate();
  interpose_on_pg_ft();

  // Try to force text fault
  sym_elevate(); foo(); sym_lower();
}

typedef void * (*vzalloc_t)(unsigned long);
typedef void (*kvfree_t)(void *);

void * get_aligned_kern_pg(){
  // TODO fix warning
  vzalloc_t vzalloc = (vzalloc_t) get_fn_address("vzalloc");
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
  // TODO: fix warning
  kvfree_t kvfree  = (kvfree_t) get_fn_address("kvfree");
  sym_elevate(); kvfree(p); sym_lower();
}

void lazy_hexdump(unsigned char *base, unsigned int amt){
  eprintf("1st bytes at %p\n", base);
  for(unsigned int i = 0; i<amt; i++){
    unsigned char * tmp = base + i;
    sym_elevate();
    unsigned char tmp_char = *tmp;
    sym_lower();
    eprintf("%02X ", tmp_char);
  }
  eprintf("\n");
}

void show_system_int_interposition_works(){
  /* sym_elevate(); */
  /* printf("c_handler_page_fault lives at %p \n", &c_handler_page_fault); */
  /* printf("c_handler_page_fault lives at %p \n", &main); */

  // Get a page to hold a copy of the system IDT
  void *idt_cp = get_aligned_kern_pg();

  sym_copy_system_idt(idt_cp);

  // Get a page to hold a copy of the c_handler_page_fault interposer
  void *handler_page = get_aligned_kern_pg();

  /* // Copy handler onto page */
  unsigned char *c_handler_page_fault_ptr = (unsigned char *) c_handler_page_fault;
#if DEBUG
  lazy_hexdump(c_handler_page_fault_ptr, 80);
#endif

  sym_elevate();
  memcpy(handler_page, c_handler_page_fault_ptr, 80);
  sym_lower();

#if DEBUG
lazy_hexdump(handler_page, 80);
#endif

  // confirm page is executable
  unsigned int level;
  /* printf("about to get pte\n"); */


  struct pte *handler_pte = sym_get_pte((uint64_t) handler_page, &level);
  /* printf("Got pte\n"); */
  /* uint64_t my_pte = *handler_pte; */

  /* fprintf(stderr, "%s(%p) : %p pglvl: %d\n", __func__, handler_page, handler_pte, level); */
#if DEBUG
sym_print_pte(handler_pte);
#endif
  /* /\* printf("Try to get pte\n"); *\/ */
  /* if(sym_is_pte_execute_disable(handler_pte)){ */
  /*   printf("Pte is XD\n"); */
  /*   sym_clear_pte_execute_disable(handler_pte); */
  /*   printf("PTE should now not be XD\n"); */
  /*   sym_print_pte(handler_pte); */
  /*   // TODO Still need to invalidate */
  /*   __asm__("mov %cr3, %cr3"); */
  /* } */
  /* printf("pte should be good\n"); */


  // Point idt_cp[pg_ft] at handler page
#if DEBUG
  printf("System idt pf desc\n");
  printf("System idtr base %lx\n", system_idtr.base);
  sym_print_idt_desc((unsigned char *) system_idtr.base, PG_FT_IDX);
#endif

#if DEBUG

  printf("Cp to modify\n");
  sym_print_idt_desc((unsigned char *) idt_cp, PG_FT_IDX);
#endif

  union idt_desc *sys_cp_pf_desc = sym_get_idt_desc((unsigned char *)idt_cp, PG_FT_IDX);

  // Prepare new handler.
  union idt_addr new_handler;
  new_handler.raw = (uint64_t) handler_page;

  // Set idt_cp to point to new handler
  // XXX comment me in!
  /* sym_load_desc_from_addr(sys_cp_pf_desc, &new_handler); */

#if DEBUG
  sym_print_idt_desc((unsigned char *) idt_cp, PG_FT_IDX);
#endif

  /* // Swing idtr onto idt_cp */
  sym_set_idtr((unsigned long)idt_cp, IDT_SZ_BYTES - 1);

  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);
#if DEBUG
  printf("New idtr base %lx\n", check_idtr.base);
#endif


  /* /\* interpose_on_pg_ft(); *\/ */
  /* // Try to force text fault */
  /* sym_elevate(); bar(); sym_lower(); */
  sym_elevate(); foo(); sym_lower();

  #if 0
  sym_load_idtr(&system_idtr);
  free_kern_mem(idt_cp);
  free_kern_mem(handler_page);
  #endif
}

void make_nop_slide(){
  printf("make_nop_slide NYI\n");
}

void show_nop_slide_works(){
  // Access the first byte of page we'll fault on.
  /* make_nop_slide(); */
  printf("char at addr is %x\n", *((unsigned char*) 0xffffffff810588da) );
  while(1);
  asm("WBINVD");
  sym_elevate();
  foo();
  sym_lower();
}

void show_prefault_works(){
  // Access the first byte of page we'll fault on.
  char *c;
  c = (char *) 0x404000; // Addr first text fault
  printf("byte at %p is %c\n", c, *c);
  sym_elevate();
  foo();
  sym_lower();
}
/*
  1) Process works
  2) Naive elevation fails
  3) Prefault works
  4) Interposition works.
*/

/* #define NORMAL_PROCESS 1 */
/* #define NAIVE_ELEVATION 1 */
/* #define PREFAULT_ELEVATION 1 */
/* #define INT_INTERPOSITION 1 */
#define SYSTEM_INT_INTERPOSITION 1
/* #define NOP_SLIDE 1 */

/* void invalidate_pte(){ */
/*   printf("Let's see if bar is mapped\n"); */
/*   printf("Bar lives at %p\n", &bar); */
  
/* } */

extern uint64_t cr3_reg;

int main(){
  printf("Starting main\n");
  sym_lib_init();

  /* printf("pid is %d\n", getpid()); */
  /* getchar(); */

  // Store initial system IDTR
  sym_store_idt_desc(&system_idtr);

#ifdef NORMAL_PROCESS
  printf("NORMAL_PROCESS\n");
  show_process_text_ft_works();
#endif

#ifdef NAIVE_ELEVATION
  printf("NAIVE_ELEVATION\n");
  show_naive_elevation_breaks();
#endif

#ifdef  PREFAULT_ELEVATION
  printf("PREFAULT_ELEVATION\n");
  show_prefault_works();
#endif

#ifdef NOP_SLIDE
  printf("NOP_SLIDE\n");
  printf("currently doesn't work\n");
  show_nop_slide_works();
#endif

#ifdef INT_INTERPOSITION
  printf("INT_ELEVATION\n");
  show_int_interposition_works();
#endif

#ifdef SYSTEM_INT_INTERPOSITION
  printf("SYSTEM_INT_ELEVATION\n");
  show_system_int_interposition_works();
#endif

  printf("Done main\n");

  // Swing back onto system idtr before exit
  sym_load_idtr(&system_idtr);
}



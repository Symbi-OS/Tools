#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <time.h>
#include <string.h>

#include "../../include/headers/sym_all.h"

// Our version of the idt. Not sure about alignment.
unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

void interpose_on_int3_ft(){
  // Copy the system idt to userspace
  sym_copy_system_idt(my_idt);

  sym_interpose_on_int3_ft(my_idt);

  // Make our user IDT live!
  sym_set_idtr((unsigned long)my_idt, IDT_SZ_BYTES - 1);
}

struct pte{
uint64_t
  SEL : 1,
  RW : 1,
  US : 1,
  PWT: 1,
  PCD: 1,
  A : 1,
  DIRTY: 1,
  MAPS : 1,
  WHOCARES2 : 4,
  PG_TBL_ADDR : 40,
  RES : 11,
  XD  : 1;
};

typedef void* (*lookup_address_t)(uint64_t address, unsigned int * level);

void modify_pt(uint64_t addr){

  // Get PTE
  lookup_address_t my_lookup_address = (lookup_address_t) 0xffffffff8105ce60;
  unsigned int level;
  void* ret = my_lookup_address(addr, &level);
  printf("got %p for a pte ptr\n", ret);

  struct pte* pte_p = (struct pte*) ret;
  printf("RW bit is %d\n", pte_p->RW);
  pte_p->RW = 1;
  printf("RW bit is %d\n", pte_p->RW);
}

void write_ktext_addr(uint64_t addr, char *s, int len){
  modify_pt(addr);
  sym_elevate();
  memcpy((void *)addr, s, len);
}

void show_int_interposition_works(){

  sym_elevate();

  interpose_on_int3_ft();

  sym_elevate();
  /* ffffffff8107dbf0 t __do_sys_getpid */
  printf("first byte of getpid is %#x \n", *((unsigned char *) 0xffffffff8107dbf0 ));

  char s = 0xcc;
  write_ktext_addr(0xffffffff8107dbf0, &s, 1);

  getpid();
  getpid();
}

int main(){
  sym_touch_every_page_text();

  printf("Starting main\n");

  // Store system idtr here for later restoration.
  struct dtr system_idtr;

  // Store initial system IDTR
  sym_store_idt_desc(&system_idtr);

  /* interpose_on_pg_ft(); */

  show_int_interposition_works();


  printf("Done main\n");
  // Swing back onto system idtr before exit
  sym_load_idtr(&system_idtr);
}

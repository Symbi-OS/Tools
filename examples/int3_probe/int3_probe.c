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



void write_ktext_addr(uint64_t addr, char *s, int len){
  // TODO if write spans pages, this will fail.
  sym_make_pte_writable(addr);
  sym_elevate();
  memcpy((void *)addr, s, len);
}

void show_int_interposition_works(){

  sym_elevate();

  interpose_on_int3_ft();

  sym_elevate();
  /* ffffffff8107dbf0 t __do_sys_getpid */
  printf("first byte of getpid is %#x \n", *((unsigned char *) 0xffffffff8107dbf0 ));

  // Magic int3 instruction.
  /* char s = 0xcc; */
  /* write_ktext_addr(0xffffffff8107dbf0, &s, 1); */

  sym_set_probe(0xffffffff8107dbf0);

  getpid();
  getpid();
}

int main(){
  sym_touch_every_page_text();

  printf("Starting main\n");

  // Store system idtr here for later restoration.
  struct dtr system_idtr;
  sym_store_idt_desc(&system_idtr);

  /* interpose_on_pg_ft(); */

  show_int_interposition_works();


  printf("Done main\n");

  // Swing back onto system idtr before exit
  sym_load_idtr(&system_idtr);
}

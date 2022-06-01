#include "L2/sym_probe.h"
#include "L0/sym_lib.h"
#include "L1/sym_interrupts.h"
#include "L2/sym_lib_page_fault.h"

#include <string.h>

#include "LIDK/idk.h"

// This is the old handler we jmp to after our interposer.
uint64_t orig_asm_exc_int3;

uint64_t cr3_reg = 0;


static void (*myprintk)(char *);

unsigned char reset_byte = 0xf;

uint64_t int3_count = 0;
uint64_t int3_rdi = 0;
uint64_t int3_rsi = 0;
uint64_t int3_rdx = 0;

uint64_t addr_msg = 0;


// NOTE: This function is not used in C code, but is used in inline assembly.
// This asks the compiler not to warn about it being unused.
/* static uint64_t __attribute((unused)) my_entry = (uint64_t) &tu_c_entry; */

/* extern uint64_t int3_jmp_to_c; */
/* MY_INT3_HANDLER(int3_jmp_to_c, *my_entry); */
MY_INT3_HANDLER(int3_jmp_to_c, bp_c_entry);

static void bp_c_entry(struct pt_regs *pt_r){
  // HACK: safe way is to generate pointer into pt_regs
  // This looks safe for first 3 args for now.
  // RAX def gets clobbered XXX

  // tcp_sendmsg

  // Assume we got here on an int3

  // NOTE: PC has moved past int3. Need it there again to replace byte
  // and to execute the instruction we replaced.
  pt_r->rip -= 1;
  unsigned char *ucp = (unsigned char *) pt_r->rip;
  /*   // assert that val we got here on was 0xcc, or int3 */
  while(*ucp != 0xcc);

  // Fixup instruction assuming it's an 0xf
  *ucp = 0xf;



  return;

  /* memcpy((void *)addr_msg, (void*)int3_rsi, 96); */

  // Our simulated memcpy
  char *csrc = (char *)int3_rsi;
  char *cdest = (char *)addr_msg;
  int n = 96;
  for (int i=0; i<n; i++)
    cdest[i] = csrc[i];

  return;

  myprintk("hey\n");

  uint64_t my_cr3;
  GET_CR3(my_cr3);
  if(cr3_reg == my_cr3){
    myprintk("wow, cr3 matches\n");

    myprintk("Need to swing overwritten byte back to 0xf\n");
    unsigned char *ucp = (unsigned char *) 0xffffffff810fbfe0;
    *ucp = reset_byte;
  }else{
    myprintk("Bummer, no match\n");
  }
}

// This is the name of our assembly we're adding to the text section.
// It will be defined at link time, but use this to allow compile time
// inclusion in C code.
/* extern uint64_t bs_asm_exc_int3; */

void sym_probe_init(){
  printf("Init SP\n");
  myprintk =  sym_get_fn_address("printk");
}

void sym_interpose_on_int3_ft_c(unsigned char * my_idt){
  /* sym_print_idt_desc(my_idt, X86_TRAP_BP); */

  // Get ptr to pf desc
  union idt_desc *desc_old;
  desc_old = sym_get_idt_desc(my_idt, X86_TRAP_BP);

  // save old asm_exc_pf ptr
  union idt_addr old_asm_exc_int3;
  sym_load_addr_from_desc(desc_old, &old_asm_exc_int3);

  // This is stored, but not used.
  orig_asm_exc_int3 = old_asm_exc_int3.raw;

  // New handler
  union idt_addr new_asm_exc_addr;
  new_asm_exc_addr.raw = (uint64_t) &int3_jmp_to_c;

  // Set IDT to point to our new interposer
  sym_load_desc_from_addr(desc_old, &new_asm_exc_addr);

  /* sym_print_idt_desc(my_idt,  X86_TRAP_BP); */
}

unsigned char sym_set_probe(uint64_t addr){
  // TODO if write spans pages, this will fail.
  sym_elevate();
  unsigned char ret = *(unsigned char *) addr;
  sym_lower();
  if (ret != 0xf){
    fprintf(stderr, "Error, byte to be replaced by int3 0xcc was not 0xf as needed by hard coded interposer.\n");
    while(1);
  }
  sym_make_pg_writable(addr);

  sym_elevate();
  // Magic write int3 instruction.
  *(unsigned char *) addr = 0xcc;
  sym_lower();

  return ret;
}

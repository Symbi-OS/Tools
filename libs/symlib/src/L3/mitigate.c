#include "L0/sym_lib.h"
#include "L0/sym_structs.h"
#include "L1/sym_interrupts.h"
#include "L2/sym_lib_page_fault.h"
#include "L3/mitigate.h"

static void sym_copy_idt(unsigned char *user_idt){
  // We want to check if another interposition has already taken over idt
  struct dtr check_idtr;
  sym_store_idt_desc(&check_idtr);

  if(check_idtr.base != (uint64_t) &user_idt){
    printf("copying the idt for pf\n");
    // Copy the system idt to userspace if we haven't already.
    sym_copy_system_idt(user_idt);
  } else{
    printf("no copy made pg_ft\n");
  }
}

static struct dtr system_idtr;
void sym_mitigate_pf(unsigned char *user_idt){
  sym_store_idt_desc(&system_idtr);

  sym_copy_idt(user_idt);
  sym_interpose_on_pg_ft_c(user_idt);
  // Make our user IDT live!
  sym_set_idtr((unsigned long)user_idt, IDT_SZ_BYTES - 1);

}

void sym_mitigate_pf_cleanup(){
  // Swing back onto system idtr before exit
  sym_load_idtr(&system_idtr);

  // Make sure we lower.
  if(sym_check_elevate()){
    printf("Didn't expect to be elevated\n");
    sym_lower();
  }
}

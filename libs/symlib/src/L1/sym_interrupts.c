#include "L0/sym_lib.h"
#include "L0/sym_structs.h"
#include <string.h>

// Fn Documentation in header file.

// Store into IDTR with struct dtr *
void sym_load_idtr(struct dtr *location) {
  sym_elevate();
  // put value in idtr from memory
  __asm__ __volatile__("lidt %0" : :  "m"(*location) );
  sym_lower();
}

// Get IDTR
void sym_store_idt_desc(struct dtr *location) {
  // put value into memory from idtr
  // Not technically a supervisor instruction, but linux does expensive
  // software emulation if not elevated.
  sym_elevate();
  __asm__ __volatile__("sidt %0" : : "m"(*location) : "memory");
  sym_lower();
}

// Load idtr with raw base and bound
void sym_set_idtr(unsigned long base, unsigned short bound ){
  struct dtr idtr = {bound,base};
  sym_load_idtr(&idtr);
}

void sym_restore_system_idt(){
  struct dtr my_dtr;

  // Get system idt
  sym_store_idt_desc(&my_dtr);


  // Set system idt
  sym_load_idtr(&my_dtr);
}

void
sym_get_idt_base (struct dtr *idtr)
{
  /* In 64-bit mode, the operand size is fixed at 8+2 bytes. The instruction stores an 8-byte base and a 2-byte limit. */

  /* _asm sgdt gdtr gdt = *((unsigned long *)&gdtr[2]); */
  sym_store_idt_desc(idtr);
  /* printf("idtr limit: #%x \n", idtr->limit); */
  /* printf("idtr base : #%lx \n", idtr->base); */
}

// This is the kernel location for the IDT, it's one page in length.
void sym_copy_system_idt(unsigned char *sym_idt_base){
  struct dtr idtr;
  // TODO: assert that idt is at LEAST page alligned.
  sym_store_idt_desc(&idtr);

  /* printf("kern idt at %p\n", idtr.base); */
  sym_elevate();
  memcpy( (void *) sym_idt_base, (const void *) idtr.base, IDT_SZ_BYTES);
  sym_lower();

}

// Returns pointer to idt entry. Does not allocate memory.
union idt_desc *
sym_get_idt_desc(unsigned char *idt_base, unsigned int idx){
  union idt_desc *my_desc = (union idt_desc *) (idt_base + (16*idx));
  return my_desc;
}

// Pass in ptr to the idt you want modified and the idx of the desc you want modified.
// Pass in ptr to the new descriptor you want copied in.
void sym_set_idt_desc(unsigned char *idt_base, unsigned int idx, union idt_desc *new_desc){
  /* printf("set idt entry\n"); */

  // Were assuming idt_base is in user side of addr space.

  union idt_desc *my_desc;
  my_desc = sym_get_idt_desc(idt_base, idx);

  sym_elevate();
  // Do deep copy
  *my_desc = *new_desc;
  sym_lower();
}

// Loads up an addr from a desc.
void sym_load_addr_from_desc(union idt_desc *desc, union idt_addr *addr){
  addr->dcmp.lo  = desc->fields.lo_addr;
  addr->dcmp.mid = desc->fields.mid_addr;
  addr->dcmp.hi  = desc->fields.hi_addr;
}

// Loads desc from an addr.
void sym_load_desc_from_addr(union idt_desc *desc, union idt_addr *addr){
  desc->fields.lo_addr  = addr->dcmp.lo;
  desc->fields.mid_addr = addr->dcmp.mid;
  desc->fields.hi_addr  = addr->dcmp.hi;
}

void sym_print_idt_desc(unsigned char *idt, unsigned int idx){
  union idt_desc *my_desc;
  my_desc = sym_get_idt_desc(idt, idx);

  printf("my_desc lives at %p\n", my_desc);

  union idt_addr my_idt_addr;
  sym_load_addr_from_desc(my_desc, &my_idt_addr);

  printf("full addr: %lx\n", my_idt_addr.raw       );
  printf("segment:   %x\n",   my_desc->fields.seg_sel);
  printf("ist:       %x\n",   my_desc->fields.ist    );
  printf("zero0:     %x\n",   my_desc->fields.zero0  );
  printf("type:      %x\n",   my_desc->fields.type   );
  printf("dpl:       %x\n",   my_desc->fields.dpl    );
  printf("p:         %x\n",   my_desc->fields.p      );
}

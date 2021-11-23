#include <unistd.h>
#include <stdio.h>

#include <assert.h>
#include <stdint.h>

#include <string.h>

#include "../../include/sym_lib_syscall.h"

// TODO Rename to generalize
struct dtr
{
  unsigned short limit; // limit 2
  unsigned long base;   // base 8
}__attribute__((packed));

union idt_addr{
  struct{
    uint16_t lo;
    uint16_t mid;
    uint32_t hi;
  } dcmp;
  struct{
    uint64_t addr;
  } addr;
}__attribute__((packed));
static_assert(sizeof(union idt_addr) == 8, "Size of idt_addr is not correct");

union idt_desc{

  struct {
    uint64_t lo;
    uint64_t hi;
  } raw;

  struct {
    // Low 2 bytes
    uint16_t  lo_addr;

    // segment selector
    uint16_t  seg_sel;

    // Zero
    uint16_t zero: 8;

    // Type
    uint16_t type: 4;

    // Zero
    uint16_t zero2: 1;

    // Descriptor Priv Level
    uint16_t dpl: 2;

    // Gate valid
    uint16_t p:1;

    // mid 2
    uint16_t  mid_addr;

    // High 4 bytes
    uint32_t hi_addr;

    //
    uint32_t res;
  } fields;
}__attribute__((packed));
static_assert(sizeof(union idt_desc) == 16, "Size of idt_desc is not correct");

void stack_test(){
  int i = 0;
  // Let's do a bunch of pushes and see if we can trigger the stack bug.
  intptr_t sp;
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Rsp is %lx\n", sp);
  for(; i < (1<<12); i++){
    asm("pushq $42");
  }
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Rsp is %lx\n", sp);

}

void touch_stack(){
  intptr_t sp;
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Touching stack Rsp is %lx\n", sp);

  int i = 0;
  // Push a bunch of values
  for(; i < (1<<12); i++){
    asm("pushq $42");
  }

  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Touching stack Rsp is %lx\n", sp);
  i = 0;

  // Pop them all off
  for(; i < (1<<12); i++){
    asm("popq %rax");
  }
  asm ("movq %%rsp, %0" : "=r" (sp) );
  printf("Touching stack Rsp is %lx\n", sp);
}

void use_ist_avoid_df(){
  // steal idt

  // make page faults use IST

}

unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));

void load_idtr(struct dtr *location) {
  // put value in idtr from memory
  __asm__ __volatile__("lidt %0" : :  "m"(*location) );
  /* __asm__ __volatile__("sidt %0" : : "m"(*location) : "memory"); */
}

void store_idt_desc(struct dtr *location) {
  // put value into memory from idtr
  __asm__ __volatile__("sidt %0" : : "m"(*location) : "memory");
}

void set_idtr(){
  struct dtr idtr = {0xfff,(unsigned long)my_idt};
  printf("We will set the idtr with \n");
  printf("idtr limit: #%x \n", idtr.limit);
  printf("idtr base : #%lx \n", idtr.base);

  /* printf("force a div by 0 error\n"); */
  /* printf("4 / 0 = %d\n", 4/0); */

  load_idtr(&idtr);
  // Trigger div by 0 exception -> divide_error()
  /* printf("4 / 0 = %d\n", 4/0); */
}

void
get_idt_base (void)
{
  /* In 64-bit mode, the operand size is fixed at 8+2 bytes. The instruction stores an 8-byte base and a 2-byte limit. */
  struct dtr idtr;

  /* _asm sgdt gdtr gdt = *((unsigned long *)&gdtr[2]); */
  store_idt_desc(&idtr);
  printf("idtr limit: #%x \n", idtr.limit);
  printf("idtr base : #%lx \n", idtr.base);
}

// This is the kernel location for the IDT, it's one page in length.
void copy_idt(){
  struct dtr idtr;
  store_idt_desc(&idtr);
  memcpy( (void *) my_idt, (const void *) idtr.base, 1<<12);
}
void print_idt_entry(unsigned char *idt, unsigned int idx){
  /* printf("table lives at %p, index is %d\n", idt, idx); */

  /* printf("address of descriptor is %p\n", idt + (16*idx)); */

  union idt_desc *my_desc = (union idt_desc *) (idt + (16*idx));

  /* printf("my_desc lives at %p\n", my_desc); */

  /* printf("raw\n"); */
  /* printf("hi: %llx\n", my_desc->raw.hi); */
  /* printf("lo: %llx\n", my_desc->raw.lo); */

  /* printf("addr\n"); */
  /* printf("hi: %llx\n", my_desc->fields.hi_addr); */
  /* printf("md: %llx\n", my_desc->fields.mid_addr); */
  /* printf("lo: %llx\n", my_desc->fields.lo_addr); */

  union idt_addr my_idt_addr;
  my_idt_addr.dcmp.lo = my_desc->fields.lo_addr;
  my_idt_addr.dcmp.mid = my_desc->fields.mid_addr;
  my_idt_addr.dcmp.hi = my_desc->fields.hi_addr;

  printf("full addr: %llx\n", my_idt_addr.addr);
  printf("segment:   %x\n",   my_desc->fields.seg_sel );
  printf("zero:      %x\n",   my_desc->fields.zero );
  printf("type:      %x\n",   my_desc->fields.type );
  printf("zero2:     %x\n",   my_desc->fields.zero2 );
  printf("dpl:       %x\n",   my_desc->fields.dpl );
  printf("p:         %x\n",   my_desc->fields.p );

}

int main(){
  printf("Copy idt\n");
  sym_touch_stack();

  sym_elevate();
  copy_idt();
  sym_lower();

  printf("Read nth idt entry\n");

  print_idt_entry(my_idt, 0);
  print_idt_entry(my_idt, 1);
  print_idt_entry(my_idt, 2);
  print_idt_entry(my_idt, 3);
  print_idt_entry(my_idt, 15);





  return 0;

  /* printf("Touch the stack\n"); */
  /* touch_stack(); */

  printf("Request elevation\n");
  sym_elevate();

  printf("Try the test\n");
  stack_test();

  printf("Lower now\n");
  sym_lower();
  return 0;
}

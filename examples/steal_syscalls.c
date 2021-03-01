#include <assert.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>
#include "elevate.h"

// TODO: function to check if you're a symbiote
// Megabyte aligned.
__attribute__((aligned(1048576))) void my_syscall_handler(){
  // Confirm in gdb we get here.
  // TODO paste in linux syscall handler.
  // TODO paste syscall table.
  while(1);
}

union idt_desc{

  struct {
  uint64_t lo;
  uint64_t hi;
  } raw;

  struct {
    // Low 8 bytes
    uint16_t  lo_addr;
    uint16_t  seg_sel;
    uint16_t  ctrl_bits; // TODO, break this down.
    uint16_t  mid_addr;

    // High 8 bytes
    uint32_t hi_addr;
    uint32_t reserved;
  } fields;
}__attribute__((packed));
static_assert(sizeof(union idt_desc) == 16, "Size of idt_desc is not correct");
/* static_assert(sizeof(idt_desc.raw) == sizeof()) */
/* static_assert(sizeof(int)      ==  4, "Code relies on int being exactly 4 bytes"); */

// TODO statically assert this is 16 bytes.
/* static_assert(sizeof(struct idt_desc) == 16 ); */

//TODO idk if there's any alignment to respect, copying divide_error's alignment
/* __attribute__((aligned(8))) void my_divide_error(){ */
  // get entry 0
  // change address to user
  // set entry 0
/* } */

/* static inline void wrmsrl(unsigned int msr, u64 val) */
/* typedef uint64_t u64; */
/* void (*wrmsrl_elevated)(int, u64) = ( void(*)(int, u64) ) 0xffffffff8107f460; */
/* wrmsrl(MSR_LSTAR, (unsigned long)entry_SYSCALL_64); */

/* ffffffff81044ea7:       48 c7 c0 00 00 e0 81    mov    $0xffffffff81e00000,%rax */
/*   ffffffff81044eae:       b9 82 00 00 c0          mov    $0xc0000082,%ecx */
/*   ffffffff81044eb3:       48 89 c2                mov    %rax,%rdx */
/*   ffffffff81044eb6:       48 c1 ea 20             shr    $0x20,%rdx */
/*   ffffffff81044eba:       0f 30                   wrmsr */
/*   ffffffff81044ebc:       0f 1f 44 00 00          nopl   0x0(%rax,%rax,1) */
void steal_syscall_handler(){
  printf("About to steal syscall MSR, STAR, LSTAR\n");
  // Need MSR_LSTAR
  // need address wrmsrl
  // XXX page aligned handler ffffffff81e00000 T entry_SYSCALL_64
  /* wrmsrl(MSR_LSTAR, (unsigned long)my_syscall_handler);     // Swing pointer here */

  asm (
       "mov    $0x600000,%rax;"
       "mov    $0xc0000082,%ecx;"
       "mov    %rax,%rdx;"
       "shr    $0x20,%rdx;"
       "wrmsr;"
       "nopl   0x0(%rax,%rax,1);"
       );

  asm("syscall");


}

__attribute__((aligned(16))) void my_interrupt_handler(){
  while(1);
}

void steal_interrupt_handler(){
  /* asm("int3"); */
  printf("Enter a char\n");
  char var;
  var = getchar();
  printf(" Welcome to  %c",var);
}

// TODO Rename to generalize
struct gdtr
{
  unsigned short limit; // limit 2
  unsigned long base;   // base 8
}__attribute__((packed));
// TODO statically assert this is 10 bytes.
/* static_assert(sizeof(struct gdtr) == 10 ); */

// Store (sidt) -> write value from idtr MSR -> memory
// Load  (lidt) -> write value from memory -> idtr
void load_idtr(struct gdtr *location) {
  // put value in idtr from memory
  __asm__ __volatile__("lidt %0" : :  "m"(*location) );
  /* __asm__ __volatile__("sidt %0" : : "m"(*location) : "memory"); */
}

void store_idt_desc(struct gdtr *location) {
  // put value into memory from idtr
  __asm__ __volatile__("sidt %0" : : "m"(*location) : "memory");
}

unsigned char my_idt [1<<12] __attribute__ ((aligned (1<<12) ));
void set_idtr(){
  struct gdtr idtr = {0xfff,(unsigned long)my_idt};
  printf("We will set the idtr with \n");
  printf("idtr limit: #%x \n", idtr.limit);
  printf("idtr base : #%lx \n", idtr.base);

  /* printf("force a div by 0 error\n"); */
  /* printf("4 / 0 = %d\n", 4/0); */

  load_idtr(&idtr);
  // Trigger div by 0 exception -> divide_error()
  /* printf("4 / 0 = %d\n", 4/0); */
}

/* asm (".global my_div_error;  my_div_error: jmp ."); */
/* asm(".global _libos_start; _libos_start: pushf; push %rdx; callq libos_init; pop %rdx; popf; jmp _start"); */

// Try to hit this path.
void __attribute__((aligned(16))) my_div_error(){
  while(1);
}

void
get_idt_base (void)
{
  /* In 64-bit mode, the operand size is fixed at 8+2 bytes. The instruction stores an 8-byte base and a 2-byte limit. */
  struct gdtr idtr;

  /* _asm sgdt gdtr gdt = *((unsigned long *)&gdtr[2]); */
  store_idt_desc(&idtr);
  printf("idtr limit: #%x \n", idtr.limit);
  printf("idtr base : #%lx \n", idtr.base);
}


void store_gdt_desc(struct gdtr *location) {
  // read the gdtr into memory
  __asm__ __volatile__("sgdt %0" : : "m"(*location) : "memory");
}

unsigned long
get_gdt_base (void)
{
  /* In 64-bit mode, the operand size is fixed at 8+2 bytes. The instruction stores an 8-byte base and a 2-byte limit. */
  struct gdtr gdtr;

  unsigned long   gdt= 0;

  /* _asm sgdt gdtr gdt = *((unsigned long *)&gdtr[2]); */
  store_gdt_desc(&gdtr);
  printf("gdtr limit: #%x \n", gdtr.limit);
  printf("gdtr base : #%lx \n", gdtr.base);


  return (gdt);

}

// ... SNIP ...

void
test3 (void)
{
  unsigned int gdt_base= 0;

  gdt_base = get_gdt_base();

  printf ("\n[+] Test 3: GDT\n");
  printf ("GDT base: 0x%x\n", gdt_base);

  if ((gdt_base >> 24) == 0xff) {
    printf ("Result  : VMware detected\n\n");
    return;

  }

  else {
    printf ("Result  : Native OS\n\n");
    return;

  }
}
// This is the kernel location for the IDT, it's one page in length.
void copy_idt(){
  memcpy( (void *) my_idt, (const void *) 0xfffffe0000000000, 1<<12);
}

union idt_desc my_desc;
void replace_div_error(){
  // Read element 0
  my_desc.raw.lo = *(uint64_t *) my_idt;
  my_desc.raw.hi = *(uint64_t *) (my_idt+8);
  printf("lo component: %lx \n", my_desc.raw.lo);
  printf("hi component: %lx \n", my_desc.raw.hi);

  // Modify to point to my_div_error
  uint64_t my_div_error_address = (uint64_t) my_div_error;
  my_desc.fields.hi_addr  = (my_div_error_address >> 32) & 0xffffffff;
  my_desc.fields.mid_addr =(my_div_error_address >> 16) & 0xffff;
  my_desc.fields.lo_addr  =my_div_error_address & 0xffff;
  printf("my_div_error is %p or by my math... %lx\n", my_div_error, my_div_error_address);
  // Write into my table
  *(union idt_desc *) my_idt = my_desc;
}

int main(int argc, char* argv[]) {
  printf("We should be a symbiote now\n");
  copy_idt();
  set_idtr();

  replace_div_error();

  printf("replaced idt entry for div by zero\n");
  printf("my favorite number is %d\n", 7/0);
  printf("dont go here\n");



  /* get_idt_base(); */
  /* steal_interrupt_handler(); */
  /* test3(); */

  // TODO
  /* restore_system_idtr(); */
  return 0;
}
/* May not be marked __init: used by software suspend */
/* void syscall_init(void) */
/* { */
/*   wrmsr(MSR_STAR, 0, (__USER32_CS << 16) | __KERNEL_CS); */   // Don't need to do this part
/*   wrmsrl(MSR_LSTAR, (unsigned long)entry_SYSCALL_64); */      // Swing pointer here
/*   wrmsrl(MSR_SYSCALL_MASK, */                                 // Might be nothing to do here.
/*                 X86_EFLAGS_TF|X86_EFLAGS_DF|X86_EFLAGS_IF| */
/*          X86_EFLAGS_IOPL|X86_EFLAGS_AC|X86_EFLAGS_NT); */
/* } */

#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string.h>

#define PG_SZ (1ULL << 12)
#define USER_FT 1<<2
#define INS_FETCH 1<<4

typedef unsigned (*asmFunc)(void);

void printer(){
  printf("Hello world\n");
}

void foo(){
  asm("movq $0x6, (%rsp)");
  asm("push %rax"); // push a // &a = rsp + 8
  asm("mov $0x0000000000401126, %rax"); // overwite reg
  asm("xor (%rsp), %rax"); // rax = a ^ p
  asm("xor %rax, (%rsp)"); // rax = (a ^ p) ^ a = p
  asm("xor (%rsp), %rax");
  asm("retq");
}

struct ef{
  uint64_t ec;
  uint64_t ip;
  uint64_t cs;
  uint64_t rf;
  uint64_t sp;
  uint64_t ss;
};

/*
  Exception frame:

  high mem, low stack
...
  5 SS
  4 RST
  3 RFLAGS
  2 CS
  1 RIP
  0 Error code <- sp
...
  low mem, high stack
 */

#define MY_PUSH_REGS                            \
  __asm__("\
  pushq   %rdi		/* pt_regs->di */ \n\t\
  pushq   %rsi		/* pt_regs->si */ \n\t\
  pushq	  %rdx		/* pt_regs->dx */ \n\t\
  pushq   %rcx		/* pt_regs->cx */ \n\t\
  pushq   %rax		/* pt_regs->ax */ \n\t\
  pushq   %r8		/* pt_regs->r8 */ \n\t\
  pushq   %r9		/* pt_regs->r9 */ \n\t\
  pushq   %r10		/* pt_regs->r10 */ \n\t\
  pushq   %r11		/* pt_regs->r11 */ \n\t\
  pushq	  %rbx		/* pt_regs->rbx */ \n\t\
  pushq	  %rbp		/* pt_regs->rbp */ \n\t\
  pushq	  %r12		/* pt_regs->r12 */ \n\t\
  pushq	  %r13		/* pt_regs->r13 */ \n\t\
  pushq	  %r14		/* pt_regs->r14 */ \n\t\
  pushq	  %r15		/* pt_regs->r15 */ \
");
#define MY_POP_REGS                             \
  __asm__("\
	popq %r15 \n\t\
	popq %r14 \n\t\
	popq %r13 \n\t\
	popq %r12 \n\t\
	popq %rbp \n\t\
	popq %rbx \n\t\
	popq %r11 \n\t\
	popq %r10 \n\t\
	popq %r9  \n\t\
	popq %r8  \n\t\
	popq %rax \n\t\
	popq %rcx \n\t\
	popq %rdx \n\t\
	popq %rsi \n\t\
	popq %rdi \
");

#define MAKE_FAKE_EF                             \
  __asm__("\
                   /*Pretend excep frame*/                              \
  pushq   $0xbad5                                                \n\t   \
  pushq   $0xbad4                                                \n\t   \
  pushq   $0xbad3                                                \n\t   \
  pushq   $0xbad1                                                \n\t   \
  pushq   $0xbad1                                                \n\t   \
  pushq   $0xbad0                                                 \n\t  \
");

#define DEFINE_TF_INTERPOSER \
__asm__("\
                  /*prologue*/                                       \
  .text                                                          \n\t\
  .align 16                                                      \n\t\
  .globl \t tf_interposer_asm                                    \n\t\
  tf_interposer_asm:                                             \n\t\
");

extern void tf_interposer_asm();
// NOTE Define fn in assembly
DEFINE_TF_INTERPOSER

// NOTE Produce fake exception frame
MAKE_FAKE_EF

// NOTE Want to pass ef pointer to interposer C code
__asm__(" \
  pushq %rdi                 /*Preserve rdi */ \n\t\
  movq %rsp, %rdi            /*Get rsp for 1st arg to c fn */ \n\t\
  add $8, %rdi               /* Push set us back 8 */ \n\t");

// NOTE Save all regs.
MY_PUSH_REGS

// NOTE Call into C code
__asm__("  call sym_tf_set_user_bit");

// NOTE Restore regs
MY_POP_REGS

__asm__("                     \
  popq %rdi                   /*Done with 1st arg, restore user rdi  */ \n\t\
  addq $48, %rsp              /*Forget ef */ \n\t\
  ret                         /*Branch to real pfh */ ");


// XXX this fn must be on the same page as tf_interposer_asm
void sym_tf_set_user_bit(struct ef * s){
  /* Are we an instruction fetch? */
  if( s->ec & INS_FETCH){
    // We don't need to special case when in ring 3.
    if(! (s->ec & USER_FT) )  {
        // Are we user code?
        // Could look in cr2, but by def rip caused the fault here.
        // This is modeled after kern:fault_in_kernel_space
        if( s->ip < ( (1UL << 47) - PG_SZ) ){
          /// Lie that code was running in user mode.
          s->ec |= USER_FT;
          /* myprintk("swinging err code for\n"); */
          /* print_ef(); */
          /* myprintki("my_ctr %d\n", my_ctr++); */
        }
      }
  }

  printf("ss %lx\n", ((struct ef *)s)->ss);
  printf("sp %lx\n", ((struct ef *)s)->sp);
  printf("rf %lx\n", ((struct ef *)s)->rf);
  printf("cs %lx\n", ((struct ef *)s)->cs);
  printf("ip %lx\n", ((struct ef *)s)->ip);
  printf("ec %lx\n", ((struct ef *)s)->ec);
}
/* push %rax \n\t\ */
/* mov $0xffffffff81e00ac0, %rax      \n\t\ */
/* xor (%rsp), %rax \n\t\ */
/* xor %rax, (%rsp) \n\t\ */
/* xor (%rsp), %rax \n\t\ */
void alloc(){

  unsigned int codeBytes = 4096;
  void * virtualCodeAddress = 0;

  virtualCodeAddress = mmap(
                            NULL,
                            codeBytes,
                            PROT_READ | PROT_WRITE | PROT_EXEC,
                            MAP_ANONYMOUS | MAP_PRIVATE,
                            0,
                            0);

  printf("virtualCodeAddress = %p\n", virtualCodeAddress);

  // write some code in
  unsigned char * tempCode = (unsigned char *) (virtualCodeAddress);

  memcpy(tempCode, tf_interposer_asm, 4096 );

  // TODO Flush data cache: flush, write barrier, ins barrier (cpuid)

  asmFunc myFunc = (asmFunc) (virtualCodeAddress);

  unsigned out = myFunc();

  printf("out is %x\n", out);
}

int main(){
  tf_interposer_asm();
  /* alloc(); */
}


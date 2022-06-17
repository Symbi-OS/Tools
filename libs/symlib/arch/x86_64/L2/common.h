#ifndef __ARCH_X86_64_L2_COMMON__
#define __ARCH_X86_64_L2_COMMON__


#define JUMP(TARG) __asm__("jmpq " #TARG);


#define NEW_HANDLER(FN)                      \
  __asm__(".text \n\t .align 16 \n\t .globl \t" #FN "\n\t" #FN ":");

#define GET_EXCP_FRAME __asm__("movq %rsp, ef");

#define GET_PT_REG_PTR  __asm__("movq %rsp, %rdi");

#define RET_TO_PG_FT                            \
  __asm__(" push   %rax                     \n\t\
  mov    $0xffffffff81e00ac0,%rax           \n\t\
  xor    (%rsp),%rax /*Arlo's trick*/       \n\t\
  xor    %rax,(%rsp)                        \n\t\
  xor    (%rsp),%rax                        \n\t\
  ret                                           \
");

#define PUSH_REGS \
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


#define POP_REGS \
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

struct excep_frame{
  uint64_t err;
  uint64_t rip;
  uint64_t cs;
  uint64_t flag;
  uint64_t rsp;
  uint64_t ss;
};

struct pt_regs {
  /*
   * C ABI says these regs are callee-preserved. They aren't saved on kernel entry
   * unless syscall needs a complete, fully filled "struct pt_regs".
   */
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t rbp;
	uint64_t rbx;
  /* These regs are callee-clobbered. Always saved on kernel entry. */
  uint64_t r11;
  uint64_t r10;
  uint64_t r9;
  uint64_t r8;
  uint64_t rax;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
  /*
   * On syscall entry, this is syscall#. On CPU exception, this is error code.
   * On hw interrupt, it's IRQ number:
   * on int3 -> #bp no error code pushed (all traps?).
   */
  union{
    uint64_t error_code;
    uint64_t syscall_num;
    uint64_t irq_num;
  };
  /* Return frame for iretq */
	uint64_t rip;
	uint64_t cs;
	uint64_t flags;
	uint64_t rsp;
	uint64_t ss;
  /* top of stack page */
};

static_assert(sizeof(struct pt_regs) == (21 * 8), "Size of pt_regs is not correct");
struct rs_struct {
  struct pt_regs pt_r;
  uint64_t dr_hit;
  uint64_t dr7;
};

struct scratchpad {
  struct rs_struct get;
  struct rs_struct set;
  struct rs_struct control;
  uint8_t debug;
  uint8_t cnt;
  uint64_t pid;
};

#define CALL_TARG(FN) \
__asm__("call " #FN);

#define GET_CR3(VAR) __asm__("movq %%cr3,%0" : "=r"( VAR ));

#endif

#ifndef __ARCH_X86_64_L2_COMMON__
#define __ARCH_X86_64_L2_COMMON__


#define MY_JUMP(TARG) __asm__("jmpq " #TARG);


#define MY_NEW_HANDLER(FN)                      \
  __asm__(".text \n\t .align 16 \n\t .globl \t" #FN "\n\t" #FN ":");

#define MY_GET_EXCP_FRAME __asm__("movq %rsp, ef");

#define MY_PUSH_REGS \
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


#define MY_POP_REGS \
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

/* #define MY_NEW_HANDLER(FN)                      \ */
/*   __asm__("\ */
/*      .text \n\t                                 \ */
/*      .align 16 \n\t\ "                          \ */
/*           #FN ":\ */
/* "); */

#define MY_MY_CALL(FN) \
__asm__("call " #FN);

#define GET_CR3(VAR) __asm__("movq %%cr3,%0" : "=r"( VAR ));

#endif

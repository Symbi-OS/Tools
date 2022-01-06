#ifndef __ARCH_X86_64_SYM_LIB_PAGE_FAULT__
#define __ARCH_X86_64_SYM_LIB_PAGE_FAULT__

// Do an indirect jump to target. Here we stringify TARG and concatenate it with jmpq.
#define MY_JUMP(TARG) __asm__("jmpq " #TARG);


#define MY_NEW_HANDLER(FN)                      \
  __asm__("\
     .text \n\t                                 \
     .align 16 \n\t\ "                          \
          #FN ":\
");

#define MY_GET_EXCP_FRAME __asm__("movq %rsp, ef");

/* #define MY_PUSH_REGS \ */
/*   __asm__("\ */
/* ") */


/* .macro PUSH_REGS rdx=%rdx rax=%rax save_ret=0 */
/*     .if \save_ret */
/*     pushq	%rsi		/\* pt_regs->si *\/ */
/*     movq	8(%rsp), %rsi	/\* temporarily store the return address in %rsi *\/ */
/*     movq	%rdi, 8(%rsp)	/\* pt_regs->di (overwriting original return address) *\/ */
/*     .else */
/*     pushq   %rdi		/\* pt_regs->di *\/ */
/*     pushq   %rsi		/\* pt_regs->si *\/ */
/*     .endif */
/*     pushq	\rdx		/\* pt_regs->dx *\/ */
/*     pushq   %rcx		/\* pt_regs->cx *\/ */
/*     pushq   \rax		/\* pt_regs->ax *\/ */
/*     pushq   %r8		/\* pt_regs->r8 *\/ */
/*     pushq   %r9		/\* pt_regs->r9 *\/ */
/*     pushq   %r10		/\* pt_regs->r10 *\/ */
/*     pushq   %r11		/\* pt_regs->r11 *\/ */
/*     pushq	%rbx		/\* pt_regs->rbx *\/ */
/*     pushq	%rbp		/\* pt_regs->rbp *\/ */
/*     pushq	%r12		/\* pt_regs->r12 *\/ */
/*     pushq	%r13		/\* pt_regs->r13 *\/ */
/*     pushq	%r14		/\* pt_regs->r14 *\/ */
/*     pushq	%r15		/\* pt_regs->r15 *\/ */
/*     UNWIND_HINT_REGS */

/*     .if \save_ret */
/*     pushq	%rsi		/\* return address on top of stack *\/ */
/*     .endif */
/*     .endm */


#endif


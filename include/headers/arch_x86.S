#ifdef CONFIG_X86_64
// Stolen from linux
/* /home/sym/Symbi-OS/linux/arch/x86/entry/calling.h */


/*
 * 64-bit system call stack frame layout defines and helpers,
 * for assembly code:
 */

.macro UNWIND_HINT_REGS
.endm

.macro PUSH_REGS rdx=%rdx rax=%rax save_ret=0
	.if \save_ret
	pushq	%rsi		/* pt_regs->si */
	movq	8(%rsp), %rsi	/* temporarily store the return address in %rsi */
	movq	%rdi, 8(%rsp)	/* pt_regs->di (overwriting original return address) */
	.else
	pushq   %rdi		/* pt_regs->di */
	pushq   %rsi		/* pt_regs->si */
	.endif
	pushq	\rdx		/* pt_regs->dx */
	pushq   %rcx		/* pt_regs->cx */
	pushq   \rax		/* pt_regs->ax */
	pushq   %r8		/* pt_regs->r8 */
	pushq   %r9		/* pt_regs->r9 */
	pushq   %r10		/* pt_regs->r10 */
	pushq   %r11		/* pt_regs->r11 */
	pushq	%rbx		/* pt_regs->rbx */
	pushq	%rbp		/* pt_regs->rbp */
	pushq	%r12		/* pt_regs->r12 */
	pushq	%r13		/* pt_regs->r13 */
	pushq	%r14		/* pt_regs->r14 */
	pushq	%r15		/* pt_regs->r15 */
	UNWIND_HINT_REGS

	.if \save_ret
	pushq	%rsi		/* return address on top of stack */
	.endif
.endm

.macro CLEAR_REGS
	/*
	 * Sanitize registers of values that a speculation attack might
	 * otherwise want to exploit. The lower registers are likely clobbered
	 * well before they could be put to use in a speculative execution
	 * gadget.
	 */
	xorl	%edx,  %edx	/* nospec dx  */
	xorl	%ecx,  %ecx	/* nospec cx  */
	xorl	%r8d,  %r8d	/* nospec r8  */
	xorl	%r9d,  %r9d	/* nospec r9  */
	xorl	%r10d, %r10d	/* nospec r10 */
	xorl	%r11d, %r11d	/* nospec r11 */
	xorl	%ebx,  %ebx	/* nospec rbx */
	xorl	%ebp,  %ebp	/* nospec rbp */
	xorl	%r12d, %r12d	/* nospec r12 */
	xorl	%r13d, %r13d	/* nospec r13 */
	xorl	%r14d, %r14d	/* nospec r14 */
	xorl	%r15d, %r15d	/* nospec r15 */

.endm

.macro PUSH_AND_CLEAR_REGS rdx=%rdx rax=%rax save_ret=0
	PUSH_REGS rdx=\rdx, rax=\rax, save_ret=\save_ret
	CLEAR_REGS
.endm

.macro POP_REGS pop_rdi=1 skip_r11rcx=0
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbp
	popq %rbx
	.if \skip_r11rcx
	popq %rsi
	.else
	popq %r11
	.endif
	popq %r10
	popq %r9
	popq %r8
	popq %rax
	.if \skip_r11rcx
	popq %rsi
	.else
	popq %rcx
	.endif
	popq %rdx
	popq %rsi
	.if \pop_rdi
	popq %rdi
	.endif
.endm

.macro NEW_HANDLER name
 .text
 .align 16
 \name:
.endm

.macro MY_JUMP tar
  jmp \tar
.endm

.macro MY_CALL tar
  call \tar
.endm


.macro GET_EXCP_FRAME
  movq %rsp, ef
.endm

.macro DECREMENT_RETURN_RIP
  decq (%rsp)
.endm

.macro MY_IRET
  iretq
.endm

#endif //CONFIG_X86_64

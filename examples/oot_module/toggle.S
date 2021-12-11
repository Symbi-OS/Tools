/*
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ali Raza");
MODULE_DESCRIPTION("Divide by zero");
MODULE_VERSION("1.0");
*/

#include <asm/segment.h>

	.globl sym_lower
	.globl sym_elevate

sym_lower:
	movq	%rsp, %r15
	movq	(%rsp), %r14
	addq	$8, %r15

	pushq	$__KERNEL_DS
	pushq	%r15
	pushfq
	pushq	$__USER_CS
	pushq	%r14

	iretq


sym_elevate:
	movq	%rsp, %r15
	movq	(%rsp), %r14
	addq	$8, %r15

	pushq	$__KERNEL_DS
	pushq	%r15
	pushfq
	pushq	$__KERNEL_CS
	pushq	%r14

	iretq

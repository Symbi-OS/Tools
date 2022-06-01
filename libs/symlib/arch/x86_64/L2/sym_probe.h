#ifndef __ARCH_X86_64_SYM_PROBE__
#define __ARCH_X86_64_SYM_PROBE__

// License C 2021-
// Author: Thomas Unger
// Level: 2

#include "./common.h"

#define MY_DECREMENT_RETURN_RIP __asm__("decq (%rsp)");
#define IRET __asm__("iretq");

#define PUSH_FAKE_ERROR __asm__("push $0xbadbad");
#define GET_PT_REG_PTR  __asm__("movq %rsp, %rdi");
#define DROP_FAKE_ERROR __asm__("add $8, %rsp");

#define X86_TRAP_BP		 3

#endif

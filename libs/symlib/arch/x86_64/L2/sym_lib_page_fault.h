#ifndef __ARCH_X86_64_SYM_LIB_PAGE_FAULT__
#define __ARCH_X86_64_SYM_LIB_PAGE_FAULT__
#include "./common.h"

// Depricated, but maybe useful sometime.
#if 0
#define DEFINE_TF_INTERPOSER \
__asm__("\
                  /*prologue*/                                       \
  .text                                                          \n\t\
  .align 16                                                      \n\t\
  .globl \t tf_interposer_asm                                    \n\t\
  tf_interposer_asm:                                             \n\t\
");

// NOTE Define fn in assembly
DEFINE_TF_INTERPOSER

// NOTE Want to pass ef pointer to interposer C code
__asm__(" \
  pushq %rdi                 /*Preserve rdi */ \n\t\
  movq %rsp, %rdi            /*Get rsp for 1st arg to c fn */ \n\t\
  add $8, %rdi               /* Push set us back 8 */ \n\t");

// NOTE Save all regs.
PUSH_REGS

// NOTE Call into C code
__asm__("  call sym_tf_set_user_bit");

// NOTE Restore regs
POP_REGS

__asm__("                     \
  popq %rdi                   /*Done with 1st arg, restore user rdi  */ \n\t\
\
  push   %rax                      \n\t\
  mov    $0xffffffff81e00ac0,%rax  /* asm_exc_page_fault */ \n\t   \
  xor    (%rsp),%rax               /*Arlo's trick*/ \n\t\
  xor    %rax,(%rsp)               \n\t\
  xor    (%rsp),%rax               \n\t\
  ret \
");



// 6 = user + write
// Rest is to call into 8 byte address without clobbering any registers.
// Push random reg to stack, put addr in that reg & swap w/o dirtying a reg.
// Tell me if there's an easier way!
__asm__("\
  .text \n\t\
  .align 16 \n\t\
  .globl \t df_asm_handler \n\t\
  df_asm_handler: \n\t\
  movq   $0x6,(%rsp) \n\t\
  push   %rax \n\t\
  mov    $0xffffffff81e00ac0,%rax \n\t\
  xor    (%rsp),%rax /*Arlo's trick*/ \n\t\
  xor    %rax,(%rsp) \n\t\
  xor    (%rsp),%rax \n\t\
  ret \
");




// Text fault handler
// Save rsi
// move ptr to ef into rsi
// set user bit of error code to lie to kernel
// Store error code back in ef
// slide into normal pf handler
__asm__("\
  .text \n\t\
  .align 16 \n\t\
  .globl \t tf_asm_handler \n\t\
  tf_asm_handler: \n\t\
  pushq %rsi                    \n\t\
  movq 8(%rsp),%rsi             \n\t\
  orq $0x4, %rsi                \n\t\
  movq %rsi, 8(%rsp)            \n\t\
  popq %rsi                     \n\t\
  mov    $0xffffffff81e00ac0,%rax \n\t\
  xor    (%rsp),%rax /*Arlo's trick*/ \n\t\
  xor    %rax,(%rsp) \n\t\
  xor    (%rsp),%rax \n\t\
  ret \
");
#endif

#endif


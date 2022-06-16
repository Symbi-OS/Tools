#ifndef __ARCH_X86_64_SYM_PROBE__
#define __ARCH_X86_64_SYM_PROBE__

// License C 2021-
// Author: Thomas Unger
// Level: 2

#include "./common.h"

#define DECREMENT_RETURN_RIP __asm__("decq (%rsp)");
#define IRET __asm__("iretq");

#define PUSH_FAKE_ERROR __asm__("push $0xbadbad");
#define DROP_FAKE_ERROR __asm__("add $8, %rsp");

#define X86_TRAP_BP		 3
#define X86_TRAP_DB		 1

struct DR6 {
  union {
    uint64_t val;
    struct {
      uint64_t B0 : 1, B1 : 1, B2 : 1, B3 : 1, Res : 9, BD : 1, BS : 1, BT : 1;
    };
  }__attribute__((packed));
};
static_assert(sizeof(struct DR6) == 8, "Size of DR6 is not correct");

struct DR7 {
  union {
    uint64_t val;
    struct {
      uint64_t L0 : 1, G0 : 1, L1 : 1, G1 : 1, L2 : 1, G2 : 1, L3 : 1, G3 : 1,
          LE : 1, GE : 1, : 3, GD : 1, : 2, RW0 : 2, LEN0 : 2, RW1 : 2,
          LEN1 : 2, RW2 : 2, LEN2 : 2, RW3 : 2, LEN3 : 2, : 32;
    };
  }__attribute__((packed));
};
static_assert(sizeof(struct DR7) == 8, "Size of DR7 is not correct");

#endif

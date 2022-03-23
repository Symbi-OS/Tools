#ifndef __SYM_LIB_PAGE_FAULT__
#define __SYM_LIB_PATE_FAULT__

#include "L0/sym_structs.h"
#include "L0/sym_lib.h"

#ifdef CONFIG_X86_64
#include "../../arch/x86_64/L2/sym_lib_page_fault.h"
#endif
// License C 2021-
// Author: Thomas Unger
// Level: 2

/*

Exception frame 0 -> 0 Interrupt;
-----------------------
SS         (RSP + 40)
RSP        (RSP + 32)
RFLAGS     (RSP + 24)
CS         (RSP + 16)
RIP        (RSP + 8)
ERROR CODE (<- RSP)
-----------------------


Page fault error codes: 
 31              15                             4               0
+---+--  --+---+-----+---+--  --+---+----+----+---+---+---+---+---+
|   Reserved   | SGX |   Reserved   | SS | PK | I | R | U | W | P |
+---+--  --+---+-----+---+--  --+---+----+----+---+---+---+---+---+

Legend: Set (Clear)
0 Present (not)
1 Write (Read)
2 User (Supervisor)
3 Reserved bit set in page directory (not)
4 Inst fetch

Our code for interposing on text fault.
.text                       Lives in text.
.align 16                   All the rest look 16 byte aligned.
bs_asm_exc_page_fault:      Our handler, will be placed in IDT.
pushq %rsi                  Preserve old rsi (Think just a user reg).
movq 8(%rsp),%rsi           Grab error code from HW pushed exception frame.
orq $0x4, %rsi              Set user bit lying that we were in ring 3
movq %rsi, 8(%rsp)          Throw val back in error code on stack.
popq %rsi                   Restore user rsi.
jmp *orig_asm_exc_page_fault
*/

// NOTE: Haven't really thought about stringification
#define MY_FINAL_HANDLER(LAB, TARG, OLD_HAND) \
  MY_NEW_HANDLER(LAB)                         \
  MY_GET_EXCP_FRAME                           \
  MY_PUSH_REGS                                \
  MY_MY_CALL(TARG)                            \
  MY_POP_REGS                                 \
  MY_JUMP(OLD_HAND)

// NOTE Err codes:
#define PRESENT 1
#define WR_FT   1<<1
#define USER_FT 1<<2
#define INS_FETCH 1<<4

// Interrupt vectors
#define PG_FT_IDX 14
#define DF_IDX 8

struct pte{
  /* union { */
    uint64_t
    SEL : 1,
      RW : 1,
      US : 1,
      PWT: 1,
      PCD: 1,
      A : 1,
      DIRTY: 1,
      MAPS : 1,
      WHOCARES2 : 4,
      PG_TBL_ADDR : 40,
      RES : 11,
      XD  : 1;
    /* uint64_t raw; */
  /* }; */
}; //__attribute__((__packed__))
static_assert(sizeof(struct pte) ==8, "Size of pte is not correct");

extern void c_handler_page_fault();
extern void df_asm_handler();

void sym_print_pte(struct pte *pte);
void sym_lib_page_fault_init();

extern struct pte * sym_get_pte(uint64_t addr, unsigned int *level);

// Execute disable
static inline int sym_is_pte_execute_disable(struct pte *pte) {
  sym_elevate(); int ret = pte->XD; sym_lower();
  return ret;
}

static inline void sym_set_pte_execute_disable(struct pte *pte) {
  sym_elevate(); pte->XD = 1; sym_lower();
}

static inline void sym_clear_pte_execute_disable(struct pte *pte) {
  sym_elevate(); pte->XD = 0; sym_lower();
}

// TODO these should all take ptrs right?
static inline int sym_is_pte_writeable(struct pte pte) {
  return pte.RW;
}

static inline void sym_set_pte_writeable(struct pte *pte) {
  pte->RW = 1;
}
static inline void sym_clear_pte_writeable(struct pte *pte) {
  pte->RW = 0;
}

static inline int sym_is_pte_readaable(struct pte pte) {
  return pte.RW == 0;
}

struct excep_frame{
  uint64_t err;
  uint64_t rip;
  uint64_t cs;
  uint64_t flag;
  uint64_t rsp;
  uint64_t ss;
};

void sym_make_pg_writable(uint64_t addr);
void sym_make_pg_unwritable(uint64_t addr);

/* void sym_interpose_on_pg_ft(unsigned char* new_idt); */
void sym_interpose_on_pg_ft_c(unsigned char* new_idt);

void sym_interpose_on_df_c(unsigned char* new_idt);
void sym_interpose_on_df_asm(unsigned char* new_idt, unsigned char* handler_pg);

void sym_make_pg_ft_use_ist(unsigned char *my_idt);

// TODO figure out if this should be in l1
void sym_toggle_pg_ft_ist(unsigned char *my_idt, unsigned int enable);

#endif

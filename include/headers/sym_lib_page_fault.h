#ifndef __SYM_LIB_PAGE_FAULT__
#define __SYM_LIB_PATE_FAULT__
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
#define PG_FT_IDX 14

void sym_interpose_on_pg_ft(char* new_idt);

#endif

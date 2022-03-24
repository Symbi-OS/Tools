#ifndef __SYM_STRUCTS
#define __SYM_STRUCTS

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>

#define PG_SZ (1ULL << 12)
#define IDT_SZ_BYTES PG_SZ

// Descriptor table entry. For use with GDT, IDT
struct dtr
{
  uint16_t limit; // limit 2
  uint64_t base;   // base 8
}__attribute__((packed));
static_assert(sizeof(struct dtr) == 10, "Size of dtr is not correct");

union idt_addr{
  struct{
    uint16_t lo;
    uint16_t mid;
    uint32_t hi;
  } dcmp;
    uint64_t raw;
}__attribute__((packed));
static_assert(sizeof(union idt_addr) == 8, "Size of idt_addr is not correct");

// IDT interrupt descriptor.
union idt_desc{

  struct {
    uint64_t lo;
    uint64_t hi;
  } raw;

  struct {
    // Low 2 bytes
    uint16_t  lo_addr;

    // segment selector
    uint16_t  seg_sel;

    // IS
    unsigned ist : 3, zero0 : 5, type : 5, dpl : 2, p : 1;
    // Zero
    /* uint16_t zero: 8; */
    /* uint16_t type: 4; */
    /* uint16_t zero2: 1; */
    /* uint16_t dpl: 2; */
    /* uint16_t p:1; */

    uint16_t  mid_addr;
    uint32_t hi_addr;

    uint32_t res;
  } fields;
}__attribute__((packed));
static_assert(sizeof(union idt_desc) == 16, "Size of idt_desc is not correct");
#endif

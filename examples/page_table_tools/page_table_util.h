#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PAGE_SIZE        4096
#define NUM_ENTRIES      512
#define PAGE_PRESENT     0x1

struct page_table_entry {
    union
    {
        struct
        {
            uint64_t present                : 1;    // Must be 1, region invalid if 0.
            uint64_t read_write             : 1;    // If 0, writes not allowed.
            uint64_t user_supervisor        : 1;    // If 0, user-mode accesses not allowed.
            uint64_t page_write_through     : 1;    // Determines the memory type used to access the memory.
            uint64_t page_cache_disabled    : 1;    // Determines the memory type used to access the memory.
            uint64_t accessed               : 1;    // If 0, this entry has not been used for translation.
            uint64_t dirty                  : 1;    // If 0, the memory backing this page has not been written to.
            uint64_t page_access_type       : 1;    // Determines the memory type used to access the memory.
            uint64_t global                 : 1;    // If 1 and the PGE bit of CR4 is set, translations are global.
            uint64_t ignored2               : 3;
            uint64_t page_frame_number      : 36;   // The page frame number of the backing physical page.
            uint64_t reserved               : 4;
            uint64_t ignored3               : 7;
            uint64_t protection_key         : 4;    // If the PKE bit of CR4 is set, determines the protection key.
            uint64_t execute_disable        : 1;    // If 1, instruction fetches not allowed.
        };
        uint64_t value;
    };
} __attribute__((packed));

struct page_table {
    struct page_table_entry entries[512];
} __attribute__((aligned(PAGE_SIZE)));

void print_page_table_entry(struct page_table_entry* entry);
void* get_current_task();

extern void* get_task_base_vma(void* task);
extern void* get_next_vma(void* vma);

extern uint64_t get_task_vma_start(void* vma);
extern uint64_t get_task_vma_end(void* vma);

extern struct page_table_entry* get_pte_for_address(void* task, uint64_t addr);

extern void fill_page_table_info_for_address(
    void* task,
    uint64_t addr,
    struct page_table_entry* opgd,
    struct page_table_entry* op4d,
    struct page_table_entry* opud,
    struct page_table_entry* opmd,
    struct page_table_entry* opte
);

extern void make_pte_readonly(struct page_table_entry* pte);
extern void flush_tlb();

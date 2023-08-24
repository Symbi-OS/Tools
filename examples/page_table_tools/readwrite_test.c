#include "page_table_util.h"
#include <LINF/sym_all.h>

const unsigned char myCharArr[PAGE_SIZE] __attribute__((aligned (PAGE_SIZE)));

void change_pte_readwrite_permissions() {
    sym_elevate();
    void* current_task = get_current_task();
    
    struct page_table_entry* pte = get_pte_for_address(current_task, (uint64_t)myCharArr);
    if (!pte) {
        printf("Failed to get pte for test buffer address\n");
        return;
    }
    assert((pte->read_write == 0));

    pte->read_write = 1;
    flush_tlb();

    sym_lower();
}

void read_and_print_char_value() {
    printf("Read byte value: '%c'\n", *myCharArr);
}

void modify_char_value() {
    *((unsigned char*)myCharArr) = 'x';
}

int main(int argc, char** argv) {
    (void)argv;

    read_and_print_char_value();

    // Apply the readwrite permission
    // modification through the PTE.
    if (argc > 1) {
        change_pte_readwrite_permissions();
    }

    modify_char_value();
    read_and_print_char_value();

    return 0;
}

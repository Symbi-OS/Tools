#include "page_table_util.h"
#include <LINF/sym_all.h>

void walk_pagetable(int should_print_pte) {
    uint64_t pages_visited = 0;
    uint64_t pages_present = 0;
    void* current_task = get_current_task();
    
    void* vma = get_task_base_vma(current_task);
    while (vma) {
        uint64_t vm_start = get_task_vma_start(vma);
        uint64_t vm_end = get_task_vma_end(vma);

        if (should_print_pte) {
            printf("vma->vm_start : 0x%lx\n", vm_start);
            printf("vma->vm_end   : 0x%lx\n", vm_end);
        }

        for (uint64_t vmpage = vm_start; vmpage < vm_end; vmpage += PAGE_SIZE) { 
            struct page_table_entry* pte = get_pte_for_address(current_task, vmpage);
            if (!pte)
                continue;

            ++pages_visited;

            if (pte->present) {
                ++pages_present;
                if (should_print_pte) {
                    print_page_table_entry(pte);
                }
            }
        }

        vma = get_next_vma(vma);
    }

    printf("\n==============================\n");
    printf("----- Visited %li Pages -----\n", pages_visited);
    printf("     Pages Present : %li\n", pages_present);
    printf("==============================\n");
}

int main() {
    sym_elevate();

    walk_pagetable(0);

    unsigned char* ptr = (unsigned char*)malloc(5 * PAGE_SIZE);
    ptr[1 * PAGE_SIZE - 1] = 'h';
    ptr[2 * PAGE_SIZE - 1] = 'e';
    ptr[3 * PAGE_SIZE - 1] = 'l';
    ptr[4 * PAGE_SIZE - 1] = 'l';
    ptr[5 * PAGE_SIZE - 1] = 'o';

    walk_pagetable(0);

    free(ptr);
    sym_lower();

    return 0;
}

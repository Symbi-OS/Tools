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

    printf("----- Visited %li Pages -----\n", pages_visited);
    printf("     Pages Present : %li\n\n", pages_present);
}

int main(int argc, char** argv) {
    int pages_to_allocate = 5;
	if (argc > 1) {
		pages_to_allocate = atoi(argv[1]);
	}
	
	sym_elevate();

    walk_pagetable(0);

	// Allocating more pages of memory
	printf("Allocating %i bytes (%i pages)...\n\n", pages_to_allocate * PAGE_SIZE, pages_to_allocate);
    unsigned char* ptr = (unsigned char*)malloc(pages_to_allocate * PAGE_SIZE);
	
	// Touching the pages to make them present
	for (int i = 1; i <= pages_to_allocate; ++i) {
		ptr[i * PAGE_SIZE - 1] = 'x';
	}

    walk_pagetable(0);

    free(ptr);
    sym_lower();

    return 0;
}

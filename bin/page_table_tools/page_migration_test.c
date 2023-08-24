#include "page_table_util.h"
#include <LINF/sym_all.h>
typedef void*(*vzalloc_t)(unsigned long);

int globalUserIntArr[PAGE_SIZE / sizeof(int)] __attribute__((aligned (PAGE_SIZE))) = { 4554, 0 };

void* get_aligned_kern_pg() {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
	vzalloc_t vzalloc = (vzalloc_t) sym_get_fn_address("vzalloc");
#pragma GCC diagnostic pop
	
	SYM_ON_KERN_STACK_DO( \
		void* p = vzalloc(PAGE_SIZE);\
	);

	// Re-elevate	
	sym_elevate();
	
	assert( ((long unsigned )p % PAGE_SIZE ) == 0);
	return p;
}

int read_global_user_int() {
	return globalUserIntArr[0];
}

void write_global_user_int(int val) {
	globalUserIntArr[0] = val;
}

int read_kernel_int(void* kern_page) {
	return *((int*)kern_page);
}

void redirect_user_global_int_pte(void* kern_page) {
	void* current_task = get_current_task();

    struct page_table_entry* user_pte = get_pte_for_address(current_task, (uint64_t)globalUserIntArr);
    if (!user_pte) {
        printf("Failed to get pte for test address\n");
        return;
    }

	struct page_table_entry* kern_page_pte = get_pte_for_address(current_task, (uint64_t)kern_page);
    if (!kern_page_pte) {
        printf("Failed to get pte for kernel page\n");
        return;
    }
	
	// Redirect the page frame number and make page writable
	user_pte->page_frame_number = kern_page_pte->page_frame_number;
	user_pte->read_write = 1;

	// Flush the TLB
	flush_tlb();

	printf("PTE redirected to a kernel page...\n");
}

int main() {
    /*
    Abstract Idea:
        - allocate an integer and set its value to 4554 (global user variable PAGE ALIGNED! make sure nothing else is there)
        - create a temporary local (user stack) int variable and set it to the value of global variable
        - allocate a kernel page
        - change the PTE of global variable to point to the kernel page
        - write the temp int variable into the global
        - read from the kernel page address to see if it got copied :D

        *Notes* might need to be on the kernel stack
    */

	// Read and confirm the value of the global variable
	printf("Value of the global variable: %i\n", read_global_user_int());
	
	// Allocate an integer on the local user stage
	// with the same value as the global user variable.
	int localUserInt = globalUserIntArr[0];

	// Allocate an aligned kernel page
	sym_elevate();
	void* kern_page = get_aligned_kern_pg();
	printf("Allocated kernel page: %p\n", kern_page);
	printf("Value on kernel page: %i\n", read_kernel_int(kern_page));

	redirect_user_global_int_pte(kern_page);
	printf("Value of the global variable: %i\n", read_global_user_int());

	printf("Writing '37' into kernel page...\n");
	((int*)kern_page)[0] = 37;

	printf("Value on kernel page: %i\n", read_kernel_int(kern_page));
	printf("Value of the global variable: %i\n", read_global_user_int());

	printf("Writing in the saved local value '%i' into global var...\n", localUserInt);
	write_global_user_int(localUserInt);

	printf("Value of the global variable: %i\n", read_global_user_int());
	printf("Value on kernel page: %i\n", read_kernel_int(kern_page));

    return 0;
}

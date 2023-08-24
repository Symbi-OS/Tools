#include "page_table_util.h"
#include <LINF/sym_all.h>
typedef void*(*vzalloc_t)(unsigned long);
typedef void(*vfree_t)(void*);

int globalUserIntArr[PAGE_SIZE / sizeof(int)] __attribute__((aligned (PAGE_SIZE))) = { 0 };

uint64_t g_preserved_user_pfn = 0;

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

void free_kern_page(void* kern_page) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
	vfree_t vfree = (vfree_t) sym_get_fn_address("vfree");
#pragma GCC diagnostic pop

	sym_elevate();
	vfree(kern_page);
	sym_lower();
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

	// Preserve the initial user page frame number
	g_preserved_user_pfn = user_pte->page_frame_number;
	
	// Redirect the page frame number and make page writable
	user_pte->page_frame_number = kern_page_pte->page_frame_number;

	// Flush the TLB
	flush_tlb();

	printf("PTE redirected to a kernel page...\n");
}

void restore_user_global_int_pfn() {
	void* current_task = get_current_task();

    struct page_table_entry* user_pte = get_pte_for_address(current_task, (uint64_t)globalUserIntArr);
    if (!user_pte) {
        printf("Failed to get pte for test address\n");
        return;
	}

	// Restore the original page frame number to the user physical page
	user_pte->page_frame_number = g_preserved_user_pfn;

	// Flush the TLB
	flush_tlb();
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

    // Write in the initial value
    write_global_user_int(4554);

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

	sym_lower();
	printf("[after sym_lower] Writing '6000' into global var...\n");
	write_global_user_int(6000);
	printf("[after sym_lower] Value of the global variable: %i\n", read_global_user_int());

	printf("Restoring user PTE...\n");
	sym_elevate();
	restore_user_global_int_pfn();
	sym_lower();

	printf("Value of the global variable: %i\n", read_global_user_int());

	free_kern_page(kern_page);
    return 0;
}

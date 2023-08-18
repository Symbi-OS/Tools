#include "page_table_util.h"
#include <LINF/sym_all.h>

typedef void *(*bpf_get_current_task_t)();

void* get_current_task() {
    bpf_get_current_task_t gct = NULL;

    if (gct == NULL) {
        gct = (bpf_get_current_task_t) sym_get_fn_address("bpf_get_current_task");
        if (gct == NULL) {
            fprintf(stderr, "failed to find __fdget() \n");
            exit(-1);
        }
    }

    sym_elevate();
    return gct();
}

void print_page_table_entry(struct page_table_entry* entry) {
    printf("------ page_table_entry info ------\n");
    printf("    present             : %i\n", (int)entry->present);
    printf("    read_write          : %i\n", (int)entry->read_write);
    printf("    user_supervisor     : %i\n", (int)entry->user_supervisor);
    printf("    page_write_through  : %i\n", (int)entry->page_write_through);
    printf("    page_cache_disabled : %i\n", (int)entry->page_cache_disabled);
    printf("    accessed            : %i\n", (int)entry->accessed);
    printf("    dirty               : %i\n", (int)entry->dirty);
    printf("    page_access_type    : %i\n", (int)entry->page_access_type);
    printf("    global              : %i\n", (int)entry->global);
    printf("    page_frame_number   : %lx\n",(uint64_t)entry->page_frame_number);
    printf("    protection_key      : %i\n", (int)entry->protection_key);
    printf("    execute_disable     : %i\n", (int)entry->execute_disable);
    printf("\n");
}

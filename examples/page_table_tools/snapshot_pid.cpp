extern "C" {
    #include "page_table_util.h"
    #include <LINF/sym_all.h>
}

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <signal.h>
#include "json11.hpp"

std::string to_hex_string(uint64_t value) {
    std::stringstream ss;
    ss << std::hex << value;
    return ss.str();
}

#include <iostream>
#include <map>

class ProcessPageTableSnapshot {
public:
    struct PageTableLevelRepr {
        struct page_table_entry entry;
        std::map<uint64_t, PageTableLevelRepr> children;
    };

    void update_entries_for_vaddr(
        struct page_table_entry pgd,
        struct page_table_entry p4d,
        struct page_table_entry pud,
        struct page_table_entry pmd,
        struct page_table_entry pte
    ) {
        // Populate or retrieve PGD entry
        PageTableLevelRepr& pgd_entry = root_map[pgd.page_frame_number];
        pgd_entry.entry = pgd;

        // Populate or retrieve P4D entry
        PageTableLevelRepr& p4d_entry = pgd_entry.children[p4d.page_frame_number];
        p4d_entry.entry = p4d;

        // Populate or retrieve PUD entry
        PageTableLevelRepr& pud_entry = p4d_entry.children[pud.page_frame_number];
        pud_entry.entry = pud;

        // Populate or retrieve PMD entry
        PageTableLevelRepr& pmd_entry = pud_entry.children[pmd.page_frame_number];
        pmd_entry.entry = pmd;

        // Populate or retrieve PTE entry
        PageTableLevelRepr& pte_entry = pmd_entry.children[pte.page_frame_number];
        pte_entry.entry = pte;
    }

    json11::Json to_json(const PageTableLevelRepr& entry_repr) const {
        auto& entry = entry_repr.entry;

        json11::Json::object json_object;
        json_object["pfn"] = to_hex_string(entry.page_frame_number);
        json_object["present"] = (int)entry.present;
        json_object["read_write"] = (int)entry.read_write;
        json_object["user_supervisor"] = (int)entry.user_supervisor;
        json_object["page_write_through"] = (int)entry.page_write_through;
        json_object["page_cache_disabled"] = (int)entry.page_cache_disabled;
        json_object["accessed"] = (int)entry.accessed;
        json_object["dirty"] = (int)entry.dirty;
        json_object["page_access_type"] = (int)entry.page_access_type;
        json_object["global"] = (int)entry.global;
        json_object["protection_key"] = (int)entry.protection_key;
        json_object["execute_disable"] = (int)entry.execute_disable;
      
        json11::Json::array children_json_object;
        for (const auto& child : entry_repr.children) {
            children_json_object.push_back(to_json(child.second));
        }
        json_object["children"] = children_json_object;

        return json_object;
    }

    json11::Json to_json() const {
        static struct page_table_entry dummy_entry = { .value = 0 };

        return to_json({.entry = dummy_entry, .children = root_map});
    }

    inline std::map<uint64_t, PageTableLevelRepr>& get_root_map() { return root_map; }

private:
    std::map<uint64_t, PageTableLevelRepr> root_map;
};

std::shared_ptr<ProcessPageTableSnapshot> snapshot_pagetable(
    void* task
) {
    auto snapshot = std::make_shared<ProcessPageTableSnapshot>();
    uint64_t pages_recorded = 0;

    void* vma = get_task_base_vma(task);
    while (vma) {
        uint64_t vm_start = get_task_vma_start(vma);
        uint64_t vm_end = get_task_vma_end(vma);

        for (uint64_t vmpage = vm_start; vmpage < vm_end; vmpage += PAGE_SIZE) {
            struct page_table_entry pgd, p4d, pud, pmd, pte = { .value = 0 };
            fill_page_table_info_for_address(
                task,
                vmpage,
                &pgd, &p4d, &pud, &pmd, &pte
            );

            // Make sure the PTE is valid and present
            if (pte.page_frame_number == 0 || pte.present == 0) {
                continue;
            }

            // Record the page table level entries for the snapshot
            snapshot->update_entries_for_vaddr(pgd, p4d, pud, pmd, pte);

            ++pages_recorded;
        }

        vma = get_next_vma(vma);
    }

    printf("Recorded %li\n", pages_recorded);
    return snapshot;
}

void export_pagetable_snapshot_to_file(
    std::shared_ptr<ProcessPageTableSnapshot>& snapshot,
    const std::string& filename
) {
    std::string dump_content = snapshot->to_json().dump();

    std::ofstream out(filename + ".json");
    out << dump_content;
}

int main(int argc, char** argv) {
    int current_pid = getpid();
    int target_pid = current_pid;
    std::string export_filename = "pte_dump_" + std::to_string(target_pid);

    // Set the target pid to the provided argument
	if (argc > 1) {
		int requested_target_pid = atoi(argv[1]);

        if (requested_target_pid != 0) {
            target_pid = requested_target_pid;
            export_filename = "pte_dump_" + std::to_string(target_pid);
        }
	}

    // Set the dump export filename if such is provided
    if (argc > 2) {
        export_filename = std::string(argv[2]);
    }
	
    printf("Current PID: %i\n", current_pid);
    printf("Target  PID: %i\n", target_pid);

    // printf("TEST DEBUG\n");
	sym_elevate();

    // uint64_t currentTaskBefore = *((uint64_t*)0xffffffff82e7b120);
    // uint64_t currentTaskAfter = *((uint64_t*)0xffffffff82e7b118);
    // uint64_t pidBefore = *((uint64_t*)0xffffffff82e7b110);
    // uint64_t pidAfter = *((uint64_t*)0xffffffff82e7b108);

    // sym_lower();

    // printf("currentTaskBefore  : 0x%lx\n", currentTaskBefore);
    // printf("currentTaskAfter   : 0x%lx\n", currentTaskAfter);
    // printf("pidBefore : %li\n", pidBefore);
    // printf("pidAfter  : %li\n", pidAfter);
    // return 0;

    void* target_task_struct = get_task_struct_from_pid(target_pid);
    if (!target_task_struct) {
        sym_lower();
        printf("Error> target PID could not be found\n");
        return -1;
    }

    //auto snapshot = snapshot_pagetable(target_task_struct);
    kill(target_pid, SIGSTOP);
    impersonate_syscall(target_task_struct);

    kill(target_pid, SIGCONT);
    sym_lower();

    //export_pagetable_snapshot_to_file(snapshot, export_filename);
    return 0;
}

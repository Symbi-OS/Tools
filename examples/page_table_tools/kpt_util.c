#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pid.h>
#include <asm/pgtable.h>
#include <asm/tlbflush.h>
#include <asm/desc.h>
#include <asm/segment.h>
#include <asm/switch_to.h>
#include <linux/printk.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <asm/pkru.h>
#include <asm/fpu/internal.h>
#include <asm/mmu_context.h>
#include <asm/prctl.h>
#include <asm/desc.h>
#include <asm/proto.h>
#include <asm/ia32.h>
#include <asm/debugreg.h>
#include <asm/switch_to.h>
#include <asm/resctrl.h>
#include <asm/unistd.h>
#include <asm/fsgsbase.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Albert Slepak");
MODULE_DESCRIPTION("Page-Walk Module");
MODULE_VERSION("1.0");

struct task_struct* get_task_struct_from_pid(__kernel_pid_t pid) {
    struct pid* pid_struct;
    struct task_struct* task;

    pid_struct = find_get_pid(pid);
    if (!pid_struct) {
        return NULL;
    }
    
    task = pid_task(pid_struct, PIDTYPE_PID);
    put_pid(pid_struct);

    return task;
}

struct vm_area_struct* get_task_base_vma(struct task_struct* task) {
    struct mm_struct* mm = task->mm;
    return mm->mmap;
}

struct vm_area_struct* get_next_vma(struct vm_area_struct* vma) {
    return vma->vm_next;
}

uint64_t get_task_vma_start(struct vm_area_struct* vma) {
    return vma->vm_start;
}

uint64_t get_task_vma_end(struct vm_area_struct* vma) {
    return vma->vm_end;
}

pte_t* get_pte_for_address(struct task_struct* task, uint64_t addr) {
    struct mm_struct* task_mm;
    pgd_t* pgd;
    p4d_t* p4d;
    pud_t* pud;
    pmd_t* pmd;
    pte_t* pte;

    task_mm = task->mm;

    pgd = pgd_offset(task_mm, addr);
    if (pgd_none(*pgd) || pgd_bad(*pgd))
        return NULL;

    p4d = p4d_offset(pgd, addr);
    if (p4d_none(*p4d) || p4d_bad(*p4d))
        return NULL;

    pud = pud_offset(p4d, addr);
    if (pud_none(*pud) || pud_bad(*pud))
        return NULL;

    pmd = pmd_offset(pud, addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd))
        return NULL;

    pte = pte_offset_kernel(pmd, addr);
    if (!pte)
        return NULL;

    return pte;
}

void fill_page_table_info_for_address(
    struct task_struct* task,
    uint64_t addr,
    uint64_t* out_pgd,
    uint64_t* out_p4d,
    uint64_t* out_pud,
    uint64_t* out_pmd,
    uint64_t* out_pte
) {
    struct mm_struct* task_mm;
    pgd_t* pgd;
    p4d_t* p4d;
    pud_t* pud;
    pmd_t* pmd;
    pte_t* pte;

    task_mm = task->mm;

    pgd = pgd_offset(task_mm, addr);
    if (pgd_none(*pgd) || pgd_bad(*pgd))
        return;

    p4d = p4d_offset(pgd, addr);
    if (p4d_none(*p4d) || p4d_bad(*p4d))
        return;

    pud = pud_offset(p4d, addr);
    if (pud_none(*pud) || pud_bad(*pud))
        return;

    pmd = pmd_offset(pud, addr);
    if (pmd_none(*pmd) || pmd_bad(*pmd))
        return;

    pte = pte_offset_kernel(pmd, addr);
    if (!pte)
        return;

    // Write the results into output variables
    *out_pgd = pgd_val(*pgd);
    *out_p4d = p4d_val(*p4d);
    *out_pud = pud_val(*pud);
    *out_pmd = pmd_val(*pmd);
    *out_pte = pte_val(*pte);
}

void make_pte_readonly(pte_t* pte) {
    pte_wrprotect(*pte);
}

void flush_tlb(void) {
    __flush_tlb_all();
}

extern __latent_entropy struct task_struct *copy_process(
    struct pid *pid,
    int trace,
    int node,
    struct kernel_clone_args *args
);

#define task_top_of_stack(task) ((unsigned long)(task_pt_regs(task) + 1))

#define task_pt_regs(task) \
({									\
    unsigned long __ptr = (unsigned long)task_stack_page(task);	\
    __ptr += THREAD_SIZE - TOP_OF_KERNEL_STACK_PADDING;		\
    ((struct pt_regs *)__ptr) - 1;					\
})

void impersonate_syscall(struct task_struct* target_task) {
    /*
        Outline:
            original_task = current
            current = target_task
            *syscall*
            current = original_task
    */
    struct task_struct*  original_task;
    struct task_struct*  forked_task;
    struct task_struct** this_core_current;
    unsigned long cpu_off;
    char* per_cpu_data;
    // uint64_t gsbase;
    int test_pid;
    // int test_pid2;
    // int test_pid3;
    // int ppid;
    // unsigned long gs_0;
    struct kernel_clone_args args = {
        .exit_signal = SIGCHLD,
    };

    // Disable preemption and local interrupts
    local_irq_disable();
    preempt_disable();

    cpu_off = __per_cpu_offset[0];
    per_cpu_data = (char*)cpu_off;
    this_core_current = (struct task_struct**)(per_cpu_data + 0x17bc0);
    original_task = *this_core_current;

    printk("current->pid    : %i\n", current->pid);

    this_cpu_write(current_task, target_task);
    if (!static_branch_likely(&switch_to_cond_stibp)) {
        asm volatile("nop");
    }

    test_pid = current->pid;

    forked_task = copy_process(NULL, 0, NUMA_NO_NODE, &args);

    this_cpu_write(current_task, original_task);
    if (!static_branch_likely(&switch_to_cond_stibp)) {
        asm volatile("nop");
    }

    // Enable preemption and local interrupts
    preempt_enable();
    local_irq_enable();

    printk("forked_task     : 0x%llx\n", (uint64_t)forked_task);
    printk("test_pid        : %i\n", test_pid);
    printk("current->pid    : %i\n", current->pid);
    printk("\n");
}

int init_module(void) {
    return 0;
}

void cleanup_module(void) {
}

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

struct task_struct** read_per_cpu_offset_current_struct_ptr(int cpu) {
    unsigned long cpu_offset;

    cpu_offset = (unsigned long)__per_cpu_offset[cpu];
    // return (struct task_struct**)(cpu_offset + 0x17bc0);
    return (struct task_struct**)(cpu_offset + 0x0);
}

int test_get_current(void) {
    return current->pid;
}

enum which_selector {
	FS,
	GS
};

static noinstr void __wrgsbase_inactive(unsigned long gsbase)
{
	lockdep_assert_irqs_disabled();

	if (!static_cpu_has(X86_FEATURE_XENPV)) {
		native_swapgs();
		wrgsbase(gsbase);
		native_swapgs();
	} else {
		instrumentation_begin();
		wrmsrl(MSR_KERNEL_GS_BASE, gsbase);
		instrumentation_end();
	}
}

static noinstr unsigned long __rdgsbase_inactive(void)
{
	unsigned long gsbase;

	lockdep_assert_irqs_disabled();

	if (!static_cpu_has(X86_FEATURE_XENPV)) {
		native_swapgs();
		gsbase = rdgsbase();
		native_swapgs();
	} else {
		instrumentation_begin();
		rdmsrl(MSR_KERNEL_GS_BASE, gsbase);
		instrumentation_end();
	}

	return gsbase;
}

static __always_inline void save_fsgs(struct task_struct *task)
{
	savesegment(fs, task->thread.fsindex);
	savesegment(gs, task->thread.gsindex);
	task->thread.fsbase = rdfsbase();
    task->thread.gsbase = __rdgsbase_inactive();
}

static __always_inline void x86_fsgsbase_load(struct thread_struct *prev,
					      struct thread_struct *next)
{
	/* Update the bases. */
    wrfsbase(next->fsbase);
    __wrgsbase_inactive(next->gsbase);
}

extern __latent_entropy struct task_struct *copy_process(
    struct pid *pid,
    int trace,
    int node,
    struct kernel_clone_args *args
);

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
    uint64_t gsbase;
    int test_pid;
    int test_pid2;
    int test_pid3;
    int ppid;
    unsigned long gs_0;
    struct kernel_clone_args args = {
		.exit_signal = SIGCHLD,
	};

    // Disable preemption and local interrupts
    /*local_irq_disable();
    preempt_disable();

    cpu_off = __per_cpu_offset[0];
    per_cpu_data = (char*)cpu_off;
    this_core_current = (struct task_struct**)(per_cpu_data + 0x17bc0);
    original_task = *this_core_current;

    *this_core_current = target_task;
    test_pid = (*this_core_current)->pid;
    test_pid2 = current->pid;
    *this_core_current = original_task;

    // Read gs
    asm("rdgsbase %0" : "=r" (gsbase));

    asm("mov $0xdeadbeef, %rax");
    this_cpu_write(current_task, target_task);

    test_pid3 = current->pid;
    //ppid = get_ppid();
    ppid = task_tgid_vnr(current->real_parent);
    this_cpu_write(current_task, original_task);

    // my_switch_to(original_task, target_task);
    // ppid = task_tgid_vnr(current->real_parent);
    // //test_pid4 = current->pid;
    // my_switch_to(original_task, original_task);

    // Enable preemption and local interrupts
    preempt_enable();
    local_irq_enable();

    printk("target_task->pid  : %i\n", target_task->pid);
    printk("cpu_off           : 0x%llx\n", cpu_off);
    printk("__per_cpu_offset  : 0x%llx\n", __per_cpu_offset);
    printk("*this_core_current: 0x%llx\n", *this_core_current);
    printk("current           : 0x%llx\n", current);
    printk("gsbase            : 0x%llx\n", gsbase);
    printk("current->pid      : %i\n", current->pid);
    printk("original_task->pid: %i\n", original_task->pid);
    printk("test_pid          : %i\n", test_pid);
    printk("test_pid2         : %i\n", test_pid2);
    printk("test_pid3         : %i\n", test_pid3);
    printk("ppid              : %i\n", ppid);

    printk("\n");
    */
    cpu_off = __per_cpu_offset[0];
    per_cpu_data = (char*)cpu_off;
    this_core_current = (struct task_struct**)(per_cpu_data + 0x17bc0);
    original_task = *this_core_current;

    // printk("current->pid    : %i\n", current->pid);

    this_cpu_write(current_task, target_task);
    // test_pid = current->pid;
    // ppid = task_tgid_vnr(current->real_parent);

    forked_task = copy_process(NULL, 0, NUMA_NO_NODE, &args);

    this_cpu_write(current_task, original_task);

    printk("forked_task        : 0x%llx\n", forked_task);
    // printk("test_pid        : %i\n", forked_task->pid);
}

int init_module(void) {
	return 0;
}

void cleanup_module(void) {
}

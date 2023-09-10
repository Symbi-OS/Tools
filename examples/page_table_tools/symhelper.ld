INCLUDE default_ld_script.ld

printk = 0xffffffff81ca81f7;
pgdir_shift = 0xffffffff826f1a6c;
boot_cpu_data = 0xffffffff82b63640;
physical_mask = 0xffffffff82708310;
sme_me_mask = 0xffffffff8284def8;
ptrs_per_p4d = 0xffffffff826f1a68;
pv_ops = 0xffffffff8283cc40;
page_offset_base = 0xffffffff826f1a60;
phys_base = 0xffffffff8281a010;
find_get_pid = 0xffffffff8110cd60;
put_pid = 0xffffffff8110cc10;
pid_task = 0xffffffff8110ca30;
__default_kernel_pte_mask = 0xffffffff82b63bd8;
__supported_pte_mask = 0xffffffff82b63be0;
__warn_printk = 0xffffffff81ca1f28;
__flush_tlb_all = 0xffffffff8107f1d0;
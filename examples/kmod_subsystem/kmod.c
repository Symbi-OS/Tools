#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/pid.h>
#include <linux/printk.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anonymous Template");
MODULE_DESCRIPTION("kmod");
MODULE_VERSION("1.0");

void __kmod_kprint(const char* msg) {
    printk(msg);
}

int init_module(void) {
    return 0;
}

void cleanup_module(void) {
}

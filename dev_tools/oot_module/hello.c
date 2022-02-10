#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tommy UNGER");
MODULE_DESCRIPTION("Print Stuff");
MODULE_VERSION("1.0");

extern void sym_lower(void);
extern void sym_elevate(void);

int init_module(void)
{

	printk(KERN_INFO "hello_mod: Module Init\n");

	/* printk(KERN_INFO "err_mod: Going to lower module\n"); */

	//sym_lower();


	/* printk(KERN_INFO "err_mod: Going to trigger kernel panic\n"); */
	/* panic("err_mod: force-panic"); */

	/*
	printk(KERN_INFO "err_mod: In Kernel Mode\n");
	sym_lower();
	printk(KERN_INFO "err_mod: In User Mode\n");
	sym_elevate();
	printk(KERN_INFO "err_mod: In Kernel Mode\n");
	*/

	/*
	err = 10/0;
	printk(KERN_INFO "err_mod: Err = %d\n", err);
	*/

	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "hello_mod: Cleanup!\n");
	printk(KERN_INFO "hello_mod: Cleanup!\n");
	printk(KERN_INFO "hello_mod: Cleanup!\n");
}


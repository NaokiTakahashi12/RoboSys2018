
#include <linux/module.h>

MODULE_AUTHOR("Naoki Takahashi");
MODULE_DESCRIPTION("Driver for LED control");

static int __init init_mod(void) {
	printk(KERN_INFO "%s is loaded.\n", __FILE__);
	return 0;
}

static void  __exit cleanup_mod(void) {
	printk(KERN_INFO "%s is unloaded.\n", __FILE__);
}

module_init(init_mod);
module_exit(cleanup_mod);


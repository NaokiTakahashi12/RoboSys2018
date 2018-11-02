#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_AUTHOR("Naoki Takahashi");
MODULE_DESCRIPTION("Driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static struct class *class_ptr = NULL;
static dev_t device;
static struct cdev character_device;

static ssize_t led_write(struct file *file_ptr, const char *buffer, size_t count, loff_t *pos) {
	printk(KERN_INFO "Led write is called.\n");
	return 1;
}

static struct file_operations led_file_operations = {
	.owner = THIS_MODULE,
	.write = led_write
};

static int __init init_mod(void) {
	int return_region_value;
	int return_character_device_add;

	return_region_value = alloc_chrdev_region(&device, 0, 1, "led");

	cdev_init(&character_device, &led_file_operations);
	return_character_device_add = cdev_add(&character_device, device, 1);

	class_ptr = class_create(THIS_MODULE, "led");

	if(return_region_value < 0) {
		printk(KERN_ERR "alloc_chrdev_region failed. major number:%d minor number:%d\n", MAJOR(device), MINOR(device));
		return return_region_value;
	}
	else if(return_character_device_add < 0) {
		printk(KERN_ERR "cdev_add failed. major numebr:%d minor number:%d\n", MAJOR(device), MINOR(device));
		return return_character_device_add;
	}
	else if(IS_ERR(class_ptr)) {
		printk(KERN_ERR "class_create failed.\n");
		return PTR_ERR(class_ptr);
	}
	else {
		device_create(class_ptr, NULL, device, NULL, "led%d", MINOR(device));
		printk(KERN_INFO "%s is loaded. major number %d.\n", __FILE__, MAJOR(device));
	}

	return 0;
}

static void  __exit cleanup_mod(void) {
	device_destroy(class_ptr, device);
	class_destroy(class_ptr);
	cdev_del(&character_device);
	unregister_chrdev_region(device, 1);

	printk(KERN_INFO "%s is unloaded. major number is %d\n", __FILE__, MAJOR(device));
}

module_init(init_mod);
module_exit(cleanup_mod);


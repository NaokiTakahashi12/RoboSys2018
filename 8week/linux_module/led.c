#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/io.h>

MODULE_AUTHOR("Naoki Takahashi");
MODULE_DESCRIPTION("Driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static dev_t device;
static struct class *class_ptr = NULL;
static struct cdev character_device;
static volatile u32 *gpio_base = NULL;

static ssize_t character_module_write(struct file *file_ptr, const char *buffer, size_t count, loff_t *pos) {
	char c;

	printk(KERN_INFO "Led write is called. ");

	if(copy_from_user(&c, buffer, sizeof(char))) {
		return -EFAULT;
	}

	if(c == '0') {
		gpio_base[10] = 1 << 25;
		printk(KERN_INFO "Led is down. ");
	}
	else if(c == '1') {
		gpio_base[7] = 1 << 25;
		printk(KERN_INFO "Led is up. ");
	}

	return 1;
}

static ssize_t character_module_read(struct file *file_ptr, char *buffer, size_t count, loff_t *pos) {
	int size = 0;
	char sushi[] = {0xF0, 0x9F, 0x8D, 0xA3, 0x0A};

	if(copy_to_user(buffer + size, (const char *)sushi, sizeof(sushi))) {
		printk(KERN_INFO "copy_to_user() failed.\n");
		return -EFAULT;
	}

	size += sizeof(sushi);

	return size;
}

static struct file_operations led_file_operations = {
	.owner = THIS_MODULE,
	.write = character_module_write,
	.read = character_module_read
};

static int __init init_mod(void) {
	int return_region_value;
	int return_character_device_add;
	int led_number;

	u32 led, index, shift, mask;

	led_number = 25;

	gpio_base = ioremap_nocache(0x3F200000, 0xA0);
	led = (u32)led_number;
	index = led / 10;
	shift = (led % 10) * 3;
	mask = ~(0x7 << shift);
	gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift);

	return_region_value = alloc_chrdev_region(&device, 0, 1, "RoboSysLED");
	if(return_region_value < 0) {
		printk(KERN_ERR "alloc_chrdev_region failed. major number:%d minor number:%d\n", MAJOR(device), MINOR(device));
		return return_region_value;
	}

	cdev_init(&character_device, &led_file_operations);
	return_character_device_add = cdev_add(&character_device, device, 1);
	if(return_character_device_add < 0) {
		printk(KERN_ERR "cdev_add failed. major numebr:%d minor number:%d\n", MAJOR(device), MINOR(device));
		return return_character_device_add;
	}

	class_ptr = class_create(THIS_MODULE, "RoboSysLED");
	if(IS_ERR(class_ptr)) {
		printk(KERN_ERR "class_create failed.\n");
		return PTR_ERR(class_ptr);
	}

	device_create(class_ptr, NULL, device, NULL, "RoboSysLED%d", MINOR(device));
	printk(KERN_INFO "%s is loaded. major number %d.\n", __FILE__, MAJOR(device));

	return 0;
}

static void  __exit cleanup_mod(void) {
	device_destroy(class_ptr, device);
	class_destroy(class_ptr);
	cdev_del(&character_device);
	unregister_chrdev_region(device, 1);

	printk(KERN_INFO "%s is purge. major number is %d\n", __FILE__, MAJOR(device));
}

module_init(init_mod);
module_exit(cleanup_mod);


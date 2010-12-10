/*
 * minimal_character_dd.c
 *
 *  Created on: Dec 10, 2010
 *      Author: bogdan
 */

#include <linux/module.h>
#include <linux/fs.h>

#define HELLO_MAJOR 233

static int debug_enable = 0;
module_param(debug_enable, int, 0);
MODULE_PARM_DESC(debug_enable, "Enable module debugging mode");

struct file_operations hello_fops;

static int hello_open(struct inode *inode, struct file *file)
{
	printk("%s: successful\n", __func__);
	return 0;
}

static int hello_release(struct inode *inode, struct file *file)
{
	printk("%s: successful\n", __func__);
	return 0;
}

static ssize_t hello_read(struct file *file, char *buf, size_t count, loff_t *ptr)
{
	printk("%s: returning zero bytes\n", __func__);
	return 0;
}

static ssize_t hello_write(struct file *file, const char *buf, size_t count, loff_t *ppos)
{
	printk("%s: accepting zero bytes\n", __func__);
	return 0;
}

static long hello_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("%s: cmd = %d, arg = %ld\n", __func__, cmd, arg);
	return 0;
}

static int __init hello_init(void)
{
	int ret;

	printk("Hello Example Init - debug mode is %s\n", debug_enable == 1 ? "enabled":"disabled");

	ret = register_chrdev(HELLO_MAJOR, "hello1", &hello_fops);
	if (0 > ret)
	{
		printk("Error registering hello device");
		goto hello_fail;
	}
	printk("Register module successfully\n");

	return 0;

hello_fail:
	return ret;
}

static void __exit hello_exit(void)
{
	printk("Hello Example Exit\n");
}

struct file_operations hello_fops =
{
		owner: THIS_MODULE,
		read: hello_read,
		write: hello_write,
		compat_ioctl: hello_ioctl,
		open: hello_open,
		release: hello_release,
};

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("BC");
MODULE_DESCRIPTION("Hello World Example");
MODULE_LICENSE("GPL");

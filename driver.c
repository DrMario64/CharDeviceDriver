#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/jiffies.h>

#define WR_VALUE _IOW('a', 'a', struct runner *)
#define RD_VALUE _IOR('a', 'b', struct runner *)

int32_t value = 0;

struct runner {
	int number;
	int position;
	int lane;
	int lap;
	char name[20];
	char school[10];
	struct runner *next;

	unsigned long start_time;
	unsigned long cur_lap_time;
	unsigned long total_time;
};

struct runner *krunner;	//kernelspace struct of runners

unsigned long j;	//jiffies variable
int s;
dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t * off);
static ssize_t etx_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/* All file operation implementation from previous hw assignment. */
static struct file_operations fops =
{
	.owner		= THIS_MODULE,
	.read		= etx_read,
	.write		= etx_write,
	.open		= etx_open,
	.unlocked_ioctl	= etx_ioctl,
	.release	= etx_release,
};

static int etx_open(struct inode *inode, struct file *file)
{
	pr_info("Device File Opened!\n");
	return 0;
}

static int etx_release(struct inode *inode, struct file *file)
{
	pr_info("Device File Closed!\n");
	return 0;
}

static ssize_t etx_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	pr_info("Read Function\n");
	return 0;
}

static ssize_t etx_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	pr_info("Write function\n");
	return len;
}

/* ioctl function called from userspace.
 *
 * A dereferenced pointer is passed from userspace (pointer to a pointer to a runner struct)
 * which allows ioctl to share data between userspace and the kernel.
 * Lap time is determined by jiffies where no additional calculations are done to alter them.
 * Speed for each runner is constant.
 * When a runner runs for 9999 jiffies, they complete a lap and the time variables are all updated.
 */
static long etx_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct runner *tmp = kmalloc(sizeof(struct runner), GFP_KERNEL);
	switch(cmd) {
		case WR_VALUE:
			if (copy_from_user(&krunner, (struct runner **) arg, sizeof(krunner))) {
				pr_err("Data Write: Error :(\n");
				break;	
			}
			tmp = krunner;
			while (tmp != NULL) {
				if (tmp->start_time == 0) {
					tmp->start_time = jiffies;
				}
				else {
					tmp->cur_lap_time = jiffies - tmp->start_time;
					while (tmp->cur_lap_time >= 9999) {
						tmp->lap++;
						if (tmp->lap > 3) {
							tmp->lap--;
							tmp->cur_lap_time = 0;
							tmp->total_time = 9999 * 3;
							break;
						}
						tmp->total_time += tmp->cur_lap_time;
						tmp->start_time = jiffies;
						tmp->cur_lap_time -= 9999;
					}
				}
				tmp = tmp->next;
			}
			//pr_info("Value (driver) = %d %d %s %s\n", krunner->number, krunner->position, krunner->name, krunner->school);
			break;
		case RD_VALUE:
			if (copy_to_user((struct runner  **) arg, &krunner, sizeof(krunner))) {
				pr_err("Data Read: Error :(\n");
			}
			//pr_info("krunners value = %d %d %s %s\n", krunner->number, krunner->position, krunner->name, krunner->school);
			//pr_info("Value (driver to user) = %d %d %s %s\n",
					//((struct runners *)arg)->number, ((struct runners *)arg)->position, ((struct runners *)arg)->name, ((struct runners *)arg)->school);
			break;
		default:
			pr_info("Default\n");
			break;
	}
	return 0;
}

/* Initializes the driver. */
static int __init etx_driver_init(void)
{
	krunner = kmalloc(sizeof(struct runner), GFP_KERNEL);
	if ((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) < 0) {
		pr_err("Cannot allocate major number\n");
		return -1;
	}
	pr_info("Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));

	cdev_init(&etx_cdev, &fops);

	if ((cdev_add(&etx_cdev, dev, 1)) < 0) {
		pr_err("Cannot add the device to the system\n");
		goto r_class;
	}

	if ((dev_class = class_create(THIS_MODULE, "etx_class")) == NULL) {
		pr_err("Cannot create the struct class\n");
		goto r_class;
	}

	if ((device_create(dev_class, NULL, dev, NULL, "etx_device")) == NULL) {
		pr_err("Cannot create the Device\n");
		goto r_device;
	}

	pr_info("Device Driver Insert Done!!\n");
	return 0;

r_device:
	class_destroy(dev_class);

r_class:
	unregister_chrdev_region(dev, 1);
	return -1;
}

/* exits the driver. */
static void __exit etx_driver_exit(void)
{
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&etx_cdev);
	unregister_chrdev_region(dev, 1);
	pr_info("Device Driver Remove Done!!\n");
	//kfree(krunner);
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");

/* ----------------------------------------------- DRIVER gmem --------------------------------------------------
 
 Basic driver example to show skelton methods for several file operations.
 
 ----------------------------------------------------------------------------------------------------------------*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/jiffies.h>

#include<linux/init.h>
#include<linux/moduleparam.h>

#define DEVICE_NAME                 "gmem"  // device name to be created and registered

/* per device structure */
struct gmem_dev {
	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/
	char in_string[256];			/* buffer for the input string */
	int current_write_pointer;
} *gmem_devp;

static dev_t gmem_dev_number;      /* Allotted device number */
struct class *gmem_dev_class;          /* Tie with the device model */
static struct device *gmem_dev_device;

static char *user_name = "Dear John";

module_param(user_name,charp,0000);	//to get parameter from load.sh script to greet the user

/*
* Open gmem driver
*/
int gmem_driver_open(struct inode *inode, struct file *file)
{
	struct gmem_dev *gmem_devp;
//	printk("\nopening\n");

	/* Get the per-device structure that contains this cdev */
	gmem_devp = container_of(inode->i_cdev, struct gmem_dev, cdev);


	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = gmem_devp;
	printk("\n%s is openning \n", gmem_devp->name);
	return 0;
}

/*
 * Release gmem driver
 */
int gmem_driver_release(struct inode *inode, struct file *file)
{
	struct gmem_dev *gmem_devp = file->private_data;
	
	printk("\n%s is closing\n", gmem_devp->name);
	
	return 0;
}

/*
 * Write to gmem driver
 */
ssize_t gmem_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{
	struct gmem_dev *gmem_devp = file->private_data;
	
	while (count) {	
		get_user(gmem_devp->in_string[gmem_devp->current_write_pointer], buf++);
		count--;
		if(count){
			gmem_devp->current_write_pointer++;
			if( gmem_devp->current_write_pointer == 256)
				gmem_devp->current_write_pointer = 0;
		}
	}
	printk("Writing -- %d %s \n", gmem_devp->current_write_pointer, gmem_devp->in_string);
	return 0;
}
/*
 * Read to gmem driver
 */
ssize_t gmem_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{
	int bytes_read = 0;
	struct gmem_dev *gmem_devp = file->private_data;
	/*
	 * If we're at the end of the message, 
	 * return 0 signifying end of file 
	 */
	if (gmem_devp->in_string[0] == 0)
		return 0;

	/* 
	 * Actually put the data into the buffer 
	 */
	while (count && gmem_devp->in_string[bytes_read]) {

		put_user(gmem_devp->in_string[bytes_read], buf++);
		count--;
		bytes_read++;
	}
	printk("Reading -- '%s'\n",gmem_devp->in_string);
	/* 
	 * Most read functions return the number of bytes put into the buffer
	 */
	return bytes_read;

}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations gmem_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= gmem_driver_open,        /* Open method */
    .release	= gmem_driver_release,     /* Release method */
    .write		= gmem_driver_write,       /* Write method */
    .read		= gmem_driver_read,        /* Read method */
};

/*
 * Driver Initialization
 */
int __init gmem_driver_init(void)
{
	int ret;
	int time_since_boot;

	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&gmem_dev_number, 0, 1, DEVICE_NAME) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); return -1;
	}

	/* Populate sysfs entries */
	gmem_dev_class = class_create(THIS_MODULE, DEVICE_NAME);

	/* Allocate memory for the per-device structure */
	gmem_devp = kmalloc(sizeof(struct gmem_dev), GFP_KERNEL);
		
	if (!gmem_devp) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}

	/* Request I/O region */
	sprintf(gmem_devp->name, DEVICE_NAME);

	/* Connect the file operations with the cdev */
	cdev_init(&gmem_devp->cdev, &gmem_fops);
	gmem_devp->cdev.owner = THIS_MODULE;

	/* Connect the major/minor number to the cdev */
	ret = cdev_add(&gmem_devp->cdev, (gmem_dev_number), 1);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	/* Send uevents to udev, so it'll create /dev nodes */
	gmem_dev_device = device_create(gmem_dev_class, NULL, MKDEV(MAJOR(gmem_dev_number), 0), NULL, DEVICE_NAME);		
	// device_create_file(gmem_dev_device, &dev_attr_xxx);

	memset(gmem_devp->in_string, 0, 256);
	
	time_since_boot=(jiffies-INITIAL_JIFFIES)/HZ;//since on some systems jiffies is a very huge uninitialized value at boot and saved.
	sprintf(gmem_devp->in_string, "Hi %s, this machine has been on for %d seconds", user_name, time_since_boot);
	
	gmem_devp->current_write_pointer = 0;

	printk("gmem driver initialized.\n");// '%s'\n",gmem_devp->in_string);
	return 0;
}
/* Driver Exit */
void __exit gmem_driver_exit(void)
{
	// device_remove_file(gmem_dev_device, &dev_attr_xxx);
	/* Release the major number */
	unregister_chrdev_region((gmem_dev_number), 1);

	/* Destroy device */
	device_destroy (gmem_dev_class, MKDEV(MAJOR(gmem_dev_number), 0));
	cdev_del(&gmem_devp->cdev);
	kfree(gmem_devp);
	
	/* Destroy driver_class */
	class_destroy(gmem_dev_class);

	printk("gmem driver removed.\n");
}

module_init(gmem_driver_init);
module_exit(gmem_driver_exit);
MODULE_LICENSE("GPL v2");

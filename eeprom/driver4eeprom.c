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

#include <linux/init.h>
#include <linux/moduleparam.h>


#define DEVICE_NAME "i2c_flash"  // device name to be created and registered


struct i2c_dev {
	struct i2c_adapter *adapter;
//	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/
} *i2c_devp;

static dev_t i2c_dev_number;      /* Allotted device number */
static struct class *i2c_dev_class;          /* Tie with the device model */
static struct device *i2c_dev_device;



/*
* Open driver
*/
int i2c_driver_open(struct inode *inode, struct file *file)
{
	struct i2c_dev *i2c_devp;

	/* Get the per-device structure that contains this cdev */
	i2c_devp = container_of(inode->i_cdev, struct i2c_dev, cdev);


	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = i2c_devp;
	printk("\n%s is openning \n", i2c_devp->name);
	return 0;
}

/*
 * Close driver
 */
int i2c_driver_close(struct inode *inode, struct file *file)
{
	struct i2c_dev *i2c_devp = file->private_data;
	
	printk("\n%s is closing\n", i2c_devp->name);
	
	return 0;
}

/*
 * Write to driver
 */
ssize_t i2c_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{
	struct i2c_dev *i2c_devp = file->private_data;
	size_t size=0;
	struct i2c_adapter* adapter;
	struct i2c_msg *msg;
	char buf_[65];
	int ret;

	adapter=i2c_devp->client->adapter;
	memcpy(buf_+1,buf,64);
	buf[0]=0;

	msg[0].addr=i2c_devp->client->addr;
	msg[0].flags=0;
	msg[0].len=64;
	msg[0].buf=buf_;

	ret = i2c_transfer(adapter, msg, 1);

printk(KERN_INFO "write return %d\n", ret); 

	//copy_to_user(buf,i2c_devp->queue[i2c_devp->end].string,size+1);

	printk(KERN_INFO "Writing: %s\n", buf_); 
	return size;






	struct i2c_dev *i2c_devp = file->private_data;


	if(count!=64) return -1;

	
//	copy_from_user(i2c_devp->queue[i2c_devp->front].string,buf,count);

	printk(KERN_INFO "Writing: %s\n", buf);
	return count;
}
/*
 * Read to driver
 */
ssize_t i2c_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{
	struct i2c_dev *i2c_devp = file->private_data;
	size_t size=0;
	struct i2c_adapter* adapter;
	struct i2c_msg *msg;
	char buf_[65];
	int ret;

	adapter=i2c_devp->client->adapter;
	memcpy(buf_+1,buf,64);
	buf[0]=0;

	msg[0].addr=i2c_devp->client->addr;
	msg[0].flags=0;
	msg[0].len=64;
	msg[0].buf=buf_;

	ret = i2c_transfer(adapter, msg, 1);


	//copy_to_user(buf,i2c_devp->queue[i2c_devp->end].string,size+1);

	printk(KERN_INFO "Reading: %s\n", buf); 
	return size;
}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations i2c_fops = {
    .owner		= THIS_MODULE,           /* Owner */
    .open		= i2c_driver_open,        /* Open method */
    .close		= i2c_driver_close,     /* Release method */
    .write		= i2c_driver_write,       /* Write method */
    .read		= i2c_driver_read,        /* Read method */
    .lseek		= i2c_driver_lseek,        /* Lseek method */
};
static struct i2c_driver eeprom_driver={
	.driver={
	.name="i2c_flash",
	.owner=THIS_MODULE,
	},
	.class=1;
	.probe=i2c_driver_probe,{}

};


/*
 * Driver Initialization
 */
int __init i2c_driver_init(void)
{
	int ret;
	int time_since_boot;
	int i=0;


	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&i2c_dev_number, 0, 1, DEVICE_NAME) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); return -1;
	}

	/* Populate sysfs entries */
	i2c_dev_class = class_create(THIS_MODULE, DEVICE_NAME);

	/* Allocate memory for the per-device structure */
	i2c_devp = kmalloc(sizeof(struct i2c_dev), GFP_KERNEL);
		
	if (!i2c_devp) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}

	/* Request I/O region */
	sprintf(i2c_devp->name, DEVICE_NAME);

	/* Connect the file operations with the cdev */
	cdev_init(&i2c_devp->cdev, &i2c_fops);
	i2c_devp->cdev.owner = THIS_MODULE;
	

	/* Connect the major/minor number to the cdev */
	ret = cdev_add(&i2c_devp->cdev, (i2c_dev_number), 1);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	/* Send uevents to udev, so it'll create /dev nodes */
	i2c_dev_device= device_create(i2c_dev_class, NULL, \
	MKDEV(MAJOR(i2c_dev_number), 0),NULL, \
	DEVICE_NAME);

	// device_create_file(i2c_dev_device, &dev_attr_xxx);
//	register_chrdev(MAJOR(i2c_dev_number),"DEVICE_NAME",&gmem_fops);

	
	

	printk("i2c_flash driver initialized.\n");// '%s'\n",i2c_devp->in_string);
	return 0;
}
/* Driver Exit */
void __exit i2c_driver_exit(void)
{
	int i=0;

	/* Release the major number */
	unregister_chrdev_region((i2c_dev_number), 1);

	/* Destroy device */
	device_destroy (i2c_dev_class, MKDEV(MAJOR(i2c_dev_number), 0));
	cdev_del(&i2c_devp->cdev);
	kfree(i2c_devp);
	
	/* Destroy driver_class */
	class_destroy(i2c_dev_class);

	printk("i2c driver removed.\n");
}

module_init(i2c_driver_init);
module_exit(i2c_driver_exit);
MODULE_LICENSE("GPL v2");

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

#include <linux/init.h>
#include <linux/moduleparam.h>

//#include <stdlib.h>

#define DEVICE_NAME                 "gmem"  // device name to be created and registered
#define MINOR_NUMBER 2
#define TOKEN_NUMBER 10 

/* per device structure */
struct BufferToken {
	int id;
	int enqueue;
	int dequeue;
	//uint64_t enqueue;
	//uint64_t dequeue;
	char string[80];
};

struct gmem_dev {
	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/
	struct BufferToken queue[TOKEN_NUMBER];
	int queueSize;
	int front, end;
	//char in_string[256];			/* buffer for the input string */
	//int current_write_pointer;
} *gmem_devp;

static dev_t gmem_dev_number;      /* Allotted device number */
struct class *gmem_dev_class;          /* Tie with the device model */
static struct device *gmem_dev_device[MINOR_NUMBER];
static long int sequenceNum;

static char *user_name = "Dear John";

module_param(user_name,charp,0000);	//to get parameter from load.sh script to greet the user

/*
* Open gmem driver
*/
int gmem_driver_open(struct inode *inode, struct file *file)
{
	struct gmem_dev *gmem_devp;

	/* Get the per-device structure that contains this cdev */
	gmem_devp = container_of(inode->i_cdev, struct gmem_dev, cdev);


	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = gmem_devp;
	printk(KERN_INFO "driver4sq is openning \n");
	return 0;
}

/*
 * Release gmem driver
 */
int gmem_driver_release(struct inode *inode, struct file *file)
{
	struct gmem_dev *gmem_devp = file->private_data;
	
	printk(KERN_INFO "driver4sq is closing\n");
	
	return 0;
}

/*
 * Write to gmem driver
 */
ssize_t gmem_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{
	struct gmem_dev *gmem_devp = file->private_data;
	size_t size=0;


	if(count==0 || count>=80) return -1;

	if(gmem_devp->queueSize>=TOKEN_NUMBER)  return -1;
	
	gmem_devp->front++;
	if(gmem_devp->front>=TOKEN_NUMBER) gmem_devp->front=0;
	gmem_devp->queue[gmem_devp->front].id=sequenceNum++;
	gmem_devp->queue[gmem_devp->front].enqueue=(jiffies-INITIAL_JIFFIES)/HZ;

	/*while(1){
	if(buf[size]=='\0') break;
	size++;
	}*/
	//size=strlen(buf);
	copy_from_user(gmem_devp->queue[gmem_devp->front].string,buf,count);
	gmem_devp->queueSize++;

	printk(KERN_INFO "Writing -- front:%d, end:%d, frontString:%s, stringSize:%d \n", \
	gmem_devp->front, gmem_devp->end, gmem_devp->queue[gmem_devp->front].string, count);
	return size;
}
/*
 * Read to gmem driver
 */
ssize_t gmem_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{
	struct gmem_dev *gmem_devp = file->private_data;
	size_t size=0;

	if(gmem_devp->queueSize<=0)  return -1;
	
	gmem_devp->queue[gmem_devp->end].dequeue=(jiffies-INITIAL_JIFFIES)/HZ;

	while(1){
	if(gmem_devp->queue[gmem_devp->end].string[size]=='\0') break;
	size++;
	}
	//size=strlen(buf);
	copy_to_user(buf,gmem_devp->queue[gmem_devp->end].string,size+1);

	gmem_devp->end++;
	if(gmem_devp->end>=TOKEN_NUMBER) gmem_devp->end=0;
	gmem_devp->queueSize--;

	printk(KERN_INFO "Reading -- front:%d, end:%d, endString:%s, stringSize:%d \n", \
	gmem_devp->front, gmem_devp->end, gmem_devp->queue[gmem_devp->end].string, count);
	return size;
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
	int i=0;

	sequenceNum=0;

	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&gmem_dev_number, 0, 2, DEVICE_NAME) < 0) {
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
	
	gmem_devp->front=TOKEN_NUMBER-1;
	gmem_devp->end=0;
	gmem_devp->queueSize=0;

	/* Connect the major/minor number to the cdev */
	ret = cdev_add(&gmem_devp->cdev, (gmem_dev_number), 2);

	if (ret) {
		printk("Bad cdev\n");
		return ret;
	}

	/* Send uevents to udev, so it'll create /dev nodes */
	for(i=0;i<MINOR_NUMBER;i++){
	gmem_dev_device[i]= device_create(gmem_dev_class, NULL, \
	MKDEV(MAJOR(gmem_dev_number), i),NULL, \
	DEVICE_NAME"%d", i);
	}
	// device_create_file(gmem_dev_device, &dev_attr_xxx);
//	register_chrdev(MAJOR(gmem_dev_number),"DEVICE_NAME",&gmem_fops);

//	memset(gmem_devp->in_string, 0, 256);
	
	time_since_boot=(jiffies-INITIAL_JIFFIES)/HZ;//since on some systems jiffies is a very huge uninitialized value at boot and saved.
	//sprintf(gmem_devp->in_string, "Hi %s, this machine has been on for %d seconds", user_name, time_since_boot);
	
	//gmem_devp->current_write_pointer = 0;

	printk("gmem driver initialized.\n");// '%s'\n",gmem_devp->in_string);
	return 0;
}
/* Driver Exit */
void __exit gmem_driver_exit(void)
{
	int i=0;

	// device_remove_file(gmem_dev_device, &dev_attr_xxx);
	/* Release the major number */
	unregister_chrdev_region((gmem_dev_number), 2);

	/* Destroy device */
	for(i=0;i<MINOR_NUMBER;i++){
	device_destroy (gmem_dev_class, MKDEV(MAJOR(gmem_dev_number), i));
	}
	cdev_del(&gmem_devp->cdev);
	kfree(gmem_devp);
	
	/* Destroy driver_class */
	class_destroy(gmem_dev_class);

	printk("gmem driver removed.\n");
}

module_init(gmem_driver_init);
module_exit(gmem_driver_exit);
MODULE_LICENSE("GPL v2");

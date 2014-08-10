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
                                                                                
#include <linux/i2c.h>                                                          
#include <linux/i2c-dev.h>                                                      
                                                                                
#define DEVICE_NAME "i2c_flash"  // device name to be created and registered    
                                                                                
                                                                                
struct i2c_dev {                                                                
        struct i2c_client *client;                                              
        struct i2c_adapter *adap;                                               
        struct cdev cdev;               /* The cdev structure */                
        char name[20];                  /* Name of device*/                     
} *i2c_devp;                                                                    
                                                                                
static dev_t i2c_dev_number;      /* Allotted device number */                  
struct class *i2c_dev_class;          /* Tie with the device model */           
static struct device *i2c_dev_device;                                           
                                                                                
                                                                                
                                                                                
/*                                                                              
* Open driver                                                                   
*/                                                                              
int i2c_driver_open(struct inode *inode, struct file *file)                     
{                                                                               
        struct i2c_dev *i2c_devp;                                               
        //struct i2c_adapter* adap;                                             
        struct i2c_client *client;                                              
                                                                                
        /* Get the per-device structure that contains this cdev */              
        i2c_devp = container_of(inode->i_cdev, struct i2c_dev, cdev);           
                                                                                
        i2c_devp->adap = kzalloc(sizeof(*(i2c_devp->adap)), GFP_KERNEL);        
        i2c_devp->adap = i2c_get_adapter(i2c_devp->adap->nr);                   
        if (!i2c_devp->adap)                                                    
                return -ENODEV;                                                 
                                                                                
        i2c_devp->client = kzalloc(sizeof(*(i2c_devp->client)), GFP_KERNEL);    
        if (!i2c_devp->client) {                                                
                i2c_put_adapter(i2c_devp->adap);                                
                return -ENOMEM;                                                 
        }                                                                       
        snprintf(i2c_devp->client->name, I2C_NAME_SIZE,\
	"myi2c-dev%d", i2c_devp->adap->nr);                                                                     

                                                                                
        i2c_devp->client->adapter = i2c_devp->adap;                             
        file->private_data = i2c_devp->client;                                  
                                                                                
//        i2c_devp->client->addr = 0x54;                                          
                                                                                
        /* Easy access to cmos_devp from rest of the entry points */            
        //file->private_data = i2c_devp;                                        
                                                                                
        printk(KERN_INFO "\n%s is openning \n", i2c_devp->name);                
                                                                                
        return 0;                                                               
}                                                                               
                                                                                
/*                                                                              
 * Close driver                                                                 
 */                                                                             
int i2c_driver_close(struct inode *inode, struct file *file)                    
{                                                                               
        //struct i2c_dev *i2c_devp = file->private_data;
                                                      
        //file->private_data=NULL;   
                                                                    
        printk("\n%s is closing\n", i2c_devp->name);                            
                                                                                
        return 0;                                                               
}                                                                               
                                                                                
/*                                                                              
 * Write to driver                                                              
 */                                                                             
ssize_t i2c_driver_write(struct file *file, const char *buf,                    
           size_t count, loff_t *ppos)                                          
{                                                                               
        int ret;                                                        
        char *tmp;                                                              
        struct i2c_client *client = file->private_data;                         
                                                                                
        if (count > 64)                                                         
                count = 64;                                                     
                                                                                
        tmp = memdup_user(buf, count);                                          
        if (IS_ERR(tmp))                                                        
                return PTR_ERR(tmp);                                            
                                                                                
        //pr_debug("i2c-dev: i2c-%d writing %zu bytes.\n",                      
          //      iminor(file_inode(file)), count);                             
                                                                                
        ret = i2c_master_send(client, tmp, count);                              
        kfree(tmp);                                                             
        return ret;                                                             
                                                                                
                                                                                
/*                                                                              
                                                                                
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
                                                                                
        */                                                                      
                                                                                
                                                                                
}                                                                               
/*                                                                              
 * Read to driver                                                               
 */                                                                             
ssize_t i2c_driver_read(struct file *file, char *buf,                           
           size_t count, loff_t *ppos)                                          
{
        int ret;    
        char *tmp;                                                              
        struct i2c_client *client = file->private_data;                         
                                                                                
        if (count > 64)                                                         
                count = 64;                                                     
          
        tmp = kmalloc(count, GFP_KERNEL);
        if (tmp == NULL)
                return -ENOMEM;

        ret = i2c_master_recv(client, tmp, count);
        if (ret >= 0)
                ret = copy_to_user(buf, tmp, count) ? -EFAULT : ret;
        kfree(tmp);
        return ret;
                        
/*
                                                                                 
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
*/

                                                      
}                                                                               
                                                                                

static noinline int i2cdev_ioctl_rdrw(struct i2c_client *client,
                unsigned long arg)
{
        struct i2c_rdwr_ioctl_data rdwr_arg;
        struct i2c_msg *rdwr_pa;
        u8 __user **data_ptrs;
        int i, res;

        if (copy_from_user(&rdwr_arg,
                           (struct i2c_rdwr_ioctl_data __user *)arg,
                           sizeof(rdwr_arg)))
                return -EFAULT;

        /* Put an arbitrary limit on the number of messages that can
         * be sent at once */
        if (rdwr_arg.nmsgs > I2C_RDRW_IOCTL_MAX_MSGS)
                return -EINVAL;

        rdwr_pa = memdup_user(rdwr_arg.msgs,
                              rdwr_arg.nmsgs * sizeof(struct i2c_msg));
        if (IS_ERR(rdwr_pa))
                return PTR_ERR(rdwr_pa);

        data_ptrs = kmalloc(rdwr_arg.nmsgs * sizeof(u8 __user *), GFP_KERNEL);
        if (data_ptrs == NULL) {
                kfree(rdwr_pa);
                return -ENOMEM;
        }

        res = 0;
        for (i = 0; i < rdwr_arg.nmsgs; i++) {
                /* Limit the size of the message to a sane amount */
                if (rdwr_pa[i].len > 8192) {
                        res = -EINVAL;
                        break;
                }

                data_ptrs[i] = (u8 __user *)rdwr_pa[i].buf;
                rdwr_pa[i].buf = memdup_user(data_ptrs[i], rdwr_pa[i].len);
                if (IS_ERR(rdwr_pa[i].buf)) {
                        res = PTR_ERR(rdwr_pa[i].buf);
                        break;
                }

                if (rdwr_pa[i].flags & I2C_M_RECV_LEN) {
                        if (!(rdwr_pa[i].flags & I2C_M_RD) ||
                            rdwr_pa[i].buf[0] < 1 ||
                            rdwr_pa[i].len < rdwr_pa[i].buf[0] +
                                             I2C_SMBUS_BLOCK_MAX) {
                                res = -EINVAL;
                                break;
                        }

                        rdwr_pa[i].len = rdwr_pa[i].buf[0];
                }
        }
        if (res < 0) {
                int j;
                for (j = 0; j < i; ++j)
                        kfree(rdwr_pa[j].buf);
                kfree(data_ptrs);
                kfree(rdwr_pa);
                return res;
        }

        res = i2c_transfer(client->adapter, rdwr_pa, rdwr_arg.nmsgs);
        while (i-- > 0) {
                if (res >= 0 && (rdwr_pa[i].flags & I2C_M_RD)) {
                        if (copy_to_user(data_ptrs[i], rdwr_pa[i].buf,
                                         rdwr_pa[i].len))
                                res = -EFAULT;
                }
                kfree(rdwr_pa[i].buf);
        }
        kfree(data_ptrs);
        kfree(rdwr_pa);
        return res;
}


static long i2cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct i2c_client *client = file->private_data;
        unsigned long funcs;


        switch (cmd) {
        case I2C_SLAVE:
        case I2C_SLAVE_FORCE:
                if ((arg > 0x3ff) ||
                    (((client->flags & I2C_M_TEN) == 0) && arg > 0x7f))
                        return -EINVAL;
//                if (cmd == I2C_SLAVE && i2cdev_check_addr(client->adapter, arg))
//                        return -EBUSY;
                /* REVISIT: address could become busy later */
                client->addr = arg;
                return 0;

        case I2C_RDWR:
                return i2cdev_ioctl_rdrw(client, arg);

        default:
                return -ENOTTY;
        }
        return 0;
}



/* File operations structure. Defined in linux/fs.h */                          
static struct file_operations i2c_fops = {                                      
    .owner              = THIS_MODULE,           /* Owner */                    
    .open               = i2c_driver_open,        /* Open method */             
    .release            = i2c_driver_close,     /* Release method */            
    .write              = i2c_driver_write,       /* Write method */            
    .read               = i2c_driver_read,        /* Read method */             
    //.llseek             = i2c_driver_lseek        /* Lseek method */          
    .unlocked_ioctl	=i2cdev_ioctl
                                                                                
};                                                                              
static struct i2c_driver eeprom_driver={                                        
        .driver={                                                               
        .name="i2c_flash",                                                      
        .owner=THIS_MODULE,                                                     
        },                                                                      
        .class=1,                                                               
//        .probe=i2c_driver_probe,                                              
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
//      register_chrdev(MAJOR(i2c_dev_number),"DEVICE_NAME",&gmem_fops);        
                                                                                
                                                                                
                                                                                
                                                                                
        printk(KERN_INFO "i2c_flash driver initialized.\n");                    
                                                                                
        return 0;                                                               
}                                                                               
/* Driver Exit */                                                               
void __exit i2c_driver_exit(void)                                               
{                                                                               
        int i=0;   
        
printk(KERN_INFO "before adap free\n");                             
        i2c_put_adapter(i2c_devp->client->adapter);                           
printk(KERN_INFO "before kfree\n");                                             
        kfree(i2c_devp->client);                                              
                                                    
                                                                                
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

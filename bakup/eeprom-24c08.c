/*
 * eeprom-24c08.c - eeprom driver for some mostly-compatible I2C chips.
 *
 *  Copyright (C) 2011 William Smith
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
//#include <linux/bcd.h>
/*New Add */
#include <linux/cdev.h>
#include <arm-asm/uaccess.h>
#include <linux/fs.h>

#include <linux/device.h>


#define EEP_MAJOR 236   /* Major number can also be dynamically allocated. */
#define BNK_SIZE 1024	/* BANK SIZE */
#define BLK_SIZE 256	/* BLOCK SIZE */
#define PG_SIZE 16	/* PAGE SIZE */
#define EEPROM_SLAVE_ADDR0 0x50	/* 7bit. Binary: 1010000 */
#define EEPROM_SLAVE_ADDR1 0x51 /* 7bit. Binary: 1010001 */
#define EEPROM_SLAVE_ADDR2 0x52 /* 7bit. Binary: 1010010 */
#define EEPROM_SLAVE_ADDR3 0x53 /* 7bit. Binary: 1010011 */

#define MAX_RETRYS 30

/* For DEBUG */
//#define DEBUG_EEP
#define DEBUG_EEP_R

static int max_retrys_record = 0;

static struct eep_24c08 {
	struct i2c_client *client[4];	/* I2C client for this eeprom */
	struct cdev eep_cdev;		/* cdev */
	dev_t dev_num;				/* dev num */
	unsigned int cur_ptr;		/* Current File pointer */
} *eep;

static int eep_open (struct inode *inode, struct file *file)
{
	#ifdef DEBUG_EEP
	printk(KERN_ALERT "EEPROM open\n");
	#endif

	max_retrys_record = 0;

	eep->cur_ptr = 0;
	return 0;
}

static int eep_release (struct inode *inode, struct file *file)
{
	#ifdef DEBUG_EEP
	printk(KERN_ALERT "EEPROM close\n");
	#endif
	return 0;
}

static loff_t eep_llseek(struct file *filp, loff_t off, int whence)
{
	#ifdef DEBUG_EEP
	printk(KERN_ALERT "EEPROM lseek\n");
	#endif
	
	switch(whence)
	{
		case 0: /* SEEK_SET */
			eep->cur_ptr = off;
			break;
		case 1: /* SEEK_CUR */
			eep->cur_ptr += off;
			break;
		case 2: /* SEEK_END */
			eep->cur_ptr = BNK_SIZE + off;
			break;
		default: /* can't happen */
			return -EINVAL;
	}
	if ((eep->cur_ptr < 0) || (eep->cur_ptr > BNK_SIZE)) {
		printk(KERN_ALERT "EEPROM 'lseek()' call, param mistake\n");
		eep->cur_ptr = 0;
		return -1;
	}
	
	return eep->cur_ptr;
}

static int __rw_page(char *buf, int length, int rw)
{
	int ret,err=0;
	int block;
	unsigned char word_addr;
	struct i2c_adapter *adap;
	struct i2c_msg msg[2];
	int msg_num;
	unsigned char *buf_tmp=NULL;

	block = eep->cur_ptr / BLK_SIZE;
	adap = eep->client[block]->adapter;
	word_addr = eep->cur_ptr % BLK_SIZE;
	#ifdef DEBUG_EEP
	printk(KERN_ALERT "__rw_page(): %s: client[%d]->addr = 0x%02X, cur_ptr = %d, block = %d, word_addr = %d\n", 
		(rw?"write":"read"), block, eep->client[block]->addr, eep->cur_ptr, block, word_addr);
	#endif

	if(rw) {
		/* write data */

		/* alloc memory for buf_tmp */
		buf_tmp = kmalloc(length+1, GFP_KERNEL);
		if (!buf_tmp) {
			printk(KERN_ALERT "EEPROM kmalloc %d failed\n", length+1);
			return -ENOMEM;
		}

		memcpy(buf_tmp+1, buf, length);
		buf_tmp[0] = word_addr;
		msg[0].addr	= eep->client[block]->addr;
		msg[0].flags	= 0;	
		msg[0].len	= length+1;
		msg[0].buf	= buf_tmp;
	} else {
		/* read data */
		msg[0].addr	= eep->client[block]->addr;
		msg[0].flags	= 0;	
		msg[0].len	= 1;
		msg[0].buf	= &word_addr;

		msg[1].addr	= eep->client[block]->addr;
		msg[1].flags	= I2C_M_RD;	
		msg[1].len	= length;
		msg[1].buf	= buf;
	}
	msg_num = rw?1:2;

	#ifdef DEBUG_EEP
	printk(KERN_ALERT "msg_num = %d, msg[0]: addr = 0x%02X, flags = %d, len = %d, buf[0~3] = %02X,%02X,%02X,%02X\n",
			msg_num, msg[0].addr, msg[0].flags, msg[0].len,
			msg[0].buf[0], msg[0].buf[1], msg[0].buf[2], msg[0].buf[3]);
	#endif

	ret =  i2c_transfer(adap, msg, msg_num);
	if (ret<0) {
		#ifdef DEBUG_EEP
		printk(KERN_ALERT "i2c_transfer() err %d\n", ret);
		#endif
		err = ret;
		goto exit;
	} else if (ret<(rw?1:2)) {
		printk(KERN_ERR "__rw_page(): %s: error, actual write i2c_msg number %d, desire i2c_msg number %d\n",
				(rw?"write":"read"), ret, (rw?1:2));
		err = -EIO;
		goto exit;
	}

exit:
	if (rw)
		kfree(buf_tmp);
	return (err?err:length);
}

static int rw_page(char *buf, int length, int rw)
{
	int i;
	int ret,err=0;
	char *buf_tmp;

	/* alloc memory for buf_tmp */
	buf_tmp = kmalloc(length, GFP_KERNEL);
	if (!buf_tmp) {
		printk(KERN_ALERT "EEPROM kmalloc %d failed\n", length);
		return -ENOMEM;
	}

	for (i=0;i<MAX_RETRYS;i++) {
		ret = __rw_page(buf, length, rw);
		if (ret<0)
			continue;
		ret = __rw_page(buf_tmp, length, 0);
		if (ret<0)
			continue;
		ret = memcmp(buf, buf_tmp, length);
		if (ret == 0)	
			break;
		else {
			#ifdef DEBUG_EEP
			printk(KERN_ALERT "rw_page(): %s : memcmp return %d\n", (rw?"write":"read"),  ret);
			printk(KERN_ALERT "Original buf: ");
			for (i=0;i<length;i++)
				printk(KERN_ALERT "%02X ", buf[i]);
			printk(KERN_ALERT "\nRecheck buf: ");
			for (i=0;i<length;i++)
				printk(KERN_ALERT "%02X ", buf_tmp[i]);
			printk(KERN_ALERT "\n");
			#endif
		}
	}
	#ifdef DEBUG_EEP_R
	if (i) {
		printk(KERN_ALERT "Retry %d times.\n", i);
		if (i > max_retrys_record)
			max_retrys_record = i;
	}		
	#endif	
	if (i == MAX_RETRYS) {
		printk(KERN_ALERT "Reach MAX_RETRYS(%d times) limit.\n", MAX_RETRYS);
		err = -EAGAIN;
		goto exit;
	}
	
exit:
	kfree(buf_tmp);
	return (err?err:length);
}

static ssize_t eep_rw (char *buf, size_t count, int rw)
{
	int ret,err=0;
	int length = count;
	int seg;
	char *buf_tmp=NULL, *buf_cur;
	
	#ifdef DEBUG_EEP
	printk(KERN_ALERT "###### eep_rw(), buf=%p, count=%d, rw=%d ######\n",
			buf, count, rw);
	#endif

	/* Check param */
	if (eep->cur_ptr + count > BNK_SIZE) {
		printk(KERN_ALERT "EEPROM Try to %s over BNK_SIZE: \n"
				"\tcur_ptr = %d, count = %d, cur_ptr+count = %d\n",
				(rw?"write":"read"), eep->cur_ptr, count, eep->cur_ptr + count);
		return -EINVAL;
	}
	if (rw != 0 && rw != 1) {
		printk(KERN_ALERT "eep_rw() error, rw param value %d invalid.\n", rw);
		return -EINVAL;
	}

	/* alloc memory for buf_tmp */
	buf_tmp = kmalloc(count, GFP_KERNEL);
	if (!buf_tmp) {
		printk(KERN_ALERT "EEPROM kmalloc %d failed\n", count);
		return -ENOMEM;
	}
	buf_cur = buf_tmp;

	if (rw == 1)
		copy_from_user(buf_tmp, (const char *)buf, count);

	while(length > 0) {
		seg = length>PG_SIZE? PG_SIZE: length;
		#ifdef DEBUG_EEP
		printk(KERN_ALERT "eep_rw(): %s: count = %d, cur_ptr = %d, seg = %d, length = %d\n",
			(rw?"write":"read"), count, eep->cur_ptr, seg, length);
		#endif
		ret = rw_page(buf_cur, seg, rw);
		if (ret<0) {
			printk(KERN_ALERT "EEPROM %s error %d\n", (rw?"write":"read"), ret);
			err = ret;
			goto exit;
		}
		buf_cur += seg;
		length -= seg;
		eep->cur_ptr += seg;
	}

	if (rw == 0)
		copy_to_user(buf, (const char *)buf_tmp, count);

	#ifdef DEBUG_EEP_R
	printk(KERN_ALERT "Max retrys record is %d\n", max_retrys_record);
	#endif

exit:
	kfree(buf_tmp);
	return (err?err:count);
}

static ssize_t eep_read (struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	return eep_rw(buf, count, 0);
}

static ssize_t eep_write (struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	return eep_rw(buf, count, 1);
}


static struct file_operations eep_fops = {
	.owner		= THIS_MODULE,
	.open		= eep_open,
	.release	= eep_release,
	.llseek		= eep_llseek,
	.read		= eep_read,
	.write		= eep_write,
};

static const unsigned short normal[] = 
	{ EEPROM_SLAVE_ADDR0, EEPROM_SLAVE_ADDR1, 
	  EEPROM_SLAVE_ADDR2, EEPROM_SLAVE_ADDR3, 
	  I2C_CLIENT_END };
static const unsigned short ignore[] = { I2C_CLIENT_END };
static const struct i2c_client_address_data eep_addr_data = {
		.normal_i2c	= normal,
		.forces		= NULL,
		.probe		= ignore,
		.ignore		= ignore,
};
/* The higher version of Linux kernel is much more simple. */

/* The device name this driver supported.
 */
static const struct i2c_device_id eep_24c08_id[] = {
	{ "eeprom-blk0", EEPROM_SLAVE_ADDR0 },
	{ "eeprom-blk1", EEPROM_SLAVE_ADDR1 },
	{ "eeprom-blk2", EEPROM_SLAVE_ADDR2 },
	{ "eeprom-blk3", EEPROM_SLAVE_ADDR3 },
	{ }
};
/* Well, there is some necessary to take a explaination.
 * Because of the device 24C08 has 8K bits in it, it need 4 addresses.
 * In the I2C protocal, the SLAVE_ADDR data has only 8 bit, which could only address 256 Bytes(2048 bits).
 * So 24C08 has 4 different SLAVE_ADDR addresses: 0x50, 0x51, 0x52, 0x53. 
 * Each address is a part that has 2048 bits. Totally, 4 parts have 8192 bits(8K bits).
*/

MODULE_DEVICE_TABLE(i2c, eep_24c08_id);

/* eep_probe - probe method for new-style driver model.
 * @client:	the i2c_client which already been register. 
 * @id:		the i2c_device_id which matchs.

 * When dev and driver are both create and matched. This func will be called to init some specfic data.
 * Usually, the device's initial step will be done here.
 * Note: When a driver has N devices, the probe will be called N times.
 *       For different device, different i2c_device_id will be provided for use in the probe function.
*/
static int __devinit eep_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
	int i=0;	
	
	#ifdef DEBUG_EEP
	printk(KERN_ALERT "EEPROM probe\n");
	#endif
	
	while (eep_24c08_id[i].driver_data) {
		if (client->addr == (unsigned short)eep_24c08_id[i].driver_data) {
			#ifdef DEBUG_EEP
			printk(KERN_ALERT "probe(): client->addr = 0x%02X, i = %d, eep_24c08_id[%d].driver_data = 0x%02X, eep_24c08_id[%d].name = %s\n",
				client->addr, i, i, (unsigned int)eep_24c08_id[i].driver_data, i, eep_24c08_id[i].name);
			#endif
			eep->client[i] = client; /* Store the client into eep for further use. */
				/* client can also be stored in file->p when open(). */
		}
		i++;
	}

	return 0;
}

static int __devexit eep_remove(struct i2c_client *client)
{
	#ifdef DEBUG_EEP
	printk(KERN_ALERT "EEPROM remove\n");
	#endif
	
	return 0;
}

/* eep_detect - detect method is called when there is really some device on '@kind' address. 
 @i2c_client: 	the temp client device which is given to detect func's use. 
 			It is only temp which has not been register.
 @kind:	 indicate the number in i2c_client_address_data for each possible member. 
 		only for forces device. Others condition is -1.
 @i2c_board_info: 	the specific infomation's structure to store device's info.

 * This function was called before the real i2c_client is create. 
 * Some special device need a initializion before read/write it, do the init here.
 * Init the client's infomation, and it need to fill at least the name 
 * of i2c_board_info param for registering which is also the name of client device.
 * Notice: Its addr member has already beed set.
 */
int eep_detect(struct i2c_client *client, int kind, struct i2c_board_info *bd_info)
{
	int i=0;

	#ifdef DEBUG_EEP
	printk(KERN_ALERT "EEPROM detect\n");
	#endif
	
	/* There is no need to take action of detect on I2C bus. */

	while (eep_24c08_id[i].driver_data) {
		if (client->addr == (unsigned short)eep_24c08_id[i].driver_data) {
			strlcpy(bd_info->type, eep_24c08_id[i].name, I2C_NAME_SIZE); 
				/* In fact, giving the name is the only thing need to do. */
			bd_info->flags = 0;
			#ifdef DEBUG_EEP
			printk(KERN_ALERT "detect(): i = %d, client->addr = 0x%02X, bd_info->type = %s\n",
				i, client->addr, bd_info->type);
			#endif
		}
		i++;
	}

	/* platform_data should be study later */

	return 0;
}

static struct i2c_driver eep_driver = {
	.driver	= {
		.name	= "eeprom-driver",	/* Name */
		.owner	= THIS_MODULE,
	},
	.class			= 1,
	.probe			= eep_probe,
	.remove			= __devexit_p(eep_remove),
	.id_table		= eep_24c08_id,
	.detect			= eep_detect,
	.address_data		= &eep_addr_data,
};

static int __init eeprom_init(void)
{
	int err=0;
	
	eep = kmalloc(sizeof(struct eep_24c08), GFP_KERNEL);
	if (!eep) {
		err = -ENOMEM;
		goto exit;
	}

	/* Get and register the cdev for eeprom */
	eep->dev_num = MKDEV(EEP_MAJOR,0);
	if (register_chrdev_region(eep->dev_num, 1, "eeprom") <0 ) {
		printk(KERN_ALERT "Can't register device\n");
		goto exit_free;
	}
	
	cdev_init(&eep->eep_cdev, &eep_fops);
	eep->eep_cdev.owner = THIS_MODULE;
	eep->eep_cdev.ops = &eep_fops;
	if (cdev_add(&eep->eep_cdev, eep->dev_num, 1)) {
		printk(KERN_ALERT "Failed to add cdev of EEPROM\n");
		goto exit_unregister;
	}

	/* Register i2c_driver to i2c core */
	err = i2c_add_driver(&eep_driver);
	if (err) {
		printk(KERN_ALERT "Registering I2C driver of EEPROM failed, errno is %d\n", err);
		goto exit_del_cdev;
	}

	#ifdef DEBUG_EEP
	printk(KERN_ALERT "EEPROM Driver Initialized.\n");
	#endif
	
	return 0;

exit_del_cdev:
	cdev_del(&eep->eep_cdev);
exit_unregister:
	unregister_chrdev_region(eep->dev_num, 1);
exit_free:
	kfree(eep);
exit:
	return err;	
}

static void __exit eeprom_exit(void)
{
	i2c_del_driver(&eep_driver);
	cdev_del(&eep->eep_cdev);
	unregister_chrdev_region(eep->dev_num, 1);
	kfree(eep);

	#ifdef DEBUG_EEP
	printk(KERN_ALERT "EEPROM Driver Removed.\n");
	#endif
}

module_init(eeprom_init);
module_exit(eeprom_exit);

MODULE_DESCRIPTION("EEPROM driver for 24C08 and similar chips");
MODULE_LICENSE("GPL");

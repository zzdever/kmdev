#include <stdio.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include "eeprom_user.h"

static unsigned short eeprom_cur_pgoff;
static int eeprom_fd;

int open_EEPROM()
{
	char *i2c_bus = EEPROM_I2C_DEV;

	if ((eeprom_fd = open(i2c_bus, O_RDWR)) <= 0)
	{
		printf("Error opening i2c bus.\n");
		return -1;
	}

	if (ioctl(eeprom_fd, I2C_SLAVE_FORCE, EEPROM_ADDR) < 0) {
		printf("Error while accessing EEPROM. (set slave addr)   %d\n", eeprom_fd);
		close(eeprom_fd);
		return -1;
	}

	return eeprom_fd;
}

int write_EEPROM(void *buf, int n_pages)
{
	int i, ret;
	int write_pos = 0;

	if (NULL == buf)
	{
		return -1;
	}

	for (i = 0; i < n_pages; i++)
	{	
		char tmp[EEPROM_ADDR_SIZE+EEPROM_PAGE_SIZE+1] = {0};

		snprintf(tmp, EEPROM_ADDR_SIZE+EEPROM_PAGE_SIZE+1, 
				"%c%c%s", 
				(eeprom_cur_pgoff*EEPROM_PAGE_SIZE >> 8),
				(eeprom_cur_pgoff*EEPROM_PAGE_SIZE & 0xFF),
				((char *)buf)+write_pos);
		
		do
		{
			ret = write(eeprom_fd, tmp, EEPROM_ADDR_SIZE+EEPROM_PAGE_SIZE);
			if ((ret < 0) && (errno != EREMOTEIO))
			{
				printf("Could not write data. errno=%d", errno);
				return -1;
			}			
		} while (ret < 0);

		write_pos+=EEPROM_PAGE_SIZE;

		if (++eeprom_cur_pgoff >= EEPROM_NUM_PAGES)
		{
			eeprom_cur_pgoff = 0;
		}
	}

	return 0;
}

int eeprom_prepare_msg(struct i2c_msg *msg,
					   unsigned short slave_addr,
					   int is_read,
					   char *buf,
					   int buf_len)
{
	if (msg == NULL)
	{
		return -1;
	}

	msg->addr = slave_addr;
	msg->flags = (is_read ? I2C_M_RD : 0);
	msg->len = buf_len;
	msg->buf = (__u8 *)buf;

	return 0;
}

int read_EEPROM(void *buf, int n_pages)
{
	int ret;
	char mem_addr[EEPROM_ADDR_SIZE];
	struct i2c_msg msg[2];
	struct i2c_rdwr_ioctl_data ioctl_data;

	mem_addr[0] = eeprom_cur_pgoff*EEPROM_PAGE_SIZE >> 8;
	mem_addr[1] = eeprom_cur_pgoff*EEPROM_PAGE_SIZE & 0xFF;

	eeprom_prepare_msg(&msg[0],       /* i2c_msg */
					   EEPROM_ADDR,   /* slave addr */
					   0,             /* isRead */
					   mem_addr,      /* buf */
					   EEPROM_ADDR_SIZE);
	
	eeprom_prepare_msg(&msg[1],       /* i2c_msg */
					   EEPROM_ADDR,   /* slave addr */
					   1,             /* isRead */ 
					   buf,           /* buf */
					   (n_pages * EEPROM_PAGE_SIZE));
	
	ioctl_data.msgs = msg;
	ioctl_data.nmsgs = 2;

	do {
		ret = ioctl(eeprom_fd, I2C_RDWR, &ioctl_data);
		if ((ret < 0) && (errno != EREMOTEIO)) {
			perror("ioctl failed");
			close(eeprom_fd);
			eeprom_fd = -1;
			return -1;
		}
	} while(ret < 0);
	
	*((char *)buf+EEPROM_PAGE_SIZE*n_pages) = '\0';

	eeprom_cur_pgoff += n_pages;
	if (eeprom_cur_pgoff >= EEPROM_NUM_PAGES)
	{
		eeprom_cur_pgoff = 0;
	}

	return 0;
}

int seek_EEPROM(int offset)
{
	if ((offset >= 0) && (offset <= EEPROM_NUM_PAGES-1))
	{
		eeprom_cur_pgoff = offset;
		return 0;
	}

	return -1;
}

int close_EEPROM()
{
	int fd = eeprom_fd;
	eeprom_fd = 0;
	eeprom_cur_pgoff = 0;
	return close(fd);
}

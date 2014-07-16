/*
 a sample program to read/write 24HC256 EEPROM on Quark i2c bus.
 all operations are done via i2c-dev client driver

 the slave EEPROM address is 0x54
 the i2c-dev device is "/dev/i2c-dev0"

*/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "eeprom_user.h"

//#define EEPROM_PAGE_SIZE 64

int show_menu()
{
	printf("****************MENU*****************\n \
	1. Write one page with random characters\n \
	2. Read pages\n \
	3. Seek to page offset\n \
	4. Initialization all pages \n \
	5. Exit\nSelect an operation for the EEPROM: ");

	return 0;
}

int main(int argc, char *argv[])
{
	int ret, i, fd;
	int val = 0;
	char c;
	
	char wdata[EEPROM_PAGE_SIZE+1] = {0};
	char rdata[EEPROM_PAGE_SIZE+1] = {0};

	/* open i2c_flash device */	
	fd = open_EEPROM();

	if (fd < 0)
	{ 
		printf("Can not open device file.\n");		
		return -1;
	}


	while (1)
	{
		show_menu();
		scanf("%c",&c);
		getchar();
		switch (c)
		{
			case '1':
			case 'w':
				/* write page */
				
				for(i=0;i<EEPROM_PAGE_SIZE;i++)
					wdata[i]=rand()%3<2?rand()%26+'A':rand()%10+'0';
				
				ret = write_EEPROM(wdata, 1);
				if (ret == -1 )
				{
					printf("write() failed.\n");
				}
				break;
			case '2':
			case 'r':
				/* read page */
				printf("How many pages to read: ");
				scanf("%d",&val);
				getchar();
				if ((val < 0x0000) || (val >= 0x0200))
				{
					printf("Invalid input...\n");
				}
				else {
					for (i=0; i<val; i++) {
						ret = read_EEPROM(rdata, 1);
						if (ret < 0)
						{
							printf("read() failed.\n");
							break;
						}
						else
						{
							printf("Read Data = %s\n", rdata);
						}
					}
				}
				break;
			case '3':
			case 's':
				/* seek to page offset */
				printf("Choose the input value: ");
				scanf("%d", &val);
				getchar();
				printf("\n");
				if ((val < 0x0000) || (val >= 0x7FFF))
				{
					printf("Invalid input...\n");
				}
				else
				{
					if (seek_EEPROM(val))
					{
						printf("seek failed\n");
					}
				}
				break;
			case '4':
				for(i=0;i<EEPROM_PAGE_SIZE;i++)
					wdata[i]='.';
 				seek_EEPROM(0);

				for (i=0; i<EEPROM_NUM_PAGES; i++) {
					ret = write_EEPROM(wdata, 1);
					if (ret < 0)
					{
						printf("write() failed.\n");
						break;
					}
					else printf("writing done for page  %d\n", i);
					usleep(10000);
				}
				break;
			case '5':				
				/* exit */
				close_EEPROM();
				exit(0);
				break;
			default:
				/* invalid input */
				printf("Invalid input\n");
				break;
		}
	}

	return 0;
}

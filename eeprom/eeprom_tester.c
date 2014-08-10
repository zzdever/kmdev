#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "eeprom_user.h"

int show_menu()
{
	printf("****************MENU*****************\n \
	1. Write Page\n \
	2. Read Page\n \
	3. Seek to Page Offset\n \
	4. Exit\nSelect an operation for the EEPROM: ");

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	int val = 0;
	char c;
	char rdata[EEPROM_PAGE_SIZE] = {0};
	const char *wdata = "thisdataiswrittenintotheEEPROMdeviceandisoflengthsixtyfourbytes!";

	/* open eeprom device */	
	ret = open_EEPROM();

	if (ret < 0)
	{ 
		printf("Can not open device file.\n");		
		return -1;
	}


	while (1)
	{
		show_menu();
		c = getchar();
		getchar();
		printf("\n");
		switch (c)
		{
			case '1':
				/* write page */
				ret = write_EEPROM(wdata, 1);
				if (ret <= 0)
				{
					printf("write() failed.\n");
				}
				break;
			case '2':
				/* read page */
				ret = read_EEPROM(rdata, 1);
				if (ret <= 0)
				{
					printf("read() failed.\n");
				}
				printf("Read Data = %s\n", rdata);
				break;
			case '3':
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
						printf("set failed\n");
					}
				}
				break;
			case '4':				
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

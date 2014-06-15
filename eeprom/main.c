#include <stdio.h>
#include <unistd.h>
#include "eeprom_user.h"

int main(int argc, char *argv[])
{
	int ret;
	char w_data[EEPROM_PAGE_SIZE] = "HELLO, WORLD!";
	char r_data[EEPROM_PAGE_SIZE+1] = {0}; 

	ret = open_EEPROM();
	if (ret <= 0)
	{
		printf("Error opening EEPROM.\n");
		return -1;
	}

	ret = write_EEPROM(w_data, 1);
	if (ret < 0)
	{
		printf("Error writing to EEPROM.\n");
		return -1;
	}

	printf("Write success.\n");

	ret = read_EEPROM(r_data, 1);
	if (ret < 0)
	{
		printf("Error writing to EEPROM.\n");
		return -1;
	}

	printf("read data = %s\n", r_data);

	ret = seek_EEPROM(0);
	
	ret = read_EEPROM(r_data, 1);
	if (ret < 0)
	{
		printf("Error writing to EEPROM.\n");
		return -1;
	}

	printf("read data = %s\n", r_data);

	close_EEPROM();

	return 0;
}

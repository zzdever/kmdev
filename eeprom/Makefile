obj-m+=driver4eeprom.o
#obj-m+=wq_driver4eeprom.o

KDIR:=/opt/clanton-full/1.4.2/sysroots/i586-poky-linux/usr/src/kernel
#PWD:= $(shell pwd)

CC = i586-poky-linux-gcc
ARCH = x86
CROSS_COMPILE = i586-poky-linux-
SROOT=/opt/clanton-full/1.4.2/sysroots/i586-poky-linux/


APP = eeprom_user

all:
#	$(CC) i2c_flash_tester.c eeprom_user.c -o $(APP)
	make -C /usr/src/kernel/ M=$(shell pwd) modules

tester:
	$(CC) eeprom_tester.c eeprom_user.c -o tester

tester2:
	$(CC) i2c_flash_tester.c eeprom_user.c -o tester

clean:
	rm -f *.o
	rm -f *.ko
	rm -f *.mod.*
	rm -f $(APP)


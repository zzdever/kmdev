#ifndef __EEPROM_USER__
#define __EEPROM_USER__

#define EEPROM_I2C_DEV "/dev/i2c-0"
#define EEPROM_ADDR 0x54
#define EEPROM_NUM_PAGES 512
#define EEPROM_PAGE_SIZE 64
#define EEPROM_ADDR_SIZE 2

int open_EEPROM(void);
int write_EEPROM(void *buf, int n_pages);
int read_EEPROM(void *buf, int n_pages);
int seek_EEPROM(int pg_off);
int close_EEPROM(void);

#endif /* __EEPROM_USER__ */

#! /bin/sh

#insmod /lib/modules/3.8.7-yocto-standard/kernel/drivers\
#/i2c/i2c-dev.ko

echo 29 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio29/direction
echo 0 > /sys/class/gpio/gpio29/value

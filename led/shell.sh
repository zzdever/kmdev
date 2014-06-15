#! /bin/sh

echo 28 > /sys/class/gpio/export             
echo out > /sys/class/gpio/gpio28/direction 
echo 17 > /sys/class/gpio/export             
echo out > /sys/class/gpio/gpio17/direction 
for((;;))                                   
do                                                                            
echo 0 > /sys/class/gpio/gpio28/value                                          
echo 1 > /sys/class/gpio/gpio17/value                                          
sleep 1                                                                       
echo 1 > /sys/class/gpio/gpio28/value                                          
echo 0 > /sys/class/gpio/gpio17/value                                          
sleep 1                                                                       
done 

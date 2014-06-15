#include <linux/module.h>	
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/gpio.h>

#define LED1	28
#define LED2	17

static void delay(){
	int i;
	for(i=0;i<400000000;i++);
}

static int __init gpiomod_init(void)
{
	int ret = 0;
	int i=0;

	// register, turn off 
	ret = gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "led1");
	ret = gpio_request_one(LED2, GPIOF_OUT_INIT_LOW, "led2");

	for(i=0;i<10;i++){
	gpio_set_value(LED1, 1); 
	gpio_set_value(LED2, 0);
	delay(); 
	gpio_set_value(LED1, 0); 
	gpio_set_value(LED2, 1);
	delay(); 
	}

	return ret;
}

static void __exit gpiomod_exit(void)
{

	// turn LED off
	gpio_set_value(LED1, 0); 
	
	// unregister GPIO 
	gpio_free(LED1);
}

MODULE_LICENSE("GPL");

module_init(gpiomod_init);
module_exit(gpiomod_exit);

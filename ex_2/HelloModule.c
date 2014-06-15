/*****************************
*
*  A test program for Linux loadable module
*   when the module is installed, display the calendar time
*
***********************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/delay.h>
MODULE_LICENSE("GPL");

static int hello_init(void)
{
int i;
struct timeval curTime;	// keeps the number of seconds and microseconds since the Epoch 
struct tm bkd_time;	// containing a calendar date and time broken down into its components

    for (i=0; i<10; i++)  {
		do_gettimeofday(&curTime);
		time_to_tm(curTime.tv_sec, 0, &bkd_time);
		printk(KERN_ALERT "Hello, world. The current time is: %d:%d:%d:%ld\n", bkd_time.tm_hour, bkd_time.tm_min, bkd_time.tm_sec, curTime.tv_usec);
		msleep(i*1000);
	}
}

static void hello_exit(void)
{
    printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);

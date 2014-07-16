/* 
 * Time driven signal handle
 *
 * yingzhemin@gmail.com
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <linux/hpet.h>
#include <setjmp.h>
#include <sys/poll.h>
#include <sys/ioctl.h>

#define DEVICE_NAME "/dev/hpet"  //hpet device name

static int count;  //count numbers
static jmp_buf env; // jmp environment

/* signal handler */
static void sig_handler(int val)
{
        printf("Sig handler called\n");
	longjmp(env,1);
}


int main(void)
{
        int fd, freq, value, i; 
        struct hpet_info info;
        sig_t oldsig;

	/* install new signal handler */
        if ((oldsig = signal(SIGIO, sig_handler)) == SIG_ERR) {
                fprintf(stderr, "Failed to set signal handler\n");
		return -1;
        }

	/* open hpet device */
        fd = -1;
        fd = open(DEVICE_NAME, O_RDONLY);
        if (fd < 0) {
                fprintf(stderr, "Failed to open %s\n", DEVICE_NAME);
                goto out;
        }

	/* set device owner and flags */
        if ((fcntl(fd, F_SETOWN, getpid()) == 1) ||
                ((value = fcntl(fd, F_GETFL)) == 1) ||
                (fcntl(fd, F_SETFL, value | O_ASYNC) == 1)) {
                fprintf(stderr, "Fcntl failed\n");
                goto out;
        }

	/* set hpet frequency */
	freq=1;
        if (ioctl(fd, HPET_IRQFREQ, freq) < 0) {
                fprintf(stderr, "HPET_IRQFREQ failed\n");
                goto out;
        }
	/* get hpet info */
        if (ioctl(fd, HPET_INFO, &info) < 0) {
                fprintf(stderr, "Failed to get info\n");
                goto out;
        }
	/* check hpet EPI */
        if (info.hi_flags && (ioctl(fd, HPET_EPI, 0) < 0)) {
                fprintf(stderr, "hpet_fasync: HPET_EPI failed\n");
                goto out;
        }
	/* check hpet IE on-off */
        if (ioctl(fd, HPET_IE_ON, 0) < 0) {
                fprintf(stderr, "hpet_fasync, HPET_IE_ON failed\n");
                goto out;
        }

	/* set environment and jump */
	if(setjmp(env)){
		printf("After 1s, calculation done, count=%d\n",count);
                goto out;
	}
	/* start computing task */
	else{	
		printf("Start computing, 1s needed...\n");
		while(1) count++;    // calculation
	} 

out:
	/* restore signal handler */
        signal(SIGIO, oldsig);
        if (fd >= 0)
                close(fd);

        return;
}


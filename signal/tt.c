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



//extern void hpet_fasync(unsigned long freq, int iterations);

#include <sys/poll.h>
#include <sys/ioctl.h>
static int hpet_sigio_count;
static void hpet_sigio(int val)
{
        fprintf(stderr, "hpet_sigio: called\n");
        hpet_sigio_count++;
}

int main(int argc, const char ** argv)
{
        int     i;
sig_t oldsig;

        if ((oldsig = signal(SIGIO, hpet_sigio)) == SIG_ERR) {
                fprintf(stderr, "hpet_fasync: failed to set signal handler\n");
                return;
        }
	hpet_fasync(1, 10);
for(i=0;i<10000000000;i++); 
        fprintf(stderr, "do_hpet: command s not implemented\n");

        return -1;
}




void hpet_fasync(unsigned long freq, int iterations)
{
        int                     i, fd, value;
        sig_t                   oldsig;
        struct hpet_info        info;

        hpet_sigio_count = 0;
        fd = -1;



        fd = open("/dev/hpet", O_RDONLY);

        if (fd < 0) {
                fprintf(stderr, "hpet_fasync: failed to open s\n" );
                return;
        }


        if ((fcntl(fd, F_SETOWN, getpid()) == 1) ||
                ((value = fcntl(fd, F_GETFL)) == 1) ||
                (fcntl(fd, F_SETFL, value | O_ASYNC) == 1)) {
                fprintf(stderr, "hpet_fasync: fcntl failed\n");
                goto out;
        }


        if (ioctl(fd, HPET_IRQFREQ, freq) < 0) {
                fprintf(stderr, "hpet_fasync: HPET_IRQFREQ failed\n");
                goto out;
        }

        if (ioctl(fd, HPET_INFO, &info) < 0) {
                fprintf(stderr, "hpet_fasync: failed to get info\n");
                goto out;
        }

        fprintf(stderr, "hpet_fasync: info.hi_flags 0x%lx\n", info.hi_flags);

        if (info.hi_flags && (ioctl(fd, HPET_EPI, 0) < 0)) {
                fprintf(stderr, "hpet_fasync: HPET_EPI failed\n");
                goto out;
        }

        if (ioctl(fd, HPET_IE_ON, 0) < 0) {
                fprintf(stderr, "hpet_fasync, HPET_IE_ON failed\n");
                goto out;
        }

out:
        signal(SIGIO, oldsig);

        if (fd >= 0)
                close(fd);

        return;
}

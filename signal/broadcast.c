/*
 * Broadcast signal to all threads
 * that does not block this signal
 *
 * Comments of codes relate to hpet can be found in time-driven.c 
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
#include <pthread.h>

#define DEVICE_NAME "/dev/hpet"
#define SIGIO_RECIEVED (SIGUSR2+1)  //self-defined signal to broadcast and notify

/* threads node struct */
struct Threads{
	pthread_t tid;
	struct Threads *next;
};

static int count;  //count numbers
static jmp_buf env;  //jmp environment
static struct Threads *thread_list_head=NULL; //threads list head

/* add new thread to the list */
void AddThread(pthread_t tid)
{
	/* first node */
	if(thread_list_head==NULL){
		thread_list_head=(struct Threads*)malloc(sizeof(struct Threads));
		thread_list_head->tid=tid;
		thread_list_head->next=NULL;
	}
	/* more nodes */
	else{	
		struct Threads *tmp;

		tmp=thread_list_head;
		/* find last node */
		while(tmp->next!=NULL) tmp=tmp->next;
		tmp->next=(struct Threads *)malloc(sizeof(struct Threads));
		tmp=tmp->next;
		tmp->tid=tid;
		tmp->next=NULL;
	}
	return;
}

/* signal handler */
static void sig_handler(int val)
{
        printf("Sig handler called\n");
}
/* broadcast signal to all threads */
static void sig_broadcaster(int val)
{
	struct Threads *tmp;
	printf("Broadcasting SIGIO...\n");
	
	tmp=thread_list_head;
	/* find thread and send signal */
	while(tmp!=NULL){
		pthread_kill(tmp->tid,SIGIO_RECIEVED);
		tmp=tmp->next;
	} 
}

/* threads to be creatd */
void *thread1(void *arg)
{
	while(1){
		(void)pause();
		printf("Thread 1 recieved\n");
	}
}
void *thread2(void *arg)
{
	while(1){
		(void)pause();
		printf("Thread 2 recieved\n");
	}
}


int main(void)
{
        int fd, freq, value, i; 
        struct hpet_info info;
	pid_t pid;
	pthread_t tid1,tid2;
        sig_t oldsig;
	sigset_t set;

	/* install signal handler of */
        if ((oldsig = signal(SIGIO_RECIEVED, sig_handler)) == SIG_ERR) {
                fprintf(stderr, "Failed to set signal handler\n");
		return;
        }

	/* create threads */
	value=pthread_create(&tid1, NULL, thread1, NULL);
        if(value!=0){
                fprintf(stderr, "Failed to create thread 1\n");
                return -1;
        }
	/* add thread to list */
	AddThread(tid1);
	value=pthread_create(&tid2, NULL, thread2, NULL);
        if(value!=0){
                fprintf(stderr, "Failed to create thread 2\n");
                return -1;
        }
	AddThread(tid2);

	/* block SIGIO_RECIEVED in main thread */
	if(sigemptyset(&set)<0)
		fprintf(stderr,"Failed to init sig set\n");
	if(sigaddset(&set,SIGIO_RECIEVED)<0)
		fprintf(stderr,"Failed to add sig\n");
	if(pthread_sigmask(SIG_BLOCK,&set,NULL)<0)
		fprintf(stderr,"Failed to add a block sig set\n");

	/* install signal handler to broadcast */
        if ((oldsig = signal(SIGIO, sig_broadcaster)) == SIG_ERR) {
                fprintf(stderr, "Failed to set signal broadcast handler\n");
		return;
        }

        fd = -1;
        fd = open(DEVICE_NAME, O_RDONLY);
        if (fd < 0) {
                fprintf(stderr, "Failed to open %s\n", DEVICE_NAME);
                goto out;
        }

        if ((fcntl(fd, F_SETOWN, getpid()) == 1) ||
                ((value = fcntl(fd, F_GETFL)) == 1) ||
                (fcntl(fd, F_SETFL, value | O_ASYNC) == 1)) {
                fprintf(stderr, "Fcntl failed\n");
                goto out;
        }

	freq=1;
        if (ioctl(fd, HPET_IRQFREQ, freq) < 0) {
                fprintf(stderr, "HPET_IRQFREQ failed\n");
                goto out;
        }
        if (ioctl(fd, HPET_INFO, &info) < 0) {
                fprintf(stderr, "Failed to get info\n");
                goto out;
        }
        if (info.hi_flags && (ioctl(fd, HPET_EPI, 0) < 0)) {
                fprintf(stderr, "hpet_fasync: HPET_EPI failed\n");
                goto out;
        }
        if (ioctl(fd, HPET_IE_ON, 0) < 0) {
                fprintf(stderr, "hpet_fasync, HPET_IE_ON failed\n");
                goto out;
        }


	pthread_join(tid1,NULL);
	pthread_join(tid2,NULL);
	while(1)count++;

out:
	/* restore signal handler */
        signal(SIGIO, oldsig);
        if (fd >= 0)
                close(fd);

        return;
}


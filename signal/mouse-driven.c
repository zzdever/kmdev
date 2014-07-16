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
#include <setjmp.h>                                                             
#include <sys/poll.h>                                                           
#include <sys/ioctl.h>                                                          
#include <linux/input.h>
#include <sys/stat.h>
#include <pthread.h>

#define MOUSE "/dev/input/mice"  //mouse device name
#define RIGHT 2  //right mouse button type number 
#define TIME_THRES 500000 //double click time threshold
#define MOUSE_2X_RIGHT_SIG (SIGUSR2+1) //self-defined signal

static int count; //count numbers
static jmp_buf env; //jmp environment

/* signal handler */
static void sig_handler(int val)                                                
{                                                                               
	printf("Sig handler called\n");                                         
	longjmp(env,1);                                                         
}                                                                               

void *MouseListen(void*);

int main(void)                                                                  
{                                                                               
	int fd, freq, value, i;                                                 
	sig_t oldsig;                                                           
	pthread_t tidMouse;

	/* create thread to listen to mouse */
	value=pthread_create(&tidMouse, NULL, MouseListen, NULL);
	if(value!=0){
		fprintf(stderr, "Failed to create mouse listen thread\n");
		return -1;
	}

	/* install new signal handler */
	if ((oldsig = signal(MOUSE_2X_RIGHT_SIG, sig_handler)) == SIG_ERR) {                 
		fprintf(stderr, "Failed to set signal handler\n");              
		return -1;
	}                                                                       


	/* set environment and jump */
	if(setjmp(env)){                                                        
		printf("Mouse right double clicked, count=%d\n",count);         
		goto out;
	}                                                                       
	/* start task */
	else{                                                                   
		printf("Start waiting for mouse right double click...\n");                      
		while(1) count++;    // calculation                             
	}                                                                       

out:                                                                            
	/* restore signal handler */
	signal(MOUSE_2X_RIGHT_SIG, oldsig);                                                  
	if (fd >= 0)                                                            
		close(fd);                                                      

	return;                                                                 
} 

void *MouseListen(void *arg)
{
	int fd, retval;
	char rightClicked;
	char buf[6];
	fd_set readfds;
	struct timeval tv, tv_mouse1, tv_mouse2;
	int type, dX, dY, dZ;

	/* open mouse device file */
	if(( fd = open(MOUSE, O_RDONLY))<0)
	{
		printf("Failed to open %s\n", MOUSE);
		exit(1);
	}

	while(1)
	{
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		/* empty read file set */
		FD_ZERO(&readfds);
		/* add mouse file to read file set */
		FD_SET(fd, &readfds);

		/* select mouse */
		retval = select(fd+1, &readfds, NULL, NULL, &tv);
		if(retval==0)
			printf("Time out!\n");
		if(FD_ISSET(fd,&readfds))
		{
			if(read(fd, buf, 6) <= 0)  //read failed
			{
				continue;
			}
			/* get state of mouse */
			type = buf[0]&0x07; // click type
			dX=buf[1];  // X offset
			dY=buf[2];  // Y offset
			dZ=buf[3];  // Z offset, not used
			printf("Button type = %d, X = %d, Y = %d, Z = %d\n",\
				type, dX, dY, dZ);

			if(type == RIGHT){
			/* first click captured */
			if(rightClicked==0){
				rightClicked=1;
				gettimeofday(&tv_mouse1,NULL);
			}
			else {
			/* second click captured */ 
			gettimeofday(&tv_mouse2,NULL);
			/* check the time interval */
			if((tv_mouse2.tv_sec*1000000+tv_mouse2.tv_usec\
			-tv_mouse1.tv_sec*1000000-tv_mouse1.tv_usec)<TIME_THRES)
			{
				/* right double click captured */
				printf("right double clicked\n");
				raise(MOUSE_2X_RIGHT_SIG);  
				rightClicked=0;
			}
			else{
				/* time interval too long, not double click */
				rightClicked=1;
				gettimeofday(&tv_mouse1,NULL);
			}
			}
			}
		}
	}
	/* close mouse device file */
	close(fd);
	return;
}

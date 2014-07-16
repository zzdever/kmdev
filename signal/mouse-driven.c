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

#define MOUSE "/dev/input/mice"
#define RIGHT 2
#define TIME_THRES 500000
#define MOUSE_2X_RIGHT_SIG (SIGUSR2+1)

static int count;                                                               
static jmp_buf env;                                                             

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

	value=pthread_create(&tidMouse, NULL, MouseListen, NULL);
	if(value!=0){
		fprintf(stderr, "Failed to create mouse listen thread\n");
		return -1;
	}

	if ((oldsig = signal(MOUSE_2X_RIGHT_SIG, sig_handler)) == SIG_ERR) {                 
		fprintf(stderr, "Failed to set signal handler\n");              
		return -1;
	}                                                                       


	if(setjmp(env)){                                                        
		printf("Mouse right double clicked, count=%d\n",count);         
		goto out;
	}                                                                       
	else{                                                                   
		printf("Start waiting for mouse right double click...\n");                      
		while(1) count++;    // calculation                             
	}                                                                       

out:                                                                            
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

	if(( fd = open(MOUSE, O_RDONLY))<0)
	{
		printf("Failed to open %s\n", MOUSE);
		exit(1);
	}

	while(1)
	{
		tv.tv_sec = 5;
		tv.tv_usec = 0;

		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);

		retval = select(fd+1, &readfds, NULL, NULL, &tv);
		if(retval==0)
			printf("Time out!\n");
		if(FD_ISSET(fd,&readfds))
		{
			if(read(fd, buf, 6) <= 0)
			{
				continue;
			}
			type = buf[0]&0x07;
			dX=buf[1];
			dY=buf[2];
			dZ=buf[3];
			printf("Button type = %d, X = %d, Y = %d, Z = %d\n",\
				type, dX, dY, dZ);

			if(type == RIGHT){
			if(rightClicked==0){
				rightClicked=1;
				gettimeofday(&tv_mouse1,NULL);
			}
			else {
			gettimeofday(&tv_mouse2,NULL);
			if((tv_mouse2.tv_sec*1000000+tv_mouse2.tv_usec\
			-tv_mouse1.tv_sec*1000000-tv_mouse1.tv_usec)<TIME_THRES)
			{
				printf("right double clicked\n");
				raise(MOUSE_2X_RIGHT_SIG);  
				rightClicked=0;
			}
			else{
				rightClicked=1;
				gettimeofday(&tv_mouse1,NULL);
			}
			}
			}
		}
	}
	close(fd);
	return;
}

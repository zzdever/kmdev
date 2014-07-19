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

struct Task{
	int period;
	int priority;
	int iterationBefore, iterationIn, interationAfter;
	int mutexNum;
};

int main(void)
{
	int i;
	int taskAmount;
	int execTime;
	struct Task* tasks[100];
	
	scanf("%d%d", &taskAmount, &execTime);
	for(i=0;i<taskAmount;i++){
		tasks[i]=(struct Task*)malloc(sizeof struct Task);
		if(task[i]==NULL) printf("Task %d init error\n",i);
		
	}

	return 0;
} 

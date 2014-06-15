/* Hello World program and display the current time */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(void) {
    int i;
    time_t curTime;


    for (i=0; i<10; i++)  {
    	curTime = time(NULL);
		printf("Hello, world. The current time is: %s", ctime (&curTime));
		sleep(i);
	}
}

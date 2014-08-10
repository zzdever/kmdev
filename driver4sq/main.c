#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include <sys/time.h>
#include <linux/ioctl.h>
#include <linux/rtc.h>*/
#include <time.h>

#define DEV_NAME "/dev/gmem0"

int main(int argc, char **argv)
{
	int fd, res;
	char buff[1024];
	int i = 0;

	if(strcmp("0", argv[1]) == 0){
		i=100/i;
	}else if(strcmp("1", argv[1]) == 0){
		;
	}

	/* open devices */
	fd = open(DEV_NAME, O_RDWR);

	if (fd < 0 ){
		printf("Can not open device file.\n");		
		return 0;
	}else{
		if(strcmp("show", argv[1]) == 0){
			memset(buff, 0, 1024);
			res = read(fd, buff, 256);
			sleep(1);
			printf("'%s'\n", buff);
		}else if(strcmp("write", argv[1]) == 0){
			memset(buff, 0, 1024);
			if(argc >= 3){
				sprintf(buff,"%s", argv[2]);
				for(i = 3; i < argc; i++)
					sprintf(buff,"%s %s",buff,argv[i]);
			}
			printf("'%s'\n", buff);
			res = write(fd, buff, strlen(buff)+1);
			if(res == strlen(buff)+1){
				printf("Can not write to the device file.\n");		
				return 0;
			}	
		}
		/* close devices */
		close(fd);
	}
	return 0;
}

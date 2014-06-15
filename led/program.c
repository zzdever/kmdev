#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
int main(int argc, char **argv)
{
	int fd,fd1, res;
	int i = 0;


	fd = open("/sys/class/gpio/export", O_RDWR);
	write(fd,"17");
	write(fd,"28");
	close(fd);

	fd = open("/sys/class/gpio/gpio28/direction", O_RDWR);
	if (fd < 0 ){
		printf("Can not write direction: 28\n");
		return 0;
	}
	write(fd,"out");
	close(fd);
	fd = open("/sys/class/gpio/gpio17/direction", O_RDWR);
	if (fd < 0 ){
		printf("Can not write direction: 17\n");
		return 0;
	}
	write(fd,"out");
	close(fd);

	fd = open("/sys/class/gpio/gpio28/value", O_RDWR);
	if (fd < 0 ){
		printf("Can not write value: 28\n");
		return 0;
	}
	fd1 = open("/sys/class/gpio/gpio17/value", O_RDWR);
	if (fd < 0 ){
		printf("Can not write value: 17\n");
		return 0;
	}

	for(i=0;i<10;i++){
	write(fd,"1");
	write(fd1,"0");
	sleep(1);
	write(fd,"0");
	write(fd1,"1");
	sleep(1);
	}
	close(fd);
	close(fd1);

	return 0;
}

/*
 BASIC COMMAND LINE PROGRAM TO USE THE GMEM DRIVER.

 It has two main uses, read and write.
 Both these functions are used to read and write to the driver.

 */

#include<stdio.h>
#include<fcntl.h>
#include<string.h>

#define DRIVER_NAME "/dev/gmem"
#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
	int determine_command(char *);//determine which command was input...ie either help/read/write
	void format_output(int, char *);

	int choice = -1;
	char *argv2 = "";

	switch (argc) {
	case 1:
		printf(
				"Gmem_test: you must specify either read or write\nPlease type gmem_test -help for further details\n");
		return 0;

	case 2:
	case 3:
		choice = determine_command(argv[1]);

		if (argc > 2)//if more than 2 command line arguments then either a no or string was passed for read/write
			argv2 = argv[2];//since argv is local to main() only store it and pass it as parameters

		format_output(choice, argv2);
		return 0;

	default:
		printf(
				"Gmem_test: too many arguments\nPlease type gmem_test -help for usage information\n");
		return 0;

	}

	//	/if(argc>2)
//		argv2=argv[2];

//		format_output(choice,argv2);
	return 0;
}

int determine_command(char *string) {

	if (!strcmp(string, "-help") || (!strcmp(string, "help")))//many options to enable legacy input and avoid frustration
		return 1;

	if (!strcmp(string, "-read") || (!strcmp(string, "read"))
			|| (!strcmp(string, "-show")) || (!strcmp(string, "show")))
		return 2;

	if (!strcmp(string, "-write") || (!strcmp(string, "write")))
		return 3;

	return -1;
}

void format_output(int choice, char *argv2) {
	void print_help(void);
	void read_driver(char *);
	void write_driver(char *);

	switch (choice) {
	case -1:
		printf("Invalid command. Type gmem_test -help for details\n");
		return;

	case 1:
		print_help();
		return;

	case 2:
		read_driver(argv2);
		return;

	case 3:
		write_driver(argv2);
		return;
	}
}

void print_help(void) {
	printf(
			"\nGmem_test: A helper program for the driver gmem\nPossible usage is gmem_test [option] <string>\n\n");
	printf("Option\t\t\tDescription\n-----\t\t\t-----------\n\n");
	printf(
			"-help\t\t\tDisplay the help\n-read/show x\t\tShow the contents of the driver specified by x bytes. If blank -> displays the entire content\n");
	printf(
			"-write <string>\t\twrite a string to the driver specified in <string>\n\n");
	return;
}

void read_driver(char *string) {
	int LENGTH = -1, num = -1;

	LENGTH = strcmp(string, "") == 0 ? BUFFER_SIZE : atoi(string);//if no arguments passed to read default to read the entire buffer..else read only specified bytes

	if (LENGTH > BUFFER_SIZE || LENGTH < 0)	//attempt to access out of bounds
			{
		printf(
				"SECURITY VIOLATION...ATTEMPT TO ACCESS ILLEGAL MEMORY LOCATIONS\n");
		return;
	}

	char buf[BUFFER_SIZE];
	memset(buf, 0, BUFFER_SIZE);

	int fp = open(DRIVER_NAME, 0);	//open driver

	num = read(fp, &buf, LENGTH);
	//printf("x is %d ",x);..DEBUGGING

	if (num == 0) {
		printf("READ FAILED\n");
		return;
	} else if (num < LENGTH && LENGTH != BUFFER_SIZE)
		printf(
				"Sorry only %d bytes could be read\nPERHAPS YOU ARE TRYING TO READ AN UNFILLED POSITION OR THE DEVICE IS BUSY",
				num);

	printf("Driver contents: %s\n", buf);

	//printf("length is %d\n",LENGTH);...DEBUGGING

}

void write_driver(char *string) {
	int LENGTH = -1;

	if ((LENGTH = strlen(string)) == 0)	// you give the command to write but do not specify a string
			{
		printf(
				"ERROR: Gmem_test: write expects a string to be written\nPlease check gmem_test -help for usage");
		return;
	}

	//printf("Write_driver has been called with string %s\n",string);	DEBUGGING

	int fp = open(DRIVER_NAME, 0);

	int num = write(fp, string, LENGTH);

	if (num == 0) {
		printf("WRITE FAILED");
		return;
	} else if (num < LENGTH)
		printf("Sorry only %d bytes could be written\n", num);
}


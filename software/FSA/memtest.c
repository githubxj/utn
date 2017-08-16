#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>

void printUsage()
{
	printf("Usage: memtest <options> \n");
	printf("Options: \n");
	printf("\t -s Memory test size in MB . \n ");
	printf("\t -h Print this help message. \n ");
}

int main(int argc, char *argv[])
{	
	extern char *optarg;
	extern int optind;
	int c;
	int i, err, size=0;
	int pattern=0;
	int * memptr;

	while ((c = getopt(argc, argv, "s:h")) != EOF) {
		switch(c) {
		case 's':
			if(optarg != NULL)
				sscanf(optarg, "%d", &size);
			break;
		case 'h':
			printUsage();
			exit(1);
		}
	}

	if(size <= 0)
	{
		printf("Incorrect test memory size\n");
		return 1;
	}

	printf("Try to malloc %d bytes memory\n", size*1024*1024);
	size = size*1024*1024/sizeof(int);

	memptr = (int *) calloc(sizeof(int), size);
	if(memptr == NULL)
	{
		printf("Request memory failed\n");
		return 1;
	}

	while(1)
	{
//		printf("Test pattern %x\n", pattern);
		err = 0;

		for(i=0; i<size; i++)
			*(memptr + i) =  pattern + i;

		for(i=0; i<size; i++)
			if(*(memptr + i) !=  (pattern + i))
				err++;

		if(err != 0)
			printf("Error:%d, pattern=%x\n", err, pattern);
		
		if(pattern<0)
			pattern--;

		pattern = ~pattern;
	}
        return 0;
}


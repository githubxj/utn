#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "fsalib.h"

#define	RCVMSG_LOOP_PERIOD		40000 /*40 ms*/
#define 	FPGA_NAME		"/dev/fpga"

int		debugFpga;
 int		dumpFifo;
extern int		fdFpga;

int handle_fifomsg();

int convertHEX(char *buffer, int len)
{
	char temp[4];
	int i, retLen;
	int data;

	retLen=0;
	i = 0;
	while (i<len)
	{
		/* get 2 char*/
		while((buffer[i] == ' ') && (i<len) )
			i++;
		
		if(i >= len)
			break;
		
		temp[0] = buffer[i];
		i++;
		if(i >=len || buffer[i] == ' ')
			temp[1] = 0;
		else {
			temp[1] = buffer[i];
			temp[2] = 0;
			i++;
		}

		data = 0;
		sscanf(temp, "%x", &data);
		*(buffer + retLen) = (char)data;
		retLen++;		
			
	}

	return retLen;
	
}


void consolSim()
{
	unsigned char buffer[1024];
	int len, sendLen;


	if(fsalib_init() == -1)
	{
		printf("fsa lib init failed!\n");
		exit(1);
	}

	while(1)
	{
		sleep(1);
		printf("Please input:\n");
		if(gets(buffer) >=0 )
		{	
			len = strlen(buffer);

			sendLen = convertHEX(buffer, len);
			
			if( send_fifomsg_waitrsp(buffer[4], buffer[7], buffer + 8, sendLen, 1024, 10000) !=0 )
			{
				printf("Send fifo message error !\n");
				continue;
			}

		}		
	}
}

void recvSim()
{
	fdFpga = open(FPGA_NAME, O_RDWR);
    	if(fdFpga < 0) {
        	printf("ERROR:FPGA: open %s error\n", FPGA_NAME);
		return -1;
    	} 

	printf("Receive and print FIFO messages!\n");
	handle_fifomsg();

}


void print_usage()
{
		printf("Usage: fpga_fifo <options> \n");
		printf("Options: \n");
		printf("\t -i Send out input data(HEX Format) and wait response. \n ");
		printf("\t -d Receive data. \n ");
}
		

int main(int argc, char *argv[])
{	

	debugFpga = 0;
	dumpFifo = 1;

	if((argc >= 2) && (strcmp(argv[1], "-h") == 0))
	{
		print_usage();
		return 0;
	}

	if((argc >= 2) && (strcmp(argv[1], "-i") == 0))
		consolSim();
	
	if((argc >= 2) && (strcmp(argv[1], "-d") == 0))
		recvSim();
	
	print_usage();

        return 0;
}

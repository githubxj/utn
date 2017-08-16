#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/serial_comm.h"

#define	RCVMSG_LOOP_PERIOD		40000 /*40 ms*/

const char 	dev485[] = "/dev/ttyS1";
int			 fd485;
unsigned char	 card_no;


int sim_receive(const char * recv_buff,  int timeout)
{
	int  retLen, receivedLen;
	fd_set inputfds;
	struct timeval tv;
	int retval;
	int extra_wait;

	/* wait for the response */
	FD_ZERO(&inputfds);
	FD_SET(fd485, &inputfds);
		   
	/* Wait the peer response up to TIMEOUT_485MSG ms. */
	tv.tv_sec = 0;
	tv.tv_usec = timeout;
	receivedLen = 0;
	extra_wait = 0;

	while(1)
	{
		retval = select(fd485 +1, &inputfds, NULL, NULL, &tv);
		if (retval == -1) {
             		return -1;
		} else if (retval) {
			if(FD_ISSET(fd485, &inputfds))
 			{
 				retLen = read(fd485, (void *)(recv_buff + receivedLen), MAX_485MSG_LEN - receivedLen);
				receivedLen += retLen;

				if(receivedLen >= MAX_485MSG_LEN)
						return(receivedLen);
					else 
						continue;
			}
			
		} else 
			return(receivedLen);
	}

	return 0;

}

int sim_send(const char * send_buff, int length)
{
	int  retLen, sentLen;
	fd_set outpufds;
	struct timeval tv;
	int retval;

	/* wait for the response */
	FD_ZERO(&outpufds);
	FD_SET(fd485, &outpufds);
		   
	/* Wait the peer response up to 1 s. */
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	sentLen = 0;

	while(1)
	{
		retval = select(fd485 +1, NULL, &outpufds, NULL, &tv);
		if (retval == -1) {
             		return -1;
		} else if (retval) {
			if(FD_ISSET(fd485, &outpufds))
 			{
				/* sent out the message */
				retLen= write(fd485, (void*)(send_buff + sentLen), length - sentLen);
				if(retLen <= 0)
				{
					printf("Error! Sending Msg failed!");
					return(-1);
				}

				sentLen += retLen;
				if(sentLen == length)
					return(sentLen);
				else 
					continue;
			}			
		} else {
			/* send message time out */
			printf("Error! Send message timeout! sent length:%d!\n", sentLen);
			return(-1);			
		}
	}

}

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

void consolSim(int hex)
{
	char buffer[1024];
	char text[17];
	int len, i, sendLen;

	while(1)
	{
		printf("Please input:\n");
		if(gets(buffer) >=0 )
		{	
			len = strlen(buffer);
			if(len == 0)
				continue;
			
			if(hex)
				sendLen = convertHEX(buffer, len);
			else 
				sendLen = len;
			
			printf("sending...%d\n", sendLen);
			
			sim_send(buffer, sendLen);

			len = sim_receive(buffer, 2000000);

			printf("Received:%d\n", len);
			for(i =0; i< len; i++)
			{
				printf("%02x ", buffer[i]);
				if( buffer[i] >= 0x20 && buffer[i] < 0x7f)
					text[i%16] = buffer[i];
				else 
					text[i%16] = '.';
				
				if((i+1) % 16 == 0) 
				{
					text[16] = '\0';
					printf(" %s\n", text);	
				}
			}

			if( i%16 != 0)
			{	
				text[i%16] = '\0';

				for(;i%16 != 0; i++)
					printf("   ");
				
				printf(" %s\n", text);
			}

			printf("\n");
		}		
	}
}

void recvSim()
{
	char buffer[1024];
	char text[17];
	int len, i;

	while(1)
	{
 		len = read(fd485, (void *)buffer, 1024);
		if(len <=0) {
			printf("Receive none\n");
			continue;
		}
		for(i =0; i< len; i++)
		{
			printf("%02x ", buffer[i]);
			if( buffer[i] >= 0x20 && buffer[i] < 0x7f)
				text[i%16] = buffer[i];
			else 
				text[i%16] = '.';
			
			if((i+1) % 16 == 0) 
			{
				text[16] = '\0';
				printf(" %s\n", text);	
			}
		}

		if( i%16 != 0)
		{	
			text[i%16] = '\0';

			for(;i%16 != 0; i++)
				printf("   ");
			
			printf(" %s\n", text);
		}

		printf("\n");

	}
}

void echoSim()
{
	char buffer[1024];
	int len, i;

	while(1)
	{
 		len = read(fd485, (void *)buffer, 1);
		if(len <=0) {
			printf("Receive none\n");
			continue;
		}
		printf("%x \n", buffer[0]);
		write(fd485, (void*)buffer, 1);		
	}
}

void loopSim()
{
	char buffer[1024];
	int len;

	strcpy(buffer, "UUUUUUU");
	len = strlen(buffer);
	while(1)
	{
		if(write(fd485, (void*)buffer, len) <= 0)
			printf("Send failed \n");	
		
		sleep(1);
	}
}

int main(int argc, char *argv[])
{	
	/*initialize the 485 communication port */
	fd485 = OpenDev(dev485);
	if (fd485 > 0)
		set_speed(fd485, 115200);
	else
	{
		printf("Can't Open 485 Serial Port!\n");
		exit(1);
	}
	
	if (set_Parity(fd485,8,1, 0)== -1)
	{
		printf("Set 485 Serial Port Parity Error\n");
		exit(1);
	}

	if((argc >= 2) && (strcmp(argv[1], "-h") == 0))
	{
		printf("Usage: comtest <options> \n");
		printf("Options: \n");
		printf("\t -i Send out input data. \n ");
		printf("\t -ih Send out input data(HEX Format). \n ");
		printf("\t -d Receive data. \n ");
		printf("\t -e Echo. \n ");
		printf("\t -l Loop Send. \n ");
		
		return 0;
	}
	
	if((argc >= 2) && (strcmp(argv[1], "-e") == 0))
		echoSim();

	if((argc >= 2) && (strcmp(argv[1], "-i") == 0))
		consolSim(0);
	
	if((argc >= 2) && (strcmp(argv[1], "-ih") == 0))
		consolSim(1);

	if((argc >= 2) && (strcmp(argv[1], "-l") == 0))
		loopSim();

	if((argc >= 2) && (strcmp(argv[1], "-d") == 0))
		recvSim();
	
		printf("Usage: comtest <options> \n");
		printf("Options: \n");
		printf("\t -i Send out input data. \n ");
		printf("\t -ih Send out input data(HEX Format). \n ");
		printf("\t -d Receive data. \n ");
		printf("\t -e Echo. \n ");
		printf("\t -l Loop Send. \n ");


        return 0;
}

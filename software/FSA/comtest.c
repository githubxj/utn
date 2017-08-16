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
#include <termios.h> /* POSIX terminal control definitions */


#define	RCVMSG_LOOP_PERIOD		40000 /*40 ms*/
#define	MAX_MSG_LEN				1024

#define 	MODE_SEND			1
#define 	MODE_RECV			2
#define 	MODE_LOOP			3
#define 	MODE_ECHO			4
#define 	MODE_RSEQ			5

int			 fd485;
int 	print_error;

int baud_def[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300 };
int baud_val[] = {  115200,  57600,  38400,  19200,  9600,  4800,  2400,  1200,  300 };
int baud_cnt = sizeof(baud_def)/sizeof(int);

void set_speed(int fd, int speed){
	int i;
	int status;
	struct termios Opt;

	tcgetattr(fd, &Opt);
	
	for ( i= 0; i < baud_cnt; i++) 
	{
		if (speed == baud_val[i]) {
			
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, baud_def[i]);
			cfsetospeed(&Opt, baud_def[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			
			if (status != 0) {
				perror("tcsetattr fd1");
				return;
			}

			tcflush(fd,TCIOFLUSH);
		}
	}
	tcgetattr(fd, &Opt);
  //      printf("COMM FLAG c:%o, i:%o, o:%o\n", Opt.c_cflag, Opt.c_ispeed, Opt.c_ospeed);
}

/**
*@brief 设置串口数据位，停止位和效验位
*@param fd 类型 int 打开的串口文件句柄
*@param databits 类型 int 数据位 取值 为 7 或者8
*@param stopbits 类型 int 停止位 取值为 1 或者2
*@param parity 类型 int 效验类型 取值为N,E,O,,S
*/
int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	
	if ( tcgetattr( fd,&options) != 0) {
		perror("SetupSerial 1\n");
		return(-1);
	}

	options.c_cflag &= ~CSIZE;

	switch (databits) /*设置数据位数*/
	{
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr,"Unsupported data size\n"); return (-1);
	}

	switch (parity)
	{
	case 'n':
	case 'N':
	case  0 :
		options.c_cflag &= ~PARENB; /* Clear parity enable */
		options.c_iflag &= ~INPCK; /* Disnable  parity checking */
		break;
	case 'o':
	case 'O':
	case  1 :
		options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
		options.c_iflag |= INPCK; /*  Enable parity checking */
		break;
	case 'e':
	case 'E':
	case  2 :
		options.c_cflag |= PARENB; /* Enable parity */
		options.c_cflag &= ~PARODD; /* 转换为偶效验*/
		options.c_iflag |= INPCK; /* Disnable parity checking */
		break;
	case 'S':
	case 's': /*as no parity*/
	case  3 :
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported parity\n");
		return (-1);
	}

	/* 设置停止位*/
	switch (stopbits)
	{
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported stop bits\n");
		return (-1);
	}

	options.c_iflag &= ~IXON;
    	options.c_iflag &= ~IXOFF;
    	options.c_iflag |= IGNPAR;
    	options.c_iflag &= ~ICRNL;
 
    	options.c_lflag &= ~ISIG;
    	options.c_lflag &= ~ICANON;
    	options.c_lflag &= ~IEXTEN;
    	options.c_lflag &= ~ECHO;
    	options.c_lflag &= ~ECHOE;
    	options.c_lflag &= ~ECHOK;
    	options.c_lflag &= ~ECHOCTL;
    	options.c_lflag &= ~ECHOKE;
 
    	options.c_oflag &= ~OPOST;
    	options.c_oflag &= ~ONLCR;
 
    	options.c_cflag &= ~HUPCL;
  
    	options.c_cflag |= CLOCAL | CREAD;
    	options.c_cc[VTIME] = 0;
    	options.c_cc[VMIN]  = 1;

	/* Set input parity option */
/*	if (parity != 'n')*/
	tcflush(fd,TCIFLUSH);
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("SetupSerial 3\n");
		return (-1);
	}
	return (0);
}

int OpenDev(const char *Dev)
{
	int fd = open( Dev, O_RDWR | O_NOCTTY);
	//| O_NOCTTY | O_NDELAY
	if (-1 == fd)
	{
		printf("Can't Open Serial Port: %s\n", Dev);
		return -1;
	}
	else
		return fd;
}

void closeDev(int fd)
{
	close(fd);
}

void DumpMsg(unsigned char *buffer, int len)
{	
	char tmp[8];
	char hexmsg[50];
	char text[17];
	int i,index;

	for(i =0; i< len; i++)
	{
		sprintf(tmp,"%02x", buffer[i]);
		
		index = i%16;		
		if( buffer[i] >= 0x20 && buffer[i] < 0x7f)
			text[index] = buffer[i];
		else 
			text[index] = '.';

		index = index * 3;
		hexmsg[index] = tmp[0];
		hexmsg[index + 1] = tmp[1];
		hexmsg[index + 2] = ' ';
		
		if((i+1) % 16 == 0) 
		{
			text[16] = '\0';
			hexmsg[48] = '\0';
			printf( "%s %s\n", hexmsg, text);	
		}
	}

	if( i%16 != 0)
	{	
		text[i%16] = '\0';

		for(;i%16 != 0; i++)
		{
			index = (i%16) * 3;		
			hexmsg[index] = ' ';
			hexmsg[index + 1] = ' ';
			hexmsg[index + 2] = ' ';
		}
			
		hexmsg[48] = '\0';
		printf("%s %s\n", hexmsg, text);	
	}

}

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
 				retLen = read(fd485, (void *)(recv_buff + receivedLen), MAX_MSG_LEN - receivedLen);
				receivedLen += retLen;

				if(receivedLen >= MAX_MSG_LEN)
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
	char buffer[MAX_MSG_LEN];
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

void recvCheck()
{
	unsigned char buffer[MAX_MSG_LEN];
	int len, i;
	static unsigned char recv_seq;
	struct timeval now;
	unsigned int last, total_count, err_count, index;
	double total_bytes, err_bytes;

	gettimeofday(&now, NULL);
	last = now.tv_sec;
	total_count = 0;	
	err_count = 0;
	total_bytes = 0;
	err_bytes = 0;
	index = 0;
	
	while(1)
	{
 		len = read(fd485, (void *)buffer, 32);
		if(len <=0) {
			printf("Receive none\n");
			continue;
		}
		total_count += len;
		for(i=0; i<len; i++)
		{
			if(buffer[i] != recv_seq)
			{
				err_count++;
				if(print_error)
					printf("ERR! RCV:%02x, EXP:%02x, SEQ=0x%x\n", buffer[i], recv_seq, index);
			}
			recv_seq = buffer[i] + 1; 
			index++;
		}
		gettimeofday(&now, NULL);
		if(now.tv_sec - last > 60)
		{
			total_bytes += total_count;
			err_bytes += err_count;
			if(total_count != 0)
				printf("Data rate:%d bps Error:%2.4f %%, Total error:%2.6f %%\n", (total_count * 8)/(now.tv_sec - last), 100.0*err_count/total_count, 100.0*err_bytes/total_bytes);
			else
				printf("Data rate:%d bps\n", (total_count * 8)/(now.tv_sec - last));
			total_count = 0;
			err_count = 0;
			last = now.tv_sec;
		}

	}
}

void recvSim()
{
	char buffer[MAX_MSG_LEN];
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
	char buffer[MAX_MSG_LEN];
	int len;

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

void loopSim(int interval, int pause)
{
	char buffer[MAX_MSG_LEN];
	int len,count;

//	strcpy(buffer, "UUUUUUU");
//	len = strlen(buffer);
	for(len = 0; len <256; len++)
		buffer[len] = len;

	len = 256;
	pause = pause*1000;
	if(interval == 0)
		interval = 1;
	
	while(1)
	{
		if(write(fd485, (void*)buffer, len) <= 0)
			printf("Send failed \n");	
		if(pause != 0)
		{
			count++;
			if((count%interval) == 0)
				usleep(pause);
		}
//		sleep(1);
	}
}

void printUsage()
{
	printf("Usage: comtest <options> DeviceName\n");
	printf("Options: \n");
	printf("\t -i Send out input data. \n ");
	printf("\t -x Send out input data in HEX mode. \n ");
	printf("\t -s set speed. \n ");
	printf("\t -d Receive data. \n ");
	printf("\t -c Receive data and check sequence. \n ");
	printf("\t -e Echo. \n ");
	printf("\t -l Loop Send. \n ");
	printf("\t -p Loop Send pause(in ms). \n ");
	printf("\t -t Loop Send pause interval. \n ");
	printf("\t -n Not Print check error. \n ");
	printf("\t -b Bidirection. \n ");
	printf("\t -h Print this help message. \n ");
	printf("DeviceName: e.g. /dev/ttyS1. \n\n ");
}

int main(int argc, char *argv[])
{	
	extern char *optarg;
	extern int optind;
	int c;
	int speed = 115200;
	int mode = MODE_RECV;
	int hex_mod = 0;
	int loop_pause = 0;
	int loop_int = 1;

	print_error = 1;
	while ((c = getopt(argc, argv, "ixhs:dcelnp:t:b")) != EOF) {
		switch(c) {
		case 'd':
			mode=MODE_RECV;
			break;
		case 'c':
			mode=MODE_RSEQ;
			break;
		case 'x':
			hex_mod = 1;
		case 'i':
			mode=MODE_SEND;
			break;
		case 'l':
			mode=MODE_LOOP;
			break;
		case 'p':
			if(optarg != NULL)
				sscanf(optarg, "%d", &loop_pause);
			break;
		case 't':
			if(optarg != NULL)
				sscanf(optarg, "%d", &loop_int);
			break;
		case 's':
			if(optarg != NULL)
				sscanf(optarg, "%d", &speed);
			break;
		case 'e':
			mode=MODE_ECHO;
			break;
		case 'n':
			print_error = 0;
			break;
		case 'h':
			printUsage();
			exit(1);
		}
	}
  
	if (optind >= argc) {
		perror("No device name!\n");    
		printUsage();
		exit(1);
	}

	/*initialize the 485 communication port */
	fd485 = OpenDev(argv[optind]);
	if (fd485 > 0) {
		set_speed(fd485, speed);
		printf("Open serial port:%s at speed:%d\n", argv[optind], speed);
	} else  {
		printf("Can't Open Serial Port:%s!\n", argv[optind]);
		exit(1);
	}
	
	if (set_Parity(fd485,8,1, 0)== -1)
	{
		printf("Set Serial Port Parity Error\n");
		exit(1);
	}

	switch(mode) {
		case MODE_RSEQ:
			recvCheck();
			break;
		case MODE_SEND:
			consolSim(hex_mod);
			break;
		case MODE_RECV:
			recvSim();
			break;
		case MODE_LOOP:
			loopSim(loop_int, loop_pause);
			break;
		case MODE_ECHO:
			echoSim();
			break;
	}
	
        return 0;
}


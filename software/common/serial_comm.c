#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <netinet/in.h>
#include <sys/time.h>

#include "const.h"
#include "msg.h"
#include "serial_comm.h"
#include "logger.h"

int baud_def[] = { B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200, B300 };
int baud_val[] = {  115200,  57600,  38400,  19200,  9600,  4800,  2400,  1200,  300 };
int baud_cnt = sizeof(baud_def)/sizeof(int);

extern int  dump485; 


/**
*@brief 设置串口通信速率
*@param fd 类型 int 打开串口的文件句柄
*@param speed 类型 int 串口速度
*@return void
*/
void set_speed(int fd, int speed){
	int i;
	int status;
	struct termios Opt;

	tcgetattr(fd, &Opt);
	printf("the UART's baudrate is %d\n",speed);
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
		logger(LOG_ERR, "Can't Open Serial Port: %s\n", Dev);
		return -1;
	}
	else
		return fd;
}

void closeDev(int fd)
{
	close(fd);
}


/* receive message from 485 port.
	return 0 when timeout, return -1 when fail, return the total message size if success */ 
int msg485_receive(int fdCom, const char * recv_buff,  int timeout)
{
	MSG_HEAD_485 *msgheader;
	int  retLen, receivedLen;
	fd_set inputfds;
	struct timeval tv;
	int retval;
	int extra_wait;

	/* wait for the response */
	FD_ZERO(&inputfds);
	FD_SET(fdCom, &inputfds);
		   
	/* Wait the peer response up to TIMEOUT_485MSG ms. */
	tv.tv_sec = 0;
	tv.tv_usec = timeout;
	receivedLen = 0;
	extra_wait = 0;

	while(1)
	{
		retval = select(fdCom + 1, &inputfds, NULL, NULL, &tv);
		if (retval == -1)
		{
     		return -1;
		}
		else if (retval)
		{
			if(FD_ISSET(fdCom, &inputfds))
 			{
 				retLen = read(fdCom, (void *)(recv_buff + receivedLen), MAX_485MSG_LEN - receivedLen);
				if(retLen < 0)
				{ 
					logger(LOG_ERR, "Read 485 error!\n");
					return(-1);
				}
				receivedLen += retLen;

				if(receivedLen < sizeof(MSG_HEAD_485))
					continue;
				else
				{
					msgheader = (MSG_HEAD_485 *) recv_buff;
					if(msgheader->flag != COM_MSG_FLAG)
					{
						logger(LOG_ERR, "Received 485 message not started with msg flag. header flg:%d!\n", msgheader->flag);
						DumpMsg((unsigned char *)recv_buff, receivedLen); 
						return -1;
					}

					msgheader->length = ntohs(msgheader->length);
					if(msgheader->length > MAX_485MSG_LEN)
					{
						logger(LOG_ERR, "Wrong message length:%d!\n", msgheader->length);
						DumpMsg((unsigned char *)recv_buff, receivedLen); 
						return -1;
					}
						
					if(receivedLen >= (msgheader->length + sizeof(MSG_HEAD_485)))
					{
						if(dump485)
						{
							struct timeval checkpoint;

							gettimeofday(&checkpoint, NULL);
							logger(LOG_DEBUG, "%d-Received Msg:%d bytes\n", checkpoint.tv_usec/1000, receivedLen);
							DumpMsg((unsigned char *)recv_buff, receivedLen); 
						}

						return(receivedLen);
					}
					else 
						continue;
				}
				
			}
			
		} else {
			/* receive time out */			
			if(receivedLen !=0  && extra_wait == 0)
			{
				/* wait for extra 5ms  if incomplete data received*/
				logger(LOG_WARNING, "Incomplete 485 message. len:%d! Wait for another 5 ms.\n", receivedLen);
				tv.tv_usec = 5000;
				extra_wait = 1;
				FD_ZERO(&inputfds);
				FD_SET(fdCom, &inputfds);
				continue;
			} else {
				if(receivedLen  !=0 )
				{
					logger(LOG_WARNING, "Received incomplete 485 message. len:%d!\n", receivedLen);
					DumpMsg((unsigned char *)recv_buff, receivedLen); 
				}
				return 0;
			}
		}
	}

}

/* receive one message from 485 port.
	return 0 when timeout, return -1 when fail, return the total message size if success */ 
int msg485_receive_1msg(int fdCom, char * recv_buff,  int timeout)
{
	MSG_HEAD_485 *msgheader;
	int  retLen, receivedLen, expectLen;
	fd_set inputfds;
	struct timeval tv;
	int retval;
	int extra_wait;

	/* wait for the response */
	FD_ZERO(&inputfds);
	FD_SET(fdCom, &inputfds);
		   
	/* Wait the peer response up to TIMEOUT_485MSG ms. */
	tv.tv_sec = 0;
	tv.tv_usec = timeout;
	receivedLen = 0;
	extra_wait = 0;
	expectLen =  sizeof(MSG_HEAD_485);
	
	while(1)
	{
		retval = select(fdCom +1, &inputfds, NULL, NULL, &tv);
		if (retval == -1) {
             		return -1;
		} else if (retval) {
			if(FD_ISSET(fdCom, &inputfds))
 			{
//				logger(LOG_DEBUG, "receivedLen:%d, expectLen=%d\n", receivedLen, expectLen);
 				retLen = read(fdCom, (void *)(recv_buff + receivedLen),  expectLen);
				if(retLen < 0)
				{ 
					logger(LOG_ERR, "Read 485 error!\n");
					return(-1);
				}
				
				receivedLen += retLen;

				if(receivedLen < sizeof(MSG_HEAD_485))
				{
					/* wait for the complete message header */
					expectLen = sizeof(MSG_HEAD_485) - receivedLen;
					continue;					
				} else if(receivedLen == sizeof(MSG_HEAD_485)) {
					/* got the message header */
					msgheader = (MSG_HEAD_485 *) recv_buff;
					if(msgheader->flag != COM_MSG_FLAG)
					{
						int i, j;
						char * pbuff;

						logger(LOG_WARNING, "Received 485 message not started with msg flag!\n");
						DumpMsg((unsigned char *)recv_buff, receivedLen); 
						
						/* search the message start flag*/
						for(i=0; i< receivedLen; i++)
						{
							if(recv_buff[i] == COM_MSG_FLAG)
								break;
						}
						
						if(i == receivedLen)
						{
							logger(LOG_ERR, "Message Error\n");
							return(-1);
						}
						
						receivedLen -= i;
						expectLen  = sizeof(MSG_HEAD_485) - receivedLen;
						pbuff = (char *)recv_buff;
						for(j=0; j<receivedLen; j++)
							pbuff[j] = pbuff[i+j];
						
						if(dump485)
							logger(LOG_DEBUG, "Re-search Message header, rlen=%d, expect=%d\n", receivedLen, expectLen);
						continue;
					}

					msgheader->length = ntohs(msgheader->length);
					if(msgheader->length > MAX_485MSG_LEN)
					{
						logger(LOG_ERR, "Wrong message length:%d!\n", msgheader->length);
						DumpMsg((unsigned char *)recv_buff, receivedLen); 
						return -1;
					}
					
					extra_wait = 0;
				}

				/* check if there is message body */
				if(receivedLen >= (msgheader->length + sizeof(MSG_HEAD_485)))
				{
					if(dump485)
					{
						struct timeval checkpoint;
						gettimeofday(&checkpoint, NULL);
						logger(LOG_DEBUG, "%d-Received Msg:%d bytes\n", checkpoint.tv_usec/1000, receivedLen);
						DumpMsg( (unsigned char *)recv_buff, receivedLen); 
					}
					return(receivedLen);
				}
				else 
				{
					expectLen = msgheader->length - (receivedLen -sizeof(MSG_HEAD_485));
					continue;
				}
				
			}
			
		} else {
			/* receive time out */

			
			if(receivedLen !=0  && extra_wait == 0)
			{
				/* wait for extra 5ms  if incomplete data received*/
				logger(LOG_WARNING, "Incomplete 485 message. len:%d! Wait for another 5 ms.\n", receivedLen);
				tv.tv_usec = 5000;
				extra_wait = 1;
				FD_ZERO(&inputfds);
				FD_SET(fdCom, &inputfds);
				continue;
			} else {
				if(receivedLen  !=0 )
				{
					logger(LOG_WARNING, "Received incomplete 485 message. len:%d!\n", receivedLen);
					DumpMsg((unsigned char *)recv_buff, receivedLen); 
				}
				
				return 0;
			}
		}
	}

}
int msg485_send(int fdCom, const char * send_buff, int length)
{
	MSG_HEAD_485 *msgheader;
	int  retLen, sentLen;
	fd_set outpufds;
	struct timeval tv;
	int retval;

	msgheader = (MSG_HEAD_485 *) send_buff;
	msgheader->flag = COM_MSG_FLAG;
	msgheader->length = htons(msgheader->length);

	if(length > MAX_485MSG_LEN)
	{
		logger(LOG_ERR, "Sending Msg length:%d exceeds the max length:%d, sending quit.\n", length, MAX_485MSG_LEN);
		return(-1);
	}

	/* wait for the response */
	FD_ZERO(&outpufds);
	FD_SET(fdCom, &outpufds);
		   
	/* Wait the peer response up to TIMEOUT_485MSG ms. */
	tv.tv_sec = 0;
	tv.tv_usec = TIMEOUT_485MSG_SND;
	sentLen = 0;

	while(1)
	{
		retval = select(fdCom +1, NULL, &outpufds, NULL, &tv);
		if (retval == -1) {
             		return -1;
		} else if (retval) {
			if(FD_ISSET(fdCom, &outpufds))
 			{
				/* sent out the message */
				retLen= write(fdCom, (void*)(send_buff + sentLen), length - sentLen);
				if(retLen <= 0)
				{
					logger(LOG_ERR, "Sending Msg failed!");
					return(-1);
				}

				sentLen += retLen;
				if(sentLen == length)
				{
					if(dump485)
					{
						struct timeval checkpoint;

						gettimeofday(&checkpoint, NULL);
						logger(LOG_DEBUG, "%d-Send Msg:%d bytes\n", checkpoint.tv_usec/1000, sentLen);
						DumpMsg( (unsigned char *)send_buff, sentLen); 
					}
					return(sentLen);
				}
				else 
					continue;
			}			
		} else {
			/* send message time out */
			logger(LOG_ERR, "Send message timeout! sent length:%d!\n", sentLen);
			return(-1);			
		}
	}

}


/* send 485 message and wait for the response synchronization*/
/* this function is only called by the main control card */
int msg485_sendwaitrsp(int fdCom, const char * send_buff, const char * recv_buff , int timeout )
{
	MSG_HEAD_485 *msgheader;
	int  length;
	int retval;
	int i;

	msgheader = (MSG_HEAD_485 *) send_buff;
	msgheader->flag = COM_MSG_FLAG;
	msgheader->src = CTRL_CARDNO;
	length = msgheader->length + sizeof(MSG_HEAD_485);

	if(length > MAX_485MSG_LEN)
	{
		logger(LOG_ERR, "Sending Msg length:%d exceeds the max length:%d, sending quit.\n", length, MAX_485MSG_LEN);
		return(-1);
	}
	
    printf("<-------send query info, len=%d:\n", length);
    for (i = 0; i < length; i++)
        printf("%02X ", (unsigned char)send_buff[i]);
    printf("\n------------------------------------------------------->\n");

	/*clear garbage*/
	retval = msg485_receive( fdCom,  recv_buff, 1000 );
	if (  retval  > 0 )
	{
		logger(LOG_DEBUG, "receviced garbage message. retval is %d\n ", retval );
	}

	/* sent out the message */
	if(msg485_send(fdCom, send_buff, length) == -1)
	{
		logger(LOG_ERR, "Send message failed\n");
		return(-1);
	}

	/* wait for the response */
	retval = msg485_receive(fdCom, recv_buff,  timeout);
	if(retval == 0)
	{
		if(dump485)
			logger(LOG_DEBUG, "Receive message timeout\n");
		return(-1);
	} else if(retval < 0){
		logger(LOG_ERR, "Receive message failed\n");
		return(-1);
	}
	
	msgheader = (MSG_HEAD_485 *)recv_buff;
	if(msgheader->dst != 	CTRL_CARDNO)
	{
		logger(LOG_WARNING, "Received 485 message with wrong destination address:%d!\n", msgheader->dst);
		return(-1);
	}

	return 0;

}



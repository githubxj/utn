#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <netinet/in.h>

#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <asm/ioctl.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/logger.h"

const unsigned char cardno_map[] = {8,6,9,5,10,4,11,3,12,2,13,1,0,0,0,0};
const char master_cpu[] = "/sys/module/fcap100_tw2837/parameters/master_cpu";

typedef struct {
	u_int  group;	
	u_int  ctrl_pin;	
	u_int  data_out;	
	u_int  data_in;	
	
	u_int  int_clear;		
	u_int  data_direct;			
	u_int  int_enable;			
	u_int  int_trigger;
	u_int  int_both;			
	u_int  int_riseneg;			
} gpio_params;

/* Use 'g' as magic number */
#define IOC_MAGIC  'g'
#define	RCVMSG_LOOP_PERIOD		40000 /*40 ms*/
#define GPIO_READ_DATA_INPUT 	_IOWR(IOC_MAGIC, 9, gpio_params)

extern int dump485;
extern int log_level;

const char dev485[] = "/dev/ttyS2";
const char gpio_dev[] = "/dev/gpio";

int gpio_fd;
int            fd485;
int 		 ch_sock_fd=-1; 		/*socket to channel process*/
int 		 cb_sock_fd=-1; 		/*socket to cpu bridge process*/
unsigned char card_no ;

int getCardNo()
{
	int value = 0;
	int gpio_idx = 0xFFFFFFFF;
	gpio_params 	parm;
	int ret;

	/*initialize the 485 communication port */
	gpio_fd = open(gpio_dev, O_RDWR);
	if (gpio_fd  < 0) {
		printf("Can't Open file:%s!\n", gpio_fd);
		exit(1);
	}

	parm.group = 0;
	if(gpio_idx == 0xFFFFFFFF)
	{
		parm.ctrl_pin = gpio_idx;
		parm.data_out = value;
		parm.data_direct = value;
	} else {
		parm.ctrl_pin = 1 << gpio_idx;
		parm.data_out =  value << gpio_idx;
		parm.data_direct = value << gpio_idx;
	}

	parm.ctrl_pin = 0x27000;
	ret=ioctl(gpio_fd, GPIO_READ_DATA_INPUT, &parm);
	if(ret < 0)
		printf("READ GPIO value failed\n");
	else
	{
		printf("Datain:%08x\n", parm.data_in);
		value =0;
		if(parm.data_in & 0x1000)
			value |= 0x01;
		if(parm.data_in & 0x2000)
			value |= 0x04;
		if(parm.data_in & 0x4000)
			value |= 0x02;
		if(parm.data_in & 0x20000)
			value |= 0x08;
	}
			
	printf("%04x\n", value);

	if(value > 15)
		return(-1); /* abnormal data */
		
	card_no = cardno_map[value];

	return 0;
}

unsigned int getSysUart ()
{
	char send_socket[MAX_CMDBUFF_SIZE];
	char recv_socket [MAX_CMDBUFF_SIZE];
	char ret_buff[4];
	unsigned  int baudrate;
	unsigned char channelno = 0;

	memset( recv_socket, 0, MAX_CMDBUFF_SIZE);
	
	sprintf(send_socket,"GetSerial 2 \n\r" );
	if ( encode_socket( send_socket, recv_socket, channelno) != 0 )
	{
		logger (LOG_ERR  , "calling socket err\n");
		return -1;
	}
	else
	{
		logger (LOG_DEBUG, "the recv_socket is %s\n", recv_socket);
	}
	sscanf(recv_socket, "%3s", ret_buff);
	logger (LOG_DEBUG, "the ret_buff's context is %s\n", ret_buff);
	if ( !strcmp(ret_buff, "Ret") )
	{
		printf("here\n here\n here\n here\n here\n");
		sscanf (recv_socket, "%*[^=]=%u[^ ] " ,&baudrate);
		printf ("the baudrate is %d \n", baudrate);
		return (baudrate);
	}
	else {
		logger (LOG_ERR  , "the socket response message err!\n");
		return -1;
	}
}

unsigned short int is_first_cpu(void)
{
	int master_flag;
	FILE *master_fd;
	char buffer[256];
	
	master_fd = fopen(master_cpu, "r");
	if(master_fd == NULL)
	{
		perror("Open master_cpu file fail!\n");
		exit(-1);
	}

	master_flag = 0;
	if(fgets(buffer, sizeof(buffer), master_fd) != EOF)
	{
		sscanf(buffer, "%d", &master_flag);
	}

	printf("Master or Slave Cpu:%d\n", master_flag);
	return !master_flag;
	
}

int main ()
{
	char recv_buffer[MAX_485MSG_FULLLEN];/*receive message from 485 serial port*/
	MSG_HEAD_485 *msgheader;
	int recv_len;
	int counter = 0;
	
	dump485 = 1;
//	log_level=7;
	if(1 == is_first_cpu())
		exit(1);
	logger_reload();
	GetOSVersion();
	if (getCardNo())
	{
		logger (LOG_ERR  , "cannot get card number,Exiting ......\n");
		exit(1);
	}

	/*initialize the 485 communication port*/
	fd485 = OpenDev(dev485);
	if ( fd485 > 0 )
		//set_speed( fd485, 115200);
		set_speed( fd485, getSysUart());
	else{

		printf ("cannot open the 485 serial port!\n");
		exit (1);
	}

	if ( set_Parity (fd485, 8, 1, 0) == -1 )
	{
		printf ("set 485 parity err!\n");
		exit(1);
	}

	/*socket_fd =  init_socket();
	if ( -1 ==   socket_fd)
	{
		printf (" init socket  err \n");
		exit (1);
	}*/
		

/* start infinitive loop to receive message and process message. */
while (1)
{
	
	
	recv_len = msg485_receive_1msg(fd485, recv_buffer , RCVMSG_LOOP_PERIOD);


	
	if ( recv_len <= 0)
		continue;
	msgheader = (MSG_HEAD_485 *)recv_buffer;
	logger (LOG_DEBUG, "Debug : received message for card %d , receive length recvlen=%d, msgheader's length len=%d\n", msgheader->dst, recv_len, msgheader->length);
	if(recv_len > (msgheader->length + sizeof(MSG_HEAD_485)))
		logger (LOG_WARNING  , "Warning: dectect 2nd message or message garbage!\n");
	if (msgheader->dst == card_no )
		ProcessMsg( recv_buffer, recv_len, fd485);
	logger_reload();
}

close (fd485);
if(-1 != ch_sock_fd)
	close (ch_sock_fd);
if(-1 != cb_sock_fd)
	close (cb_sock_fd);
return 0;
}

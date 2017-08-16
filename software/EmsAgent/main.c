#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <sqlite3.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/serial_comm.h"
#include "../common/logger.h"
#include "message_proc.h"
#include "alarm_proc.h"
#include "card_mgmt.h"
#include "refresh_check.h"
#include "back_mgmt.h"
#include "proc_svc_switch.h"

typedef union semun
{
  int val;
  struct semid_ds *st;
  ushort * array;
}semun_t;


const char cgi_pipes[] = "/tmp/cgi_pipes"; /*cmd receive pipe*/
const char cgi_pipec[] = "/tmp/cgi_pipec"; /* cmd response pipe */
const char snmp_pipes[] = "/tmp/snmp_pipes";
const char snmp_pipec[] = "/tmp/snmp_pipec";

const char dev485[] = "/dev/ttyS1";
int				 fd485;
extern char CardName[MAX_DISPLAYNAME_LEN];

const char uart485[] = "/mnt/utn/etc/serial.ini";


/*port statistic*/
PORT_PERF *perf_data;


#define IBMGPIO_IOCTL_BASE	'Z'

struct ibm_gpio_ioctl_data {
	__u32 device;
	__u32 mask;
	__u32 data;
};

#define GPIO_MINOR             185
#define IBMGPIO_IN		_IOWR(IBMGPIO_IOCTL_BASE, 0, struct ibm_gpio_ioctl_data)
#define IBMGPIO_OUT		_IOW (IBMGPIO_IOCTL_BASE, 1, struct ibm_gpio_ioctl_data)
#define IBMGPIO_OPEN_DRAIN	_IOW (IBMGPIO_IOCTL_BASE, 2, struct ibm_gpio_ioctl_data)
#define IBMGPIO_TRISTATE	_IOW (IBMGPIO_IOCTL_BASE, 3, struct ibm_gpio_ioctl_data)
#define IBMGPIO_CFG		_IOW (IBMGPIO_IOCTL_BASE, 4, struct ibm_gpio_ioctl_data)

#define RUN_LED		0x02

int ppc_gpio_out(unsigned int mask, unsigned int data)
{
	int ret;
	int gpio_fd;

	struct ibm_gpio_ioctl_data gpio_data;	

	gpio_fd = open("/dev/gpio", O_RDWR);
	if(gpio_fd < 0) {
		return(-1);
	}

	gpio_data.device = 0;
	gpio_data.mask = mask;
	gpio_data.data = data;
	if(ret = ioctl(gpio_fd, IBMGPIO_OUT, &gpio_data) < 0){
        	perror("/dev/gpio");
	}

	close(gpio_fd);
	
	return ret;
}


int OpenPipe(const char * pipename)
{
	int fd;
	int retval;
	
	fd = open(pipename, O_RDWR);
       if(-1 == fd)
	{
		printf("Can't open pipe: %s, try to create it.\n",  pipename);
		retval = mkfifo(pipename, 0666);
        	if (retval == -1) {
 			printf("Create pipe:%s  failed.\n", pipename);
			return -1;
	 	}

		fd = open(pipename, O_RDWR);
     		if(-1 == fd)
		{
			 printf("Open pipe:%s  failed.\n", pipename);
			 return -1;
     		}
      }

	return fd;
	
}

int getBaudrate()
{
	
	FILE *fp;
	char buff[15];
	char value[10];
	int baudrate;
	fp = fopen(uart485,"r");
	if(fp == NULL)
		exit(1);
	fgets(buff,15,fp);
	sscanf(buff,"%*[^=]=%[^ ] ",value);
	baudrate = atoi(value);
	fclose(fp);
	return (baudrate);
}

#if 0 // 201708
int OpenDataTramsmitPort()
{
    int fd = -1;
    return fd;
}
#endif

int main()
{	
	int pipefd1, pipefd2, pipefd1_c, pipefd2_c, svcfd, maxfd;
#if 1	// 20150625 处理tcp socket上来的消息
	int msgfd = -1;
#endif

#if 0 // 201708
    int datafd = -1;
#endif

	fd_set inputfds;
	struct timeval tv;
	int retval;
	char buffer[MAX_CMDBUFF_SIZE];
	int data_size, led_flag;
	unsigned long loop_count = 0;
 	int semid, semcount, sem_deadlock=0;
 	union semun op;

	logger_reload();
	GetOSVersion();
	
	/*initialize the 485 communication port */
	fd485 = OpenDev(dev485);
	if (fd485 > 0)
		//set_speed(fd485, 115200);
		set_speed(fd485, getBaudrate());
	else
	{
		logger (LOG_ERR ,  "Can't Open 485 Serial Port!\n");
		exit(1);
	}
	
	if (set_Parity(fd485,8,1,'N')== -1)
	{
		logger (LOG_ERR,  "Set 485 Serial Port Parity Error\n");
		exit(1);
	}
	
	cardmgmt_init();

	/* open the communication pipes */

	pipefd1 = OpenPipe(cgi_pipes);
	maxfd = pipefd1;
	pipefd2 = OpenPipe(snmp_pipes);
	if(pipefd2 > maxfd)
		maxfd = pipefd2;	
	pipefd1_c = OpenPipe(cgi_pipec);
	pipefd2_c = OpenPipe(snmp_pipec);

	svcfd = OpenSvcPort();
	if(svcfd > maxfd)
		maxfd = svcfd;
	
#if 1 // 20150625 增加udp方式的消息处理
	msgfd = OpenMsgSvcPort();
#endif

#if 0 // 201708
    datafd = OpenDataTramsmitPort();
#endif

	if((pipefd1 == -1) || (pipefd2 == -1) ||(pipefd1_c == -1) || (pipefd2_c == -1) 
	    || svcfd == -1 || msgfd == -1 	    )
		exit(1);


	/* create the pipe access semaphore */
	semid = semget((key_t)SEM_PIPE_KEY, 1, 0666|IPC_CREAT);
	if(semid==-1)
	{
		logger( LOG_ERR , "Creating semaphore error, erron=%d.\n", errno);
		exit(1);
	}
	op.val = 1;
	semctl(semid, 0, SETVAL, op);
 
	/*init the control card information*/
	if (0 != CTRL_Config_init())
		logger  ( LOG_ERR , "init the control card err.\n");
	
	/*allocate perf_data memory */
	perf_data = (PORT_PERF *)calloc (sizeof (PORT_PERF), MAX_CARD_NUM_UTN*MAX_ONU100_NUM*MAX_PORT_NUM );
	if (NULL == perf_data )
	{
			logger (LOG_ERR ,"allocate perf_data memory err!\n");
	}
	else {
		memset (perf_data, 0 ,sizeof (PORT_PERF)*MAX_CARD_NUM_UTN*MAX_ONU100_NUM*MAX_PORT_NUM);			
	}
	
	initShelfConfig();  

 	/* start the infinitive loop, to receive message and process message, check alarms*/
	while(1) {
		loop_count++;
		
		FD_ZERO(&inputfds);
		FD_SET(pipefd1, &inputfds);
		FD_SET(pipefd2, &inputfds);
		
		if(svcfd>-1)
			FD_SET(svcfd, &inputfds);
			
#if 1 // 20150625
        if(msgfd>-1)
            FD_SET(msgfd, &inputfds);
#endif

#if 0 // 201708
        if (datafd > -1)
            FD_SET(msgfd, &inputfds);
#endif
		   
		/* Wait up to 100 ms */
		tv.tv_sec = 0;
		tv.tv_usec = TIMEOUT_CMDMSG_LOOP;
		
		retval = select(maxfd +1, &inputfds, NULL, NULL, &tv);

		if (retval == -1)
               	logger  ( LOG_ERR , "select() err"); //perror("select()");
		else if (retval) 
       	{
       		sem_deadlock=0;
       		if((svcfd>0) && (FD_ISSET(svcfd, &inputfds)))
       		{
    			ProcessSvcSwitchMsg(svcfd);
      		} 

    		if(FD_ISSET(pipefd1, &inputfds))
       		{
       			data_size = read(pipefd1, buffer, sizeof(buffer));
    			if(data_size >= sizeof(MSG_HEAD_CMD)){
    				ProcessMsg(buffer, data_size, pipefd1_c);
    			} else {
    				logger (LOG_WARNING , "Received incomplete message from CGI pipe, length=%3d\n", data_size);
    			}
      		}

			if (FD_ISSET(pipefd2, &inputfds)) {
           			data_size = read(pipefd2, buffer, sizeof(buffer));
				if(data_size >= sizeof(MSG_HEAD_CMD)){
					ProcessMsg(buffer, data_size, pipefd2_c);
				} else {
					logger (LOG_WARNING,  "Received incomplete message from SNMP pipe, length=%3d\n", data_size);
				}
			}
			
#if 1 // 20150625
            if (FD_ISSET(msgfd, &inputfds)) 
            {
                data_size = read(msgfd, buffer, sizeof(buffer));
                if(data_size >= sizeof(MSG_HEAD_CMD))
                {
                    ProcessMsg(buffer, data_size, msgfd);
                }
                else 
                {
                    logger (LOG_WARNING,  "Received incomplete message from SNMP pipe, length=%3d\n", data_size);
                }
            }
#endif

		}
		
#if 1 // 201708 // 检查是否有数据需要上传
		if(loop_count % 3 == 0)
		{
            check_data_transmit(svcfd);
        }
#endif
        
		if(loop_count % 5 == 0)
		{
			/* check every 500 ms*/
			logger_reload();

			/*check pipe semaphore deadlock*/
			semcount = semctl(semid,0,GETVAL);

			if(semcount == 0)
				sem_deadlock++;
			
			if((semcount > 1) || (sem_deadlock > 2))
			{
				op.val = 1;
				semctl(semid, 0, SETVAL, op);
			}
		
			check_cardUP();
			check_cardAlarms();
			

			/* RUN LED */
			if(led_flag)
			{
				led_flag = 0;
				ppc_gpio_out(RUN_LED, RUN_LED);
			} else {
				led_flag = 1;
				ppc_gpio_out(RUN_LED, 0);
			}

		}

		/*check back  every 8 loops (about 800ms)*/
		if((loop_count & 0x07) ==0)
			check_backstate();

		switch(loop_count & 0x7F)
		{
			case 0:
				sendLCDPower();
				break;
			case 35:
				sendLCDNetwork();
				break;
			case 70:
				sendLCDFan();
				break;
				
		}
			
		if ( loop_count % 1800 == 0 )
		{
			clearRefreshClient();
		}
    }

    return 0;
}

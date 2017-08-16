#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <errno.h>


#include "../common/const.h"
#include "../common/msg.h"
#include "../common/serial_comm.h"
#include "../common/logger.h"
#include "../common/alarm_type.h"

#include <sqlite3.h>
#include "message_proc.h"
#include "fsalib.h"
#include "fsa_alarm.h"

#define	RCVMSG_LOOP_PERIOD		40000 /*40 ms*/
#define 	CARDNO_RV_FILE		"/var/card_no_reverse"

#define WATCHDOG_IOCTL_BASE     'W'
#define	WDIOC_KEEPALIVE		_IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define	WDIOC_SETTIMEOUT        _IOWR(WATCHDOG_IOCTL_BASE, 6, int)

typedef union semun
{
  int val;
  struct semid_ds *st;
  ushort * array;
}semun_t;

#ifdef DAT_CARD
const unsigned char cardno_map[] =     {8,9,10,11,12,13,0,0,6,5,4,3,2,1,0,0};
const unsigned char cardno_map_rv[] = {8,9,10,11,12,13,0,0,6,5,4,3,2,1,0,0};;
#else
const unsigned char cardno_map[] =     {8,6,9,5,10,4,11,3,12,2,13,1,0,0,0,0};
const unsigned char cardno_map_rv[] = {8,12,10,0,9,13,11,0,6,2,4,0,5,1,3,0};
#endif

const char cgi_pipes[] = "/tmp/cgi_pipes"; /*cmd receive pipe*/
const char cgi_pipec[] = "/tmp/cgi_pipec"; /* cmd response pipe */
const char 	dev485[] = "/dev/ttyS2"; /*FSA card 485 port is conected to ttyS2*/
int			 fd485;
unsigned char	 card_no;
CARD_SET_FSA FSAConfig;

/* port statistic data  2 Dim array perfData[MAX_ONU_NUM]*[4] */
extern PORT_PERF		*perfData;

extern int dump485;
extern char CardName[MAX_DISPLAYNAME_LEN];
extern unsigned char 	adminState;

extern PORT_COUNT statisticcount;

const char wdt_dev[] = "/dev/wdt";
int wdt_fd;

int datapipe_init();

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


unsigned char *macTrans(const char *src)
{

	char c;
	int i;
	unsigned char temp, temp2;
	unsigned char mac[6];
	if ((src == NULL) && (strlen(src) < 12) )
	{
		logger  (LOG_ERR,  "input arg err\n");
		return NULL;

	}

	for (i = 0; i < 6; i++)
	{
		temp = 0;
		temp2 = 0;
		c = *src;
		if (c >= 'A' && c <= 'F')
			temp = (c - 'A') + 10;
		else if (c >= 'a' && c <= 'f')
	 		temp = ( c - 'a' ) + 10;
		else
			temp = (c - '0');
		src++;

		c = *src;
		if ( c >= 'A' && c<= 'F')
			temp2 = (c - 'A') +10;
		else if (c >= 'a' && c <= 'f')
	 		temp2 = ( c - 'a' ) + 10;
		else 
			temp2 = ( c -'0');

		temp = temp*16;
		temp += temp2;
		src++;
		*(mac+i) = temp;


	}

	return mac;


}

int Fsa_Config_init()
{
	sqlite3 *db;
	char *zErrMsg;
	int nrow,ncol, rc,fldIndex, i;
	char ** result;
	char sql_str[256];
	int portindex;
	char rawmac[12];
	unsigned char *pmacresult;
	
	rc = sqlite3_open(CONF_DB, &db);
	if (rc )
	{
		logger (LOG_ERR,  "can not open fsa config database.%s\n",sqlite3_errmsg(db) );
		return ERROR;
	}

	// init the port clock delay configuration
	sprintf(sql_str, "select portMode from FSACardPort where CardNo=0 and PortNo=0");
	rc = sqlite3_get_table(db, sql_str,	 &result, &nrow, &ncol, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR ,  "db select fsa config cardname err:%s\n", zErrMsg);
		sqlite3_close(db);
		return ERROR;

	}
	
	if ( nrow > 0)
	{
		fldIndex = ncol;
		if ( NULL != result[fldIndex])
		{
			int set_val = atoi(result[fldIndex]);
			if((set_val&0x80) && (set_val&0x70))
			{
				if(setPortClockDelay(set_val) != 0 )
					logger (LOG_ERR ,  "SetPortClockDelay failed!\n");
			}
			else
				logger (LOG_DEBUG,  "PortClockDelay not configured!\n");
		}
	}	
	sqlite3_free_table(result);

	//int card name
	sprintf(sql_str, "select CardName ,adminState from FSACard where CardNo=%d",card_no );
	rc = sqlite3_get_table(db,
					sql_str,
					&result,
					&nrow,
					&ncol,
					&zErrMsg
					);
	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR ,  "db select fsa config cardname err:%s\n", zErrMsg);
		sqlite3_close(db);
		return ERROR;
	}

	if ( nrow < 1)
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
		return 0;
	}

	fldIndex = ncol;
	if ( NULL == result[fldIndex] )
	{
		strncpy(&FSAConfig.card_name, "fsa12500" ,MAX_DISPLAYNAME_LEN);
	}
	else
	{
		strncpy(&FSAConfig.card_name, result[fldIndex] ,MAX_DISPLAYNAME_LEN);
	}

	if ( NULL == result[fldIndex+1] )
	{
		FSAConfig.admin_state = 1;
	}
	else
	{
		FSAConfig.admin_state = atoi(result[fldIndex+1]);

	}
	
	if( setFSAAdmin(0,FSAConfig.admin_state ) == 0)
	{
		logger (LOG_DEBUG , "set fsa admin successful!\n");
		adminState = FSAConfig.admin_state;
		memcpy(CardName,&FSAConfig.card_name ,MAX_DISPLAYNAME_LEN);
	}
	sqlite3_free_table(result);


	sprintf(sql_str, "select PortNo ,adminState, portMode, bwSend ,bwRecv,boundMac from FSACardPort where CardNo=%d",card_no );
	rc = sqlite3_get_table(db,
					sql_str,
					&result,
					&nrow,
					&ncol,
					&zErrMsg
					);
	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR,  "db select fsaport config  err:%s\n", zErrMsg);
		sqlite3_close(db);
		return ERROR;

	}

	if ( nrow < 1)
	{
		sqlite3_free_table(result);
		sqlite3_close(db);
		return 0;
	}

	for (i=1; i<=nrow; i++)
	{
		fldIndex = ncol;
		if ( NULL != result[fldIndex] )
		{
			portindex = atoi(result[fldIndex]);
		}

		if ( NULL != result[fldIndex+1] )
		{
			FSAConfig.port[portindex].admin_state = atoi(result[fldIndex+1]);
		}
		else
		{
			FSAConfig.port[portindex].admin_state = 1;
		}

		if ( NULL != result[fldIndex+2] )
		{
			FSAConfig.port[portindex].portMode = atoi(result[fldIndex+2]);
		}
		else
		{
			FSAConfig.port[portindex].portMode = 1;
		}

		if ( NULL != result[fldIndex+3] )
		{
			FSAConfig.port[portindex].bw_alloc_send = atoi(result[fldIndex+3]);
		}
		else
		{
			FSAConfig.port[portindex].bw_alloc_send = 102400;
		}

		if ( NULL != result[fldIndex+4] )
		{
			FSAConfig.port[portindex].bw_alloc_recv = atoi(result[fldIndex+4]);
		}
		else
		{
			FSAConfig.port[portindex].bw_alloc_recv = 102400;
		}

		if ( NULL != result[fldIndex+5] )
		{
			memcpy(rawmac ,result[fldIndex+5], 12);
			logger  (LOG_DEBUG , "rawmac is %s\n",rawmac );
			pmacresult = macTrans(rawmac);
			logger (LOG_DEBUG,  "after changed,mac is %02X%02X%02X%02X%02X%02X\n", *pmacresult,*(pmacresult+1), *(pmacresult+2),*(pmacresult+3),*(pmacresult+4),*(pmacresult+5) );
		}
		else
		{
			memcpy(rawmac ,"000000000000", 12);
		}
		
		
		FSAConfig.port[portindex].bounded_mac[0] = *pmacresult;
		FSAConfig.port[portindex].bounded_mac[1] = *(pmacresult + 1);
		FSAConfig.port[portindex].bounded_mac[2] = *(pmacresult + 2);
		FSAConfig.port[portindex].bounded_mac[3] = *(pmacresult + 3);
		FSAConfig.port[portindex].bounded_mac[4] = *(pmacresult + 4);
		FSAConfig.port[portindex].bounded_mac[5] = *(pmacresult + 5);
		if( setFSAAdmin(portindex,FSAConfig.port[portindex].admin_state) == 0)
			logger ( LOG_DEBUG,   "set fsaport %d admin successful!\n", portindex);

	}
	
	sqlite3_free_table(result);
	sqlite3_close(db);
	return 0;
	
}

int get_card_no()
{
	unsigned char data;
	char read_value;
	FILE *fp; 

	data = 0;

	//get pin PB23
	if ((fp = fopen("/sys/class/gpio/gpio87/value", "rb+")) == NULL)
	{
		logger(LOG_ERR, "Cannot open gpio87  file.\n");
		return(-1);
	}	

	fread(&read_value, 1, 1, fp);
	if(read_value == '1')
		data = data |0x01;	
	fclose(fp);

	//get pin PB24
	if ((fp = fopen("/sys/class/gpio/gpio88/value", "rb+")) == NULL)
	{
		logger(LOG_ERR, "Cannot open gpio88  file.\n");
		return(-1);
	}	

	fread(&read_value, 1, 1, fp);
	if(read_value == '1')
		data = data |0x02;
	fclose(fp);

	//get pin PB25
	if ((fp = fopen("/sys/class/gpio/gpio89/value", "rb+")) == NULL)
	{
		logger(LOG_ERR, "Cannot open gpio89  file.\n");
		return(-1);
	}	

	fread(&read_value, 1, 1, fp);
	if(read_value == '1')
		data = data |0x04;
	fclose(fp);

	//get pin PB26
	if ((fp = fopen("/sys/class/gpio/gpio90/value", "rb+")) == NULL)
	{
		logger(LOG_ERR, "Cannot open gpio90  file.\n");
		return(-1);
	}	

	fread(&read_value, 1, 1, fp);
	if(read_value == '1')
		data = data |0x08;
	fclose(fp);
			
	if(data > 15)
		return(-1); /* abnormal data */
	
	struct stat conf_stat;

	/* check card number gpio reverse */
	if(stat(CARDNO_RV_FILE, &conf_stat) !=0)
		card_no = cardno_map[data];
	else
		card_no = cardno_map_rv[data];

	logger(LOG_DEBUG, "CardNo:%d\n", card_no);
	
	return 0;
}


void run_led(unsigned char value)
{
	FILE *fp; 
	char write_value;

	//get pin PB21
	if ((fp = fopen("/sys/class/gpio/gpio85/value", "rb+")) == NULL)
	{
		return;
	}	

	if(value)
		write_value = '1';
	else
		write_value = '0';
	fwrite(&write_value, 1, 1, fp);
	fclose(fp);
	
	return;
}

int main(int argc, char *argv[])
{	
	char buffer[MAX_CMDBUFF_SIZE];
	MSG_HEAD_485 *msgheader;
	int ret, recv_len;
	unsigned long loop_count = 0;
	unsigned char run_led_flag=0;
 //	struct sched_param param;

	int pipefd1, pipefd1_c;
	fd_set inputfds;
	struct timeval tv;
	int retval;
	int data_size;
 	int semid, semcount, sem_deadlock=0;
 	union semun op;
	unsigned int wd_timeout;

	int c;
	int standalone=0;

	/* set main thread priority */
	//param.sched_priority = SCHED_PRIORITY_NORMAL;
	//pthread_setschedparam(pthread_self(), SCHED_RR, &param);

	/*get run options */
	while ((c = getopt(argc, argv, "s")) != EOF) {
		switch(c) {
		case 's':
			standalone=1;
			break;
		}
	}
	
	logger_reload();

	GetOSVersion();
	
	if(get_card_no())
	{	
		logger(LOG_ERR, "Cann't get Card number. Exiting...\n");
		exit(1);
	}

	printf("Fsadameon start, cardno=%d\n", card_no);
	
	/*initialize the 485 communication port */
	fd485 = OpenDev(dev485);
	if (fd485 > 0)
		set_speed(fd485, 115200);
	else
	{
		logger(LOG_ERR, "Can't Open 485 Serial Port!\n");
		exit(1);
	}
	
	if (set_Parity(fd485,8,1,'N')== -1)
	{
		logger(LOG_ERR, "Set 485 Serial Port Parity Error\n");
		exit(1);
	}

	wd_timeout = 5;
	wdt_fd = open(wdt_dev, O_RDWR);
	if (wdt_fd  < 0)
		printf("Can't Open file:%s!\n", wdt_dev);
	else
		ioctl(wdt_fd, WDIOC_KEEPALIVE, &wd_timeout);

	memset (&statisticcount, 0 , sizeof(statisticcount));//clear.
	statisticcount.countType = 1;
	/* allocate port perf data memory */
	perfData = (PORT_PERF *)malloc(sizeof(PORT_PERF)*MAX_ONU_NUM*4);
	if (perfData == NULL)
	{
		logger(LOG_ERR, "Allocate port perf data memory error!\n");
		exit(1);
	} else
		memset(perfData, 0, sizeof(PORT_PERF)*MAX_ONU_NUM*4);

	logger(LOG_DEBUG, "Allocate perfData, size = %x, addr=%x!\n", sizeof(PORT_PERF)*MAX_ONU_NUM*4, perfData);

	/* initialize the fpga api */
	if(fsalib_init() == -1)
		exit(1);
	
	if(datapipe_init() == -1)
		logger (LOG_ERR,  "Init Data pipe error, data passthrough is not available!\n");

	if(standalone)
		logger(LOG_NOTICE, "FSA Daemon started in standalone mode!\n");
	else
		logger(LOG_NOTICE, "FSA Daemon started!\n");

	/*init fsa config data from configdatabase*/
	if (Fsa_Config_init() != 0)
		logger (LOG_ERR,  "init fsaconfig data err!\n");

	if(standalone)
	{
		/* open the communication pipes */
		pipefd1 = OpenPipe(cgi_pipes);
		pipefd1_c = OpenPipe(cgi_pipec);
		if((pipefd1 == -1) || (pipefd1_c == -1))
		{
			logger( LOG_ERR , "Open pipe error.\n");
			exit(1);
		}

		/* create the pipe access semaphore */
		semid = semget((key_t)SEM_PIPE_KEY, 1, 0666|IPC_CREAT);
		if(semid==-1)
		{
			logger( LOG_ERR , "Creating semaphore error, erron=%d.\n", errno);
			exit(1);
		}
		op.val = 1;
		semctl(semid, 0, SETVAL, op);
	}
	
 	/* start the infinitive loop, to receive message and process message*/
	while(1) {

		if(wdt_fd > 0)
		{
			ret=ioctl(wdt_fd, WDIOC_KEEPALIVE, &wd_timeout);
			if(ret < 0)
				printf("Watch dog feed failed\n");
		}
		
		loop_count++;
		if((loop_count % 120) == 0) /*reload the logger config every 120 RCVMSG_LOOP_PERIOD counts */
			logger_reload();

		/*run led*/
		if((loop_count % 12) == 0) 
		{
			if(run_led_flag)
				run_led_flag = 0;
			else
				run_led_flag = 1;
			run_led(run_led_flag);
		}


		if(standalone)
		{
			/*process message from WEB */
			if ( loop_count % 500 == 0 )
			{
				clearRefreshClient();
			}
			
			FD_ZERO(&inputfds);
			FD_SET(pipefd1, &inputfds);
			   
			/* Wait up to TIMEOUT_CMDMSG_LOOP ms */
			tv.tv_sec = 0;
			tv.tv_usec = RCVMSG_LOOP_PERIOD;
			
			retval = select(pipefd1 +1, &inputfds, NULL, NULL, &tv);

			if (retval == -1)
	               	logger  ( LOG_ERR , "select() err"); //perror("select()");
			else if (retval) 
	           	{
	           		sem_deadlock=0;
	           		if(FD_ISSET(pipefd1, &inputfds))
	           		{
	           			data_size = read(pipefd1, buffer, sizeof(buffer));
					if(data_size >= sizeof(MSG_HEAD_CMD)){
						ProcessMsg(buffer, data_size, pipefd1_c, standalone);
					} else {
						logger (LOG_WARNING , "Received incomplete message from CGI pipe, length=%3d\n", data_size);
					}
				} else {
					logger (LOG_WARNING , "no input fds match!\n");
				}
				
			} else  {
			
				check_fsaAlarms();

				/*check pipe semaphore deadlock*/
				semcount = semctl(semid,0,GETVAL);

				if(semcount == 0)
					sem_deadlock++;
				
				if((semcount >1) || (sem_deadlock >2))
				{
					op.val = 1;
					semctl(semid, 0, SETVAL, op);
				}

			}

		}else {
			/* process message from 485 bus */
			recv_len = msg485_receive_1msg(fd485, buffer, RCVMSG_LOOP_PERIOD);
			if(recv_len <=0)
				continue;
			
			msgheader = (MSG_HEAD_485 *)buffer;
			if(dump485)
				logger(LOG_DEBUG, "Received message for card:%d, len=%d, recv_len=%d\n", msgheader->dst, msgheader->length, recv_len);

			if(recv_len > (msgheader->length + sizeof(MSG_HEAD_485))) 
				logger(LOG_WARNING, "Dectect 2nd message or message garbage!\n");
			

			if(msgheader->dst == card_no)
				ProcessMsg(buffer, recv_len, fd485, standalone);
		}
		
        }

	close(fd485);
	fsalib_cleanup();
        return 0;
}

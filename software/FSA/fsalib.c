#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <asm/ioctl.h>

#include <pthread.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/logger.h"
#include "../common/alarm_type.h"
#include "fsalib.h"
#include "db_interface.h"

//#define FPGA_SIMULATOR

#define MAX_ONULIST_ERROR 100

//ip1826
#define IP18_READ_REG	_IOW('A', 0x01, unsigned int) 
#define IP18_WRITE_REG	_IOW('A', 0x02, unsigned int) 
#define    IP18_NAME		"/dev/ip1826"

struct ip18_rw_struct {
	unsigned int reg;
	unsigned int value;
};

#define FPGA_GET_SYS_MAP	_IOW('Z', 0x01, unsigned int) 
#define FPGA_SET_SYS_MAP	_IOW('Z', 0x02, unsigned int) 
#define FPGA_FIBER0_OUN	_IOW('Z', 0x03, unsigned int)
#define FPGA_FIBER1_OUN	_IOW('Z', 0x04, unsigned int)
#define FPGA_GET_FIFO_LEN	_IOW('Z', 0x05, unsigned int)
#define FPGA_SET_FIFO_LEN	_IOW('Z', 0x06, unsigned int)
#define FPGA_FIFO2_SET		_IOW('Z', 0x0B, unsigned int)
#define FPGA_FIFO2_DATA		_IOW('Z', 0x0C, unsigned int)
#define FPGA_FIBER0_STATE	_IOW('Z', 0x0E, unsigned int)
#define FPGA_FIBER1_STATE	_IOW('Z', 0x0F, unsigned int)
#define FPGA_BUFF_LEN		_IOW('Z', 0x10, unsigned int)
#define FPGA_REG_READ		_IOW('Z', 0x11, unsigned int)
#define FPGA_INT_EN			_IOW('Z', 0x20, unsigned int)

#define FPGA_NAME		"/dev/fpga"
#define MASK_VALID_ONU	0xFFFFFF		/*make sure to process 0~23bit */
#define EMPTY_ONUMAP	0xFFFFFF	

#define MAX_CMDRSP_QUEUE_LEN	10
#define MAX_ALARM_QUEUE_LEN	100
#define MAX_IOREP_QUEUE_LEN	100

#define FIFO_MSG_LEN		80
#define FIFO_BUF_MAX_LEN	800
#define FIFO_MSGVALID_LEN	72
#define FIFO_MSG_TIMEOUT	100		/*100ms*/
#define MAX_FIFO2_LEN		144

#define MAX_DATAP_LEN		64


/*define bitmap of type in FIFO message header */ 
#define TYPE_MASK_WRITE		0x80
#define TYPE_MASK_DIR			0x40
#define TYPE_MASK_DEV			0x38
#define TYPE_MASK_CMD			0x07

#define TYPE_DEV_FPGA			0x00
#define TYPE_DEV_KS8995		0x10
#define TYPE_DEV_FIBER1			0x20
#define TYPE_DEV_FIBER2			0x30
#define TYPE_DEV_DATAP			0x08

#define CMD_FPGA_GETDSN		0x00
#define CMD_FPGA_ONUNAME		0x01
#define CMD_FPGA_ADMIN		0x02
#define CMD_FPGA_RESET			0x03
#define CMD_FPGA_UPDATESN		0x04
#define CMD_FPGA_SETID			0x05
#define CMD_FPGA_DATA			0x06
#define CMD_FPGA_UP			0x07

#define CMD_NOTIFY				0x07
#define CMD_DATA_PASSTHROUGH	0x00
#define CMD_DATA_PORT2			0x01
#define CMD_DATA_IO			0x03
#define CMD_DATA_IO_QUERY		0x04
#define CMD_DATA_PORT3			0x05
#define CMD_DATA_PORT4			0x06
#define CMD_DATA_PORT5			0x07

#define CMD_FIBER_STATE		0x01

typedef struct FIFO_MSG_HEAD {
	unsigned char 	flag[4];
	unsigned char  	dst;
	unsigned char  	src;
	unsigned char 	sn;	
	unsigned char 	type;	
}__attribute__ ((__packed__)) FIFO_MSG_HEAD;

typedef struct RSP_QUEUE {
	unsigned char  	used;
	unsigned char  	respond;
	unsigned char 	node;	
	unsigned char 	type;	
} RSP_QUEUE;


typedef struct fifo2_data_struct {
	unsigned short 	len;
	char data[MAX_FIFO2_LEN];
}FIFI2_DATA;

const unsigned char fifoMsgStartFlag[] = {0x55, 0x55, 0x55, 0x55};
RSP_QUEUE		rsp_queue[MAX_CMDRSP_QUEUE_LEN];
char 			rsp_queue_data[MAX_CMDRSP_QUEUE_LEN][FIFO_MSG_LEN];

/* ***************************************************************/
/*	alarm queue for ctrl card polling 								    	*/
/*   alarm_wait_flag is used to bloack alarm queue when system is processing*/
/*				   node list and node topo							*/
/*	reduce_flag is used to reduce alarms for the ONU node updating purpose */ 
/****************************************************************/
ONET_ALARM_INFO alarm_queue[MAX_ALARM_QUEUE_LEN];
int		alarm_queue_head, alarm_queue_tail;
int 		alarm_wait_flag, reduce_flag;

ONU_IO_REPORT ioreport_queue[MAX_IOREP_QUEUE_LEN];
int		ioreport_queue_head, ioreport_queue_tail;

ONU_LIST_ENTRY	ONUStatusList[MAX_ONU_NUM + 1];
unsigned long 		ONUNameUpdatedFlag;
unsigned long		ONUDsn[MAX_ONU_NUM + 1];
unsigned char		ONUDsnFull[MAX_ONU_NUM + 1][MAX_ONUDSN_LEN];
int 				DsnUpdateDBFlag=0;

unsigned char  	OnuPortState[MAX_ONU_NUM + 1][4];

int 		ringState;   /* 1: ring ok, 0: ring broken*/
int		fdFpga;
extern int		dumpFifo;
extern int 	debugFpga;
extern int 	rstcplog;


pthread_attr_t attrOnulist, attrFifo, attrIO;
pthread_t thidOnulist, thidFifo, thidIO;

static pthread_mutex_t mtx_dev;          /*fdFpga device lock*/
static pthread_mutex_t mtx_onulist;      /* ONUStatusList lock*/
static pthread_mutex_t mtx_rspqueue;      /* response queue lock*/
static pthread_mutex_t mtx_alarmqueue;      /* alarm queue lock*/
static pthread_mutex_t mtx_ioqueue;      /* io report queue lock*/

/*simulator data */
ETHER_PORT_SET simOnuPort[MAX_ONU_NUM + 1][4];
unsigned char simAdminState[MAX_ONU_NUM + 1];
ETHER_PORT_SET simFSAPort[2];

/* port statistic data  2 Dim array perfData[MAX_ONU_NUM]*[4] */
PORT_PERF		*perfData;
PORT_COUNT 		 statisticcount;

ONU_INFO onuInfo[MAX_ONU_NUM];
ONU_UART_CONF onuUartInfo[MAX_ONU_NUM + 1][6];

//static unsigned int count=0;

extern void downloadONUCfg(unsigned char node, unsigned long onu_dsn);
extern int updateONUPortCfgInDB(unsigned long onu_dsn ,unsigned char portindex, ETHER_PORT_SET *etherport_set);

extern void* alarm2Net();

//ip1826
int ip18_write_reg(struct ip18_rw_struct *rw)
{
	int ret;
	int fd;
	
	fd = open(IP18_NAME, O_RDWR);
    	if(fd < 0) {
        logger(LOG_ERR, " open %s error\n", IP18_NAME);
		return -1;
   	 } 
	ret = ioctl(fd, IP18_WRITE_REG, rw);	
	if(  ret < 0)
	{
		close(fd);
		logger(LOG_ERR, " ioctl write  error\n");
		return ret;
	}
	
	close(fd);
	return ret;
}


int ip18_read_reg(struct ip18_rw_struct *rw)
{
	int ret;
	int fd;
	
	fd = open(IP18_NAME, O_RDWR);
  	 if(fd < 0) {
     	 	 logger(LOG_ERR, " open %s error\n", IP18_NAME);
		return -1;
  	  } 
	 ret = ioctl(fd, IP18_READ_REG, rw );
	if( ret  < 0)
	{
		close(fd);
		logger( LOG_ERR," ioctl read  error\n");
		return ret;

	}
	
	close(fd);
	return ret;
}

int  send_fifomsg(unsigned char dst, unsigned char type, unsigned char *data, int len)
{
	int ret;
	unsigned char buffer[FIFO_MSG_LEN];
	FIFO_MSG_HEAD *header;
	header = (FIFO_MSG_HEAD*)buffer;
	
#ifdef FPGA_SIMULATOR
	return -1;
#endif

	memset(buffer, 0, FIFO_MSG_LEN);
	memcpy(header, fifoMsgStartFlag, 4);
	header->dst = dst;
	header->type = type;
	header->src = 0; /*message is from olt*/

	if(len > FIFO_MSG_LEN - sizeof(FIFO_MSG_HEAD)){
		logger(LOG_WARNING, "FIFO lenth exceeds limitation, message is trancated.\n");
		len = FIFO_MSG_LEN - sizeof(FIFO_MSG_HEAD);
	}
	
	if(len>0 && data != NULL)
		memcpy(buffer + sizeof(FIFO_MSG_HEAD), data, len);
	
	pthread_mutex_lock(&mtx_dev);
	ret = write(fdFpga, buffer, FIFO_MSG_LEN); 
       pthread_mutex_unlock(&mtx_dev);
	if(ret < 0) {
		logger(LOG_ERR, "Write FIFO error\n");
		return -1;
	}
	
	if(dumpFifo) {
		struct timeval checkpoint;

		gettimeofday(&checkpoint, NULL);
		logger(LOG_DEBUG, "%d-FIFO: send message to ONU, cmd=0x%0x, dst=%d\n", checkpoint.tv_usec/1000, type, dst);
		if(dumpFifo >1)
			DumpMsg(buffer, FIFO_MSG_LEN);
	}

/*
// fifo debug 
{
	unsigned int reg, len1, len2;
	
	count++;
	reg=0x10;
	ioctl(fdFpga, FPGA_REG_READ, &reg);
	len1=reg&0xff;
	reg=0x11;
	ioctl(fdFpga, FPGA_REG_READ, &reg);
	len1 |= (reg&0xff)<<8;
	reg=0x12;
	ioctl(fdFpga, FPGA_REG_READ, &reg);
	len2=reg&0xff;
	reg=0x13;
	ioctl(fdFpga, FPGA_REG_READ, &reg);
	len2 |= (reg&0xff)<<8;
	logger(LOG_ERR, "FIFO: LEN1=0x%08x, LEN1=0x%08x, SEND DIFF=0x%08x\n", len1, len2, len1-count);
	
}
*/

	return 0;	
}

int  send_fifomsg_waitrsp(unsigned char dst, unsigned char type, unsigned char *data, int send_len, unsigned char * recv_data, int timeout)
{
	int resp_index, ret, retry;

	if(dst == 0xFF) {
		logger(LOG_WARNING, "FIFO:Broadcast message do not support wait response!\n");
		return -1;
	}

#ifdef FPGA_SIMULATOR
	return -1;
#endif

	//find a entry in response queue
       pthread_mutex_lock(&mtx_rspqueue);
	for(resp_index=0; resp_index<MAX_CMDRSP_QUEUE_LEN; resp_index++)
	{
		if(rsp_queue[resp_index].used)
			continue;
			
		rsp_queue[resp_index].used = 1;
		rsp_queue[resp_index].respond = 0;
		rsp_queue[resp_index].node = dst;
		rsp_queue[resp_index].type = type & ~TYPE_MASK_DIR;
		break;
	}
       pthread_mutex_unlock(&mtx_rspqueue);
	if( resp_index==MAX_CMDRSP_QUEUE_LEN) {
		logger(LOG_ERR, "FIFO:Message response queue is FULL!\n");
		return -1;
	}
	
	retry = 0;
	while(retry < 3)
	{
		ret = send_fifomsg(dst, type, data, send_len);
		if( ret == 0)
		{
			int waittime=0;
			
			while(!rsp_queue[resp_index].respond &&  waittime < timeout)
			{
				usleep(20000);
				waittime +=20;
			}

			if(!rsp_queue[resp_index].respond) {			
				logger(LOG_WARNING, "FIFO: wait response timeout!Retry:%d\n", retry);
				ret = -1;
				retry++;
			}
			else {				
				memcpy(recv_data, rsp_queue_data[resp_index] + 8, FIFO_MSG_LEN -8);
				break;
			}
		}
	}

	if(ret != 0)
	{
		logger(LOG_ERR, "FIFO: Message no response!Retry:%d\n", retry);
	}
	
	//clear the entry in response queue
       pthread_mutex_lock(&mtx_rspqueue);
	rsp_queue[resp_index].used = 0;
       pthread_mutex_unlock(&mtx_rspqueue);

	return(ret);	
	
}


int gen_fsa_alarm(unsigned char alarmtype, unsigned char node, unsigned char port, unsigned char reason)
{
	pthread_mutex_lock(&mtx_alarmqueue);

	if(reduce_flag && 
		((alarmtype == ATYPE_ONUNAMECHG) || (alarmtype == ATYPE_ONUSNCHG) ))
	{
		pthread_mutex_unlock(&mtx_alarmqueue);
		return 0;
	}

	logger(LOG_DEBUG, "alarm type:%d\n", alarmtype);
	
	alarm_queue[alarm_queue_head].alarm_type = alarmtype;
	alarm_queue[alarm_queue_head].node_id= node;
	alarm_queue[alarm_queue_head].port_no = port;
	alarm_queue[alarm_queue_head].reason = reason;

	alarm_queue_head++;
	
	if(alarm_queue_head >= MAX_ALARM_QUEUE_LEN)
		alarm_queue_head = 0;
	
	if(alarm_queue_head == alarm_queue_tail) 
	{
		logger(LOG_WARNING, "FPGA:alarm queue overflow, the oldest alarm is flushed out.\n");
		alarm_queue_tail++;
		if(alarm_queue_tail >= MAX_ALARM_QUEUE_LEN)
			alarm_queue_tail = 0;
	}

	if(reduce_flag == 0)
	{
		switch(alarmtype) {
			case ATYPE_ONUONLINE:
			case ATYPE_ONUOFFLINE:
			case ATYPE_ONETRINGFAILURE:
			case ATYPE_ONETRINGRESTORE:
			case ATYPE_ONETRINGBROKEN:
			case ATYPE_ONETLINKBROKEN:
			case ATYPE_ONETLINKRESTORE:
			case ATYPE_ONUNAMECHG:
					reduce_flag = 1;
					break;
			default :
					break;
		}
	}
	pthread_mutex_unlock(&mtx_alarmqueue);
	return 0;
}

void  processONUNodes(int sysmap)
{
	unsigned long bitmask;
	unsigned char onu_index;
	int 	msg_count=1;

	bitmask = 0x01;

       pthread_mutex_lock(&mtx_onulist);
	for(onu_index=1; onu_index<=MAX_ONU_NUM; onu_index++, bitmask <<=1)
	{
		if(bitmask & sysmap)
		{
			/* onu not exist*/
			ONUDsn[onu_index] = 0;
			if(ONUStatusList[onu_index].node_id == 0)
				continue;

			if(ONUStatusList[onu_index].node_state == ONU_STATE_OFFLINE)
				continue;
			
			ONUStatusList[onu_index].node_state = ONU_STATE_OFFLINE;

			//generate onu offline alarm; 
			deleteDSN(onu_index);
			gen_fsa_alarm(ATYPE_ONUOFFLINE, onu_index, 0, 0);
			
		} else {
			/* onu exist */
			if((ONUStatusList[onu_index].node_id != 0) 
				&& (( ONUStatusList[onu_index].node_state == ONU_STATE_ONLINEE) ||( ONUStatusList[onu_index].node_state == ONU_STATE_ONLINED)))
				continue;

			ONUStatusList[onu_index].node_id = onu_index;
			ONUStatusList[onu_index].node_sn = onu_index;
			ONUStatusList[onu_index].node_state = ONU_STATE_ONLINEE;

			// clear onu name
			memset(ONUStatusList[onu_index].onu_name, 0, MAX_DISPLAYNAME_LEN);
			ONUNameUpdatedFlag &= ~(1<<onu_index);		//clear update flag;	
			//update onu name 
			send_fifomsg(onu_index, CMD_FPGA_ONUNAME, NULL,0);
			msg_count++;
			
			// slow down update message sending speed
			if((msg_count%8) == 0)
				usleep(10000); 
			
			//update dsn 
			send_fifomsg(onu_index, CMD_FPGA_GETDSN, NULL,0);
			msg_count++;

			// slow down update message sending speed
			if((msg_count%8) == 0)
				usleep(10000); 
		
			//generate onu online alarm; 			
			gen_fsa_alarm(ATYPE_ONUONLINE, onu_index, 0, 0);

		}


	}

	pthread_mutex_unlock(&mtx_onulist);

}

void  processONUTopo(int fiber1map, int fiber2map, int fiber1mapPrev, int fiber2mapPrev)
{
	unsigned long bitmask;
	int onu_index;
	unsigned char maxSN, minSN, nodeId_N, nodeId_F;

	//send SN update message
	usleep(200000); //modify 2014-4-14
//	usleep(50000);
	send_fifomsg(0xFF, CMD_FPGA_UPDATESN, NULL,0);
	usleep(50000);

	if((fiber1map == fiber2map) && (fiber1map != EMPTY_ONUMAP))
	{
		/* Fiber Ring connected*/
		ringState = 1;
		bitmask = 0x01;
       	pthread_mutex_lock(&mtx_onulist);
		for(onu_index=1; onu_index<=MAX_ONU_NUM; onu_index++, bitmask <<=1)
		{
			if(bitmask & fiber1map)
				continue;
			
			ONUStatusList[onu_index].network_state = ONU_NET_RING;
		}
       	pthread_mutex_unlock(&mtx_onulist);

		//generate ring ok information
		if(debugFpga)
			logger(LOG_DEBUG, "FPGA: Fiber Ring Connected!\n");

		gen_fsa_alarm(ATYPE_ONETRINGRESTORE, 0, 0, 0);
		
	} 
	else if((fiber1map | fiber2map) != EMPTY_ONUMAP)
	{
		/* Fiber Ring, but one direction is broken */
		/* Some nodes is seen on both fiber and some node is seen on one of the fiber */
		int flag_o2=0, err_node=0;
		logger(LOG_WARNING, "RING-1D: fibermap1=%x and fibermap2=%x\n", fiber1map, fiber2map);
		ringState = 1;
		
		bitmask = 0x01;
       	pthread_mutex_lock(&mtx_onulist);
		for(onu_index=1; onu_index<=MAX_ONU_NUM; onu_index++, bitmask <<=1)
		{
			if((fiber1map&fiber2map) == fiber1map)
			{
				if(bitmask & fiber1map)
					continue;
				
				if(bitmask & fiber2map)
					ONUStatusList[onu_index].network_state = ONU_NET_RING_O1D;
				else
					ONUStatusList[onu_index].network_state = ONU_NET_RING;
			}
			else
			{
				if(bitmask & fiber2map)
					continue;
				
				if(bitmask & fiber1map)
					ONUStatusList[onu_index].network_state = ONU_NET_RING_O2D;
				else
					ONUStatusList[onu_index].network_state = ONU_NET_RING;
				flag_o2 = 1;
			}
		}
       	pthread_mutex_unlock(&mtx_onulist);

		//generate ring ok information
		if(debugFpga)
			logger(LOG_DEBUG, "FPGA: Fiber Ring failure!\n");

		//find the error node
		if(flag_o2)
		{
			int maxSN = 0;
			for(onu_index=1; onu_index<=MAX_ONU_NUM; onu_index++)
			{
				if((ONUStatusList[onu_index].node_state == ONU_STATE_OFFLINE) || (ONUStatusList[onu_index].node_state == ONU_STATE_NOTUSE))
					continue;
				
				if(( ONUStatusList[onu_index].network_state == ONU_NET_RING) && (ONUStatusList[onu_index].node_sn > maxSN))
				{
					maxSN = ONUStatusList[onu_index].node_sn;
					err_node = ONUStatusList[onu_index].node_id;
				}	
				
			}
		}
		else
		{
			int minSN =0xFF;
			for(onu_index=1; onu_index<=MAX_ONU_NUM; onu_index++)
			{
				if((ONUStatusList[onu_index].node_state == ONU_STATE_OFFLINE) || (ONUStatusList[onu_index].node_state == ONU_STATE_NOTUSE))
					continue;
				
				if(( ONUStatusList[onu_index].network_state == ONU_NET_RING) && (ONUStatusList[onu_index].node_sn < minSN))
				{
					minSN = ONUStatusList[onu_index].node_sn;
					err_node = ONUStatusList[onu_index].node_id;
				}
			}
		}
		
		gen_fsa_alarm(ATYPE_ONETRINGFAILURE, err_node, flag_o2+1, 0);
		
	} else { 
		/* Fiber Ring broken or fiber link status changed */
		bitmask = 0x01;
       	pthread_mutex_lock(&mtx_onulist);
		for(onu_index=1; onu_index<=MAX_ONU_NUM; onu_index++, bitmask <<=1)
		{
			if((bitmask & fiber1map) == 0)
				ONUStatusList[onu_index].network_state = ONU_NET_NEAREND;
			else if((bitmask & fiber2map) == 0)
				ONUStatusList[onu_index].network_state = ONU_NET_FAREND;
			else 
				ONUStatusList[onu_index].network_state = ONU_NET_INVALIDE;				
		}
       	pthread_mutex_unlock(&mtx_onulist);
	
		/*find the change point, it is alway the tail of two fiber links */
		maxSN =0;
		minSN =0xFF;
		nodeId_N = 0;
		nodeId_F = 0;
		for(onu_index=1; onu_index<=MAX_ONU_NUM; onu_index++)
		{
			if(( ONUStatusList[onu_index].network_state == ONU_NET_NEAREND)
				&& (ONUStatusList[onu_index].node_sn > maxSN))
			{
				maxSN = ONUStatusList[onu_index].node_sn;
				nodeId_N = onu_index;	
			}	
			
			if(( ONUStatusList[onu_index].network_state == ONU_NET_FAREND)
				&& (ONUStatusList[onu_index].node_sn < minSN))
			{
				minSN = ONUStatusList[onu_index].node_sn;
				nodeId_F = onu_index;	
			}				
		}

		/*generate alarms */
		if(ringState == 1)
		{
			/* Fiber Ring broken */
			ringState = 0;

			//generate ring broken alarm
			if(debugFpga)
				logger(LOG_DEBUG, "FPGA: Fiber Ring broken at Node %d far end side, at Node %d near end side!", nodeId_N, nodeId_F);
			
			gen_fsa_alarm(ATYPE_ONETRINGBROKEN, nodeId_N, 0, ONU_NET_FAREND);
			gen_fsa_alarm(ATYPE_ONETRINGBROKEN, nodeId_F, 0, ONU_NET_NEAREND);
		}else {
			 /* fiber link status changed */
			 int changebits;

			changebits = fiber1map ^ fiber1mapPrev;
			if(changebits) {
				if(changebits & fiber1map)
					gen_fsa_alarm(ATYPE_ONETLINKBROKEN, nodeId_N, 0, ONU_NET_FAREND);
				else
					gen_fsa_alarm(ATYPE_ONETLINKRESTORE, nodeId_N, 0, ONU_NET_NEAREND);	

				if(debugFpga)
					logger(LOG_DEBUG, "FPGA: Link change=%0x, Fiber1map=%0x\n", changebits, fiber1map);
			}


			changebits = fiber2map ^ fiber2mapPrev;
			if(changebits) {
				if(changebits & fiber2map)
					gen_fsa_alarm(ATYPE_ONETLINKBROKEN, nodeId_F, 0, ONU_NET_NEAREND);
				else
					gen_fsa_alarm(ATYPE_ONETLINKRESTORE, nodeId_F, 0, ONU_NET_FAREND);	

				if(debugFpga)
					logger(LOG_DEBUG, "FPGA: Link change=%0x, Fiber2map=%0x\n", changebits, fiber2map);
			}
		}
			
	}
	
}

void check_dsn_and_name()
{
	int i;
	static int loop;

	loop++;
	if(loop%10 != 0)
		return;
	
	// check all the dsn refresh command responses are received, if not, get the dsn again
	pthread_mutex_lock(&mtx_onulist);
	for(i=1; i <= MAX_ONU_NUM; i++)
	{
		if((ONUDsn[i] == 0) &&
			((ONUStatusList[i].node_state == ONU_STATE_ONLINEE) || (ONUStatusList[i].node_state == ONU_STATE_ONLINED)))
		{
			send_fifomsg(i, CMD_FPGA_GETDSN, NULL,0);
			logger(LOG_WARNING, "ONU: %d has no valid device serial number!\n", i);
		}
		if(((ONUNameUpdatedFlag & (1<<i) ) == 0) &&
			((ONUStatusList[i].node_state == ONU_STATE_ONLINEE) || (ONUStatusList[i].node_state == ONU_STATE_ONLINED)))
		{
			send_fifomsg(i, CMD_FPGA_ONUNAME, NULL,0);
			logger(LOG_WARNING, "ONU: %d has no name!\n", i);
		}
	}
	pthread_mutex_unlock(&mtx_onulist);
	
	
}

/*
void process_onulist_err(int sysmap, int fibermap1, int fibermap2)
{
	static int sysmap_old, f1map_old, f2map_old, err_count;
	int error_map, onu_index, bitmask;

	if((sysmap != sysmap_old) || (fibermap1 != f1map_old)  || (fibermap2 != f2map_old))
	{
		err_count = MAX_ONULIST_ERROR;
		sysmap_old = sysmap;
		f1map_old = fibermap1;
		f2map_old = fibermap2;
		return;
	}
	
	err_count--;
	if(err_count <=0)
	{
		err_count=MAX_ONULIST_ERROR;
		error_map = fibermap1 ^ fibermap2;
		bitmask = 1;
		for(onu_index=1; onu_index<=MAX_ONU_NUM; onu_index++, bitmask <<=1)
		{
			if(error_map&bitmask)
			{
				send_fifomsg(onu_index, CMD_FPGA_RESET , NULL, 0);
				if(debugFpga)
					logger(LOG_DEBUG, "Reset ONU: %d\n", onu_index);
			}
		}
	}
	
}
*/

int  skip_onulist_err(int fibermap1, int fibermap2)
{
	static int f1map_old, f2map_old, err_count = MAX_ONULIST_ERROR;

	if( (fibermap1 != f1map_old)  || (fibermap2 != f2map_old))
	{
		err_count = MAX_ONULIST_ERROR;
		f1map_old = fibermap1;
		f2map_old = fibermap2;
		return 1;
	}
	
	err_count--;
	if(err_count <=0)
	{
		err_count=MAX_ONULIST_ERROR;
		f1map_old = fibermap1;
		f2map_old = fibermap2;
		return 0;
	}

	return 1;
	
}

void handle_onulist()
{
	int ret;
	int sysmap,fibermap1,fibermap2;
	int sysmapPrev,fibermap1Prev,fibermap2Prev;
	int policy;
	struct sched_param param;

	if(pthread_getschedparam(pthread_self(), &policy, &param)==0)
		printf("THD:onulist, policy:%d, priority:%d\n", policy, param.sched_priority);

	sysmapPrev = MASK_VALID_ONU;
	fibermap1Prev = MASK_VALID_ONU;
	fibermap2Prev = MASK_VALID_ONU;

	while(1) 
	{
		/* sleep50ms*/
		usleep(50000); 

		check_dsn_and_name();
		
		/*read the map of onu nodes */
   //     	pthread_mutex_lock(&mtx_dev);
#ifdef FPGA_SIMULATOR
		ret = lseek(fdFpga, 0, 0); 
		ret = read(fdFpga, &sysmap, 4); 
#else
		ret = ioctl(fdFpga, FPGA_GET_SYS_MAP, &sysmap);
#endif
  //     	pthread_mutex_unlock(&mtx_dev);
		if(ret < 0)
		{
			logger(LOG_ERR, "FPGA:  ioctl get sysmap error\n");
			continue;
		}
		sysmap &=MASK_VALID_ONU;
		
		/*read the map of onu nodes on Fiber1 */
  //      	pthread_mutex_lock(&mtx_dev);
#ifdef FPGA_SIMULATOR
		ret = read(fdFpga, &fibermap1, 4); 
#else
		ret = ioctl(fdFpga, FPGA_FIBER0_OUN, &fibermap1);
#endif
 //      	pthread_mutex_unlock(&mtx_dev);
		if(ret < 0)
		{
			logger(LOG_ERR, "FPGA:  ioctl get fibermap 1 error\n");
			continue;
		}
		fibermap1 &=MASK_VALID_ONU;
		
		/*read the map of onu nodes on Fiber2 */
   //     	pthread_mutex_lock(&mtx_dev);
#ifdef FPGA_SIMULATOR
		ret = read(fdFpga, &fibermap2, 4); 
#else
		ret = ioctl(fdFpga, FPGA_FIBER1_OUN, &fibermap2);
#endif
 //      	pthread_mutex_unlock(&mtx_dev);
		if(ret < 0)
		{
			logger(LOG_ERR, "FPGA: ioctl get fibermap 2 error\n");
			continue;
		}
		fibermap2 &=MASK_VALID_ONU;
		
		if(sysmap != (fibermap1&fibermap2) )
		{
			logger(LOG_ERR, "FPGA: sysmap=%x is not consistant with  fibermap1=%x and 2=%x.\n", sysmap, fibermap1, fibermap2);
			continue;
		}

		/* 2014-4-21 
		if((fibermap1 != fibermap2) && ((fibermap1 | fibermap2) !=0xFFFFFF))
		{
			logger(LOG_ERR, "FPGA: fibermap1=%x and is not consistant with fibermap2=%x, sysmap=%x.\n", fibermap1, fibermap2, sysmap);
			process_onulist_err(sysmap, fibermap1, fibermap2);
			continue;
		}
		*/
		if((fibermap1 != fibermap2) && ((fibermap1 | fibermap2) !=0xFFFFFF))
		{	
			//one direction fiber error, wait for MAX_ONULIST_ERROR loop period to ensure same error
			if(skip_onulist_err(fibermap1, fibermap2))
				continue;
		}
		
		if(sysmap != sysmapPrev)
		{
			if(debugFpga)
				logger(LOG_DEBUG, "FPGA: Sysmap change. sysmap:%x, fiber1:%x, fiber2:%x\n", sysmap, fibermap1, fibermap2);
			alarm_wait_flag = 1;
			processONUNodes(sysmap);
			sysmapPrev = sysmap;
		}

		if((fibermap1 != fibermap1Prev) || (fibermap2 != fibermap2Prev))
		{
			if(debugFpga)
				logger(LOG_DEBUG, "FPGA: O1 or O2 map change. sysmap:%x, fiber1:%x, fiber2:%x\n", sysmap, fibermap1, fibermap2);

			processONUTopo(fibermap1, fibermap2, fibermap1Prev, fibermap2Prev);
			fibermap1Prev = fibermap1;
			fibermap2Prev = fibermap2;
		}
		alarm_wait_flag	= 0;
	}

	
}


unsigned long extract_dsn2(unsigned char * data)
{
	unsigned long dsn1=0,dsn2=0;
	
	data[MAX_ONUDSN_LEN] = 0;
	sscanf(data, "%u-%u", &dsn1,&dsn2);
	if(dsn2!=0)
		return(dsn2);
	else 
		return(dsn1);
}

void  process_msg_8995(unsigned char cmd, unsigned char node, unsigned char *data)
{
	int i;
	
	if(node >MAX_ONU_NUM){
		logger(LOG_ERR, "FIFO: received FIFO message with wrong nodeid:%d\n", node);
		return;
	}
	
	switch(cmd) {
		case CMD_NOTIFY:
			for(i=0; i<4; i++)
			{
				if((OnuPortState[node][i] != 0) && (OnuPortState[node][i] != data[i]))
				{
					if(data[i] == 1)
						gen_fsa_alarm(ATYPE_ONUPORTUP, node, i+1, 0);
					else
						gen_fsa_alarm(ATYPE_ONUPORTDOWN, node, i+1, 0);
				}
				OnuPortState[node][i] = data[i];
			}
			break;
	}
}

void  process_msg_fiber(unsigned char cmd, unsigned char node, unsigned char *data, unsigned char dev)
{
	unsigned char sfpIndex;
	
	if(node >MAX_ONU_NUM){
		logger(LOG_ERR, "FIFO: received FIFO message with wrong nodeid:%d\n", node);
		return;
	}

	if(dev == TYPE_DEV_FIBER1)
		sfpIndex = 1;
	else 
		sfpIndex = 2;
	
	switch(cmd) {
		case CMD_NOTIFY:
			if(data[0] == 1)
				gen_fsa_alarm(ATYPE_ONUSFPREMOVED, node, sfpIndex, 0);
			else if(data[0] == 2)
				gen_fsa_alarm(ATYPE_ONUSFPINSERTED, node, sfpIndex, 0);
			else if(data[0] == 3)
				; //logger(LOG_ERR, "TEST: SFP NO SIGNAL:%d\n", node);
			else if(data[0] == 4)
				gen_fsa_alarm(ATYPE_ONUSFPERROR, node, sfpIndex, 0);
			break;
	}
}

void  process_msg_fpga(unsigned char cmd, unsigned char node, unsigned char *data)
{
	int i;
	unsigned long tmpVal;
	
	if((node >MAX_ONU_NUM) && (cmd != CMD_FPGA_UP)){
		logger(LOG_ERR, "FIFO: received FIFO message with wrong nodeid:%d\n", node);
		return;
	}
	
	switch(cmd) {
		case CMD_FPGA_ONUNAME:
			if(strncmp(ONUStatusList[node].onu_name, data, MAX_DISPLAYNAME_LEN) !=0)
			{
				memcpy(ONUStatusList[node].onu_name, data, MAX_DISPLAYNAME_LEN);
				/* clear the illegual display name */
				for(i=0; i<MAX_DISPLAYNAME_LEN; i++)
				{
					if((ONUStatusList[node].onu_name[i] > 0) && (ONUStatusList[node].onu_name[i] < 0x1F))
						ONUStatusList[node].onu_name[i] = '.';
					else if(ONUStatusList[node].onu_name[i] == 0xFF)
						ONUStatusList[node].onu_name[i] = 0;
				}
				gen_fsa_alarm(ATYPE_ONUNAMECHG, node, 0, 0);
			}
			ONUNameUpdatedFlag |= (1<<node); // name updated
			break;
		case CMD_FPGA_UPDATESN:
			break;
		case CMD_FPGA_DATA:
			break;
		case CMD_FPGA_GETDSN:
			tmpVal = extract_dsn2(data);
			if(tmpVal != ONUDsn[node])
			{
				ONUDsn[node] = tmpVal;
				memcpy(ONUDsnFull[node], data, MAX_ONUDSN_LEN);
				DsnUpdateDBFlag = 1;
			}
			break;
		case CMD_FPGA_UP:
			{
				unsigned char cmd_buf[MAX_ONUDSN_LEN+4];
				unsigned char assigned_id;
				unsigned long dsn;

				memcpy(cmd_buf + 1, data, MAX_ONUDSN_LEN);
				cmd_buf[MAX_ONUDSN_LEN+1] = 0; 
				
				if(debugFpga)
					logger(LOG_DEBUG, "FPGA: ONU power on. request id:%d, dsn=%s", node, cmd_buf + 1);

				dsn = extract_dsn2(data);
				if(dsn == 0) {
					logger(LOG_ERR, "FPGA: Received Invalide ONU unique DSN\n");
					return;
				}

				pthread_mutex_lock(&mtx_onulist);
				if(ONUDsn[node] == 0)
				{
					//the node id is not occupied
					ONUDsn[node] = dsn;
					assigned_id = node;
				}
				else if(ONUDsn[node] == dsn)
				{
					assigned_id = node;
				}
				else
				{
					//find an available node id
					i=1;
					while(i <= MAX_ONU_NUM)
					{
						if((ONUDsn[i] == 0) &&
							( (ONUStatusList[i].node_state == ONU_STATE_NOTUSE) || (ONUStatusList[i].node_state == ONU_STATE_OFFLINE)))
						{
							assigned_id = i;
							ONUDsn[assigned_id] = dsn;
							break;
						}
						i++;
					}
					if(i > MAX_ONU_NUM) {
						logger(LOG_ERR, "FPGA: No ONU is available!\n");
						pthread_mutex_unlock(&mtx_onulist);
						return;
					}
					
				}
				memcpy(ONUDsnFull[assigned_id], data, MAX_ONUDSN_LEN);
				pthread_mutex_unlock(&mtx_onulist);

				if(debugFpga)
					logger(LOG_DEBUG, "FPGA: Assign ONU id. request id:%d, assigned id:%d", node, assigned_id);

				cmd_buf[0] = assigned_id;
				send_fifomsg(0xFF, CMD_FPGA_SETID, cmd_buf, MAX_ONUDSN_LEN+1);

				/* get update the ONU name */
				send_fifomsg(assigned_id, CMD_FPGA_ONUNAME, NULL,0);

				/* after got the ONU dsn, download the configuration from database to ONU according to unique dsn*/
				downloadONUCfg(assigned_id, ONUDsn[assigned_id]);
			}
			break;
	}

}

void  process_msg_datap(unsigned char cmd, unsigned char node, unsigned char *data)
{
	unsigned char datalen;
	unsigned char port;
	int sendRS_flag = 0;
	
	if(node >MAX_ONU_NUM){
		logger(LOG_ERR, "FIFO: received FIFO message with wrong nodeid:%d\n", node);
		return;
	}
	
	switch(cmd) {
		case CMD_DATA_PASSTHROUGH:
			port = 0;
			sendRS_flag = 1;
			break;
		case CMD_DATA_PORT2:
			port = 1;
			sendRS_flag = 1;
			break;
		case CMD_DATA_PORT3:
			port = 2;
			sendRS_flag = 1;
			break;
		case CMD_DATA_PORT4:
			port = 3;
			sendRS_flag = 1;
			break;
		case CMD_DATA_PORT5:
			port = 4;
			sendRS_flag = 1;
			break;
		case CMD_DATA_IO:
			/* append the ONU IO change report into the queue */ 
			pthread_mutex_lock(&mtx_ioqueue);
	
			ioreport_queue[ioreport_queue_head].node_id= node;
			ioreport_queue[ioreport_queue_head].io_state = data[0];

			ioreport_queue_head++;
			if(ioreport_queue_head >= MAX_IOREP_QUEUE_LEN)
				ioreport_queue_head = 0;
			
			if(ioreport_queue_head == ioreport_queue_tail) 
			{
				logger(LOG_WARNING, "FPGA:io report queue overflow, the oldest one is flushed out.\n");
				ioreport_queue_tail++;
				if(ioreport_queue_tail >= MAX_IOREP_QUEUE_LEN)
					ioreport_queue_tail = 0;
			}
	
			pthread_mutex_unlock(&mtx_ioqueue);
			break;

	}

	if(sendRS_flag)
	{
		datalen = data[0];
		if(datalen <= MAX_DATAP_LEN)
			proc_recvRSData(node, port, data +1, datalen);
		else
			logger(LOG_ERR, "FIFO: received pass through data len%d exceed limitation.\n", datalen);
	}

}

int  handle_fifomsg()
{
	unsigned char buffer[FIFO_BUF_MAX_LEN];
	int i, res, receivedLen, response_processed, msg_offset;
	FIFO_MSG_HEAD *header;
	unsigned char device, cmd;
	unsigned long temp, err_count=0, loop=0;
	int policy;
	struct sched_param param;

	if(pthread_getschedparam(pthread_self(), &policy, &param)==0)
		printf("THD:fifmsg-%d, policy:%d, priority:%d\n", getpid(), policy, param.sched_priority);


	struct timeval checkpoint;

	header = (FIFO_MSG_HEAD*)buffer;

	msg_offset = 0;
	while(1)
	{
#ifdef FPGA_SIMULATOR
		sleep(3);
		continue;
#endif

/*************************************************************/
/*test purpose
loop++;
if((loop%100) == 0)
{

	gettimeofday(&checkpoint, NULL);
			
	ioctl(fdFpga, FPGA_BUFF_LEN, &temp);
	if(temp>0xf000)
	{
		printf("fifo overflow, exit for debug\n");
		exit(1);
	}
	
	logger(LOG_WARNING, "FIFO:%d, len=%08x\n", checkpoint.tv_usec, temp);
}
***********************************************************/
//        	pthread_mutex_lock(&mtx_dev);
		receivedLen = read(fdFpga, buffer + msg_offset, FIFO_MSG_LEN - msg_offset); 
 //       	pthread_mutex_unlock(&mtx_dev);
		if(receivedLen < 0) {
			logger(LOG_ERR, "Read FIFO error\n");
			sleep(3);
			msg_offset = 0;
			continue;
		}
		
		if(receivedLen == 0) {
			/* no message in FIFO, sleep 10ms*/
			if(DsnUpdateDBFlag == 1)
			{
				DsnUpdateDBFlag = 0;
				updateDSN(ONUDsn, ONUDsnFull);
			}
			else
				usleep(10000); 
			
			msg_offset = 0;
			continue;
		}

		receivedLen += msg_offset;
		if(receivedLen < FIFO_MSG_LEN) {
			/*incomlete message, wait for 5ms and read again*/
			usleep(5000); 
  //      		pthread_mutex_lock(&mtx_dev);
			res = read(fdFpga, buffer + receivedLen, FIFO_MSG_LEN -receivedLen); 
 //       		pthread_mutex_unlock(&mtx_dev);
			if(res < 0) {
				logger(LOG_ERR, "Read FIFO error2\n");
				msg_offset = 0;
				continue;
			}

			if( (res + receivedLen) !=  FIFO_MSG_LEN) {
				logger(LOG_WARNING, "FIFO: rcvd incomplete msg, len=%d, expect:%d\n", res+receivedLen, FIFO_MSG_LEN);
		//		DumpMsg(buffer, res + receivedLen);
				msg_offset = 0;
				continue;
			}				
		}
		
		/* received a complete message*/
		if(dumpFifo) {
			struct timeval checkpoint;

			gettimeofday(&checkpoint, NULL);
			logger(LOG_DEBUG, "%d-FIFO: received message:\n", checkpoint.tv_usec/1000);
			if(dumpFifo >1)
				DumpMsg(buffer, FIFO_MSG_LEN);
		}
		
		/* check the message */
		if(memcmp(fifoMsgStartFlag, buffer, sizeof(fifoMsgStartFlag))!=0) 
		{
			int i,j, tmp_len, head_found;
			
			logger(LOG_WARNING, "WARNING:FIFO: rcvd msg with no head flag.\n");
//			DumpMsg(buffer, FIFO_MSG_LEN);

			/*******************************************/
			/* test purpose
			err_count++;
			ioctl(fdFpga, FPGA_BUFF_LEN, &temp);
			logger(LOG_WARNING, "FIFO:ERR-buff len=%08x\n", temp);
			if(err_count>1000)
				exit(2);
			****************************************/
			
			/* seek the start of message */
			head_found = 0;
			for(i=0;i<FIFO_MSG_LEN; i++)
			{
				if(buffer[i] != fifoMsgStartFlag[0])
					continue;

				if(i<FIFO_MSG_LEN-sizeof(fifoMsgStartFlag))
					tmp_len = sizeof(fifoMsgStartFlag);
				else
					tmp_len = FIFO_MSG_LEN -i;
				
				if(memcmp(fifoMsgStartFlag, buffer+i, tmp_len)!=0)
					continue;

				/*find new message header, and copy the rest of message*/
				tmp_len = FIFO_MSG_LEN -i;
				for(j=0; j<tmp_len;j++)
					buffer[j] = buffer[i+j];
				
				msg_offset = tmp_len;
				
				head_found = 1;
				logger(LOG_WARNING, "FIFO: head re-seek found, index=%d, left:%d!\n", i, tmp_len);
				break;
			}

			if(head_found == 0) {
				logger(LOG_WARNING, "WARNING:FIFO: no Message flag found!\n");
				//ioctl(fdFpga, FPGA_BUFF_LEN, &temp);
				//logger(LOG_WARNING, "FIFO:buff len=%08x\n", temp);
				msg_offset = 0;
			}
			continue;			
		}
		msg_offset = 0;
	
		if(header->dst == 0xFF) {
			logger(LOG_WARNING, "FIFO: received broadcast message, unprocessed currently!\n");
			continue;
		}
		
		if(header->dst != 0x00) {
			if(dumpFifo)
				logger(LOG_DEBUG, "FIFO: received message for ONU, discard it!\n");
			continue;
		}

		if((header->type & TYPE_MASK_DIR) == 0) {
			if(dumpFifo)
				logger(LOG_DEBUG, "FIFO: received message from FSA -> ONU, discard it!\n");
			continue;
		}

		/*start to process the message*/		
		//update node SN
		if(header->src <= MAX_ONU_NUM)
		{
			if(ONUStatusList[header->src].node_sn != header->sn)
			{
				ONUStatusList[header->src].node_sn = header->sn;
				gen_fsa_alarm(ATYPE_ONUSNCHG, header->src, 0, header->sn);
			}
		}

		/*search the response queue first, if match found, set the flag and copy data */
		response_processed = 0;
        	pthread_mutex_lock(&mtx_rspqueue);
		for(i=0; i<MAX_CMDRSP_QUEUE_LEN; i++)
		{
			if((rsp_queue[i].used == 0) || rsp_queue[i].respond)
				continue;
			
			logger(LOG_DEBUG, "FIFO: Response queue-%d, used=%d, repond=%d, node=%d, type=%d!\n", 
				i, rsp_queue[i].used, rsp_queue[i].respond, rsp_queue[i].node,  rsp_queue[i].type);

			if((rsp_queue[i].node == header->src) && (rsp_queue[i].type == (header->type & ~TYPE_MASK_DIR)))
			{
				memcpy(rsp_queue_data[i], buffer, FIFO_MSG_LEN);
				rsp_queue[i].respond = 1;
				response_processed = 1;
				break;
			}
		}
        	pthread_mutex_unlock(&mtx_rspqueue);

		if(response_processed)
			continue;

		device = header->type & TYPE_MASK_DEV;
		cmd = header->type & TYPE_MASK_CMD;

		if(dumpFifo) 
			logger(LOG_DEBUG, "FIFO: Process message from ONU,  dev=0x%0x, cmd=0x%0x, src=%d,sn=%d\n", device, cmd, header->src, header->sn);

		switch(device) {
			case TYPE_DEV_FPGA:
				process_msg_fpga(cmd, header->src, buffer+sizeof(FIFO_MSG_HEAD));
				break;
			case TYPE_DEV_KS8995:
				process_msg_8995(cmd, header->src, buffer+sizeof(FIFO_MSG_HEAD));
				break;
			case TYPE_DEV_FIBER1:
			case TYPE_DEV_FIBER2:
				process_msg_fiber(cmd, header->src, buffer+sizeof(FIFO_MSG_HEAD), device);
				break;			
			case TYPE_DEV_DATAP:
				process_msg_datap(cmd, header->src, buffer+sizeof(FIFO_MSG_HEAD));
				break;			
		}
			
	}

	return(0);
}

int fsalib_init()
{
	int res;
	int i,j;
 	struct sched_param param;
 
	for(i=0; i<=MAX_ONU_NUM; i++)
	{
		simAdminState[i] = 1;
		for(j=0; j<4;j++)
		{
			simOnuPort[i][j].admin_state = 1;
			simOnuPort[i][j].bw_alloc_recv = 40;
			simOnuPort[i][j].bw_alloc_send = 40;
		}
	}

	for(i=0; i<2; i++)
	{
		simFSAPort[i].admin_state = 1;
		simFSAPort[i].bw_alloc_recv = 1000;
		simFSAPort[i].bw_alloc_recv = 1000;
	}

	memset(ONUStatusList, 0, sizeof(ONUStatusList));
	alarm_queue_head = 0;
	alarm_queue_tail = 0;
	ringState = 0;

	fdFpga = open(FPGA_NAME, O_RDWR);
    	if(fdFpga < 0) {
        	logger(LOG_ERR, "FPGA: open %s error\n", FPGA_NAME);
		return -1;
    	} 

	ioctl(fdFpga, FPGA_INT_EN, 0);

    	pthread_mutex_init(&mtx_dev, NULL);
    	pthread_mutex_init(&mtx_onulist, NULL);
    	pthread_mutex_init(&mtx_rspqueue, NULL);
    	pthread_mutex_init(&mtx_alarmqueue, NULL);
    	pthread_mutex_init(&mtx_ioqueue, NULL);

	/* create fpga register read thread*/
	pthread_attr_init(&attrOnulist);
	pthread_attr_setdetachstate(&attrOnulist, PTHREAD_CREATE_DETACHED);
	//pthread_attr_setschedpolicy(&attrOnulist, SCHED_RR);
	//param.sched_priority = SCHED_PRIORITY_NORMAL;
	//pthread_attr_setschedparam(&attrOnulist, &param);
	
	res = pthread_create(&thidOnulist, &attrOnulist, handle_onulist, NULL);
	if(res < 0)
	{
		logger(LOG_ERR, "FPGA: Unable to start fpga register read thread.\n");
		return -1;
	}
	
	/* create fifo message read thread*/
	pthread_attr_init(&attrFifo);
	pthread_attr_setdetachstate(&attrFifo, PTHREAD_CREATE_DETACHED);
	//pthread_attr_setschedpolicy(&attrFifo, SCHED_RR);
	//param.sched_priority = SCHED_PRIORITY_NORMAL;
	//pthread_attr_setschedparam(&attrFifo, &param);
	
	res = pthread_create(&thidFifo, &attrFifo, handle_fifomsg, NULL);
	if(res < 0)
	{
		logger(LOG_ERR, "FIFO: Unable to start fpga register read thread.\n");
		return -1;
	}

	/* create ONU io process thread*/
	pthread_attr_init(&attrIO);
	pthread_attr_setdetachstate(&attrIO, PTHREAD_CREATE_DETACHED);
	//pthread_attr_setschedpolicy(&attrIO, SCHED_RR);
	//param.sched_priority = SCHED_PRIORITY_NORMAL;
	//pthread_attr_setschedparam(&attrIO, &param);
	
	res = pthread_create(&thidIO, &attrIO, alarm2Net, NULL);
	if(res < 0)
	{
		logger(LOG_ERR, "FSA: Unable to start IO process thread.\n");
		return -1;
	}

	return 0;	
	
}

void fsalib_cleanup()
{
    	pthread_mutex_destroy(&mtx_dev);
    	pthread_mutex_destroy(&mtx_onulist);
    	pthread_mutex_destroy(&mtx_rspqueue);
    	pthread_mutex_destroy(&mtx_alarmqueue);
	pthread_attr_destroy(&attrOnulist);
	pthread_attr_destroy(&attrFifo);
	close(fdFpga);
	
}

int getFSASFPInfo(SFP_INFO * sfp_info, int index)
{
	int i, regval, regindex;
	unsigned short word_tmp;
	FIFI2_DATA fifo_data;

	/* clear the fifo before read sfp info */
	i = 0;
	while(i<10)
	{
//		pthread_mutex_lock(&mtx_dev);
		ioctl(fdFpga, FPGA_FIFO2_DATA, &fifo_data);
//		pthread_mutex_unlock(&mtx_dev);
		
		if(fifo_data.len == 0)
			break;
		i++;
	}

	if(i>9)
	{
		logger(LOG_ERR, "FPGA: Read FSA SFP info error! FIFO2 is not empty.\n");
		return -1;
	}

	if(index == 1)
		regval = 0x20;
	else
		regval = 0x30;
	
//	pthread_mutex_lock(&mtx_dev);
	ioctl(fdFpga, FPGA_FIFO2_SET, &regval);
//	pthread_mutex_unlock(&mtx_dev);

	usleep(20000);
	
//	pthread_mutex_lock(&mtx_dev);
	ioctl(fdFpga, FPGA_FIFO2_DATA, &fifo_data);
//	pthread_mutex_unlock(&mtx_dev);

	if(fifo_data.len <= 128)
	{
		logger(LOG_ERR, "FPGA: Read FSA  %d SFP  basic info error! Incomplete mesage read, len=%d.\n", index, fifo_data.len);
		return -1;
	}
	
	memset(sfp_info, 0, sizeof(SFP_INFO));
	sfp_info->sfpIndex = index;

	regindex = 11; /* skip the first 11 byte for fifo header and Reg start address */

	if(fifo_data.data[regindex] != 3)
	{
		sfp_info->sfpCompliance = 0xFF;
		return 0;
	}

	memcpy(sfp_info->sfpVendor, fifo_data.data + regindex + 20, 16);
	memcpy(sfp_info->sfpPartNumber, fifo_data.data + regindex + 40, 16);
	sfp_info->sfpConnector = fifo_data.data[regindex + 2];	
	sfp_info->sfpTransCode = fifo_data.data[regindex + 9] & 0x0F;
	
	if(sfp_info->sfpTransCode & 0x01)		
		sfp_info->sfpSmLength = fifo_data.data[regindex + 14];
	if(sfp_info->sfpTransCode & 0x40)
		sfp_info->sfpMmLength = fifo_data.data[regindex + 16];
	else if (sfp_info->sfpTransCode & 0x80)
		sfp_info->sfpMmLength = fifo_data.data[regindex + 17];
	
	sfp_info->sfpCopperLength = fifo_data.data[regindex + 18];	
	sfp_info->sfpBrSpeed = fifo_data.data[regindex + 12];	
	word_tmp = fifo_data.data[regindex + 60] *0x100 + fifo_data.data[regindex + 61];
	sfp_info->sfpWavelength = htons(word_tmp);

	sfp_info->sfpCompliance = fifo_data.data[regindex + 94];	
	if(sfp_info->sfpCompliance == 1)
	{
		/* clear data */
//		pthread_mutex_lock(&mtx_dev);
		ioctl(fdFpga, FPGA_FIFO2_DATA, &fifo_data);
//		pthread_mutex_unlock(&mtx_dev);

		/* get Diagnostic Monitoring data */
		regval += 1;
//		pthread_mutex_lock(&mtx_dev);
		ioctl(fdFpga, FPGA_FIFO2_SET, &regval);
//		pthread_mutex_unlock(&mtx_dev);

		usleep(20000);
		
//		pthread_mutex_lock(&mtx_dev);
		ioctl(fdFpga, FPGA_FIFO2_DATA, &fifo_data);
//		pthread_mutex_unlock(&mtx_dev);

		if(fifo_data.len <= 128)
		{
			logger(LOG_ERR, "FPGA: Read FSA SFP Diagnostic Monitoring data  error! Incomplete mesage read, len=%d.\n", fifo_data.len);
			return -1;
		}
		
		regindex = 11; /* skip the first 11 byte for fifo header and Reg start address */
		
		sfp_info->sfpTemperature = fifo_data.data[regindex + 96]; 
		word_tmp = fifo_data.data[regindex + 102] *0x100 + fifo_data.data[regindex + 103];
		sfp_info->sfpTranPower = htons(word_tmp);
		word_tmp = fifo_data.data[regindex + 104] *0x100 + fifo_data.data[regindex + 105];
		sfp_info->sfpRecvPower = htons(word_tmp);

	}
	return 0;
}

/************************************************************
*	getFSAState: query the FSA board states
*	param-out:  FSA_CARD_STATE *, the pointer of buffer that hold the result
*	return:  	0  - success
*			-1 - fail
************************************************************/
int getFSAState(FSA_CARD_STATE * state)
{
	int i;
	struct ip18_rw_struct regstate;
	unsigned  int temp;

	//read fiber 1 state	
//	pthread_mutex_lock(&mtx_dev);
	ioctl(fdFpga, FPGA_FIBER0_STATE, &temp);
//	pthread_mutex_unlock(&mtx_dev);

	if(temp == 0)
		state->spf_n_state  = 2; //up
	else if(temp == 6) {
		SFP_INFO sfp_info;		
		if(getFSASFPInfo(&sfp_info, 1) != 0)
			state->spf_n_state  = 1; //removed
		else
			state->spf_n_state  = 3; //down
	} else
		state->spf_n_state  = 4; //error
		
	//read fiber 2 state	
//	pthread_mutex_lock(&mtx_dev);
	ioctl(fdFpga, FPGA_FIBER1_STATE, &temp);
//	pthread_mutex_unlock(&mtx_dev);

	if(temp == 0)
		state->spf_f_state  = 2; //up
	else if(temp == 6) {
		SFP_INFO sfp_info;		
		if(getFSASFPInfo(&sfp_info, 2) != 0)
			state->spf_f_state  = 1; //removed
		else
			state->spf_f_state  = 3; //down
	} else
		state->spf_f_state  = 4; //error
		
	state->statistic_type = statisticcount.countType;

	//port states of two GE port;
	regstate.reg = 0xE6;
	if ( ip18_read_reg(&regstate) != 0 )
		return -1;
	logger(LOG_DEBUG, "reg 0xE6 's value is %d\n", regstate.value);
	
	if ( 0x01 ==  (regstate.value & 0x01))//port 24
		state->geport_state[0].port_state = 1;
	else
		state->geport_state[0].port_state = 2;

	if ( 0x40 ==  (regstate.value & 0x40 ) )//port 25
		state->geport_state[1].port_state = 1;
	else
		state->geport_state[1].port_state = 2;

	//port adminstate of two GE port.
	regstate.reg = 0xD9;//transimit state register.
	if ( ip18_read_reg(&regstate) != 0)
		return -1;
	logger(LOG_DEBUG, "reg 0xD9 's value is %d\n", regstate.value);
	temp = regstate.value;

	regstate.reg = 0x1F;//receive state regsiger.
	if ( ip18_read_reg( &regstate ) != 0 )
		return -1;
	logger(LOG_DEBUG, "reg 0x1F 's value is %d\n", regstate.value);

	if ( (0x100 == (temp & 0x100)) && (0x100 == (regstate.value & 0x100)) )
		state->geport_state[0].admin_state = 1;
	else
		state->geport_state[0].admin_state = 2;
	
	if ( (0x200 == (temp & 0x200)) && (0x200 == (regstate.value & 0x200)) )
		state->geport_state[1].admin_state = 1;
	else
		state->geport_state[1].admin_state = 2;
	logger (LOG_DEBUG, "geport1's adminstate is %d, geport2's adminstate is %d\n", state->geport_state[0].admin_state, state->geport_state[1].admin_state );

	//igmp snooping.
	regstate.reg = 0xC0;
	if ( ip18_read_reg( &regstate) != 0)
		return -1;
	logger (LOG_DEBUG, "reg 0xC0 's value is %d\n", regstate.value );

	if ( 0x01 == (regstate.value & 0x01))
		state->igmp_snooping = 1;
	else
		state->igmp_snooping = 2;

	//broadcast storm control
	regstate.reg = 0x42;
	if ( ip18_read_reg(&regstate) != 0)
		return -1;
	logger(LOG_DEBUG, "reg 0x42 's value is %d\n", regstate.value);
	temp = regstate.value;

	regstate.reg = 0x43;//receive state regsiger.
	if ( ip18_read_reg( &regstate ) != 0 )
		return -1;
	logger(LOG_DEBUG, "reg 0x43 's value is %d\n", regstate.value);

	if ( (0xFFFF == (temp & 0xFFFF)) && (0x7FF == (regstate.value & 0x7FF)) )
		state->broadcaststorm_ctrl = 1;
	else
		state->broadcaststorm_ctrl = 2;
	
	//broadcast storm threshold
	regstate.reg = 0x44;
	if ( ip18_read_reg(&regstate) != 0)
		return -1;
	logger (LOG_DEBUG, "reg 0x44 's value is %d\n", regstate.value );
	state->storm_threshold = (regstate.value & 0xFC00 ) >> 10;
	
	for (i = 0; i<2; i++)
	{
		state->geport_state[i].portIndex = i;
		state->geport_state[i].portWorkingMode = 1;
		//state->geport_state[i].admin_state = simFSAPort[i].admin_state;
		state->geport_state[i].bw_alloc_recv = simFSAPort[i].bw_alloc_recv;
		state->geport_state[i].bw_alloc_send = simFSAPort[i].bw_alloc_send;
		//state->geport_state[i].port_state = 1;
		state->geport_state[i].bounded_mac[0] = 1;
		state->geport_state[i].bounded_mac[1] = 2;
		state->geport_state[i].bounded_mac[2] = 3;
		state->geport_state[i].bounded_mac[3] = 4;
		state->geport_state[i].bounded_mac[4] = 5;
		state->geport_state[i].bounded_mac[5] = 6;
	}

	if(ringState)
		state->network_state = 1;
	else
		state->network_state = 0;
		
	return 0;
}

/***********************************************************
*	getONUList: get the list of ONU nodes that connected to the FSA
*	param-out:  ONU_LIST_ENTRY* pList , the first pointer of entry buffer that hold the result
*	return:  	-1 - fail
*			other - the number ONU nodes, max number is MAX_ONU_NUM
***********************************************************/
int getONUList(ONU_LIST_ENTRY* pList)
{
	unsigned char onu_index;
	int onu_number =0;
	
	usleep(20000); //make sure SN of all Nodes are updated

       pthread_mutex_lock(&mtx_onulist);
	for(onu_index=1; onu_index<=MAX_ONU_NUM; onu_index++)
	{
		/* onu not exist*/
		if(ONUStatusList[onu_index].node_id == 0) 
			continue;

		/* do not return the offline node */
		if(ONUStatusList[onu_index].node_state == 3)
			continue;

		memcpy(pList + onu_number, ONUStatusList + onu_index, sizeof(ONU_LIST_ENTRY));
		onu_number++;
	}
	pthread_mutex_unlock(&mtx_onulist);
	
	return (onu_number);
}

/***********************************************************
*	getONUDSNList: get the DSN list of ONU nodes that connected to the FSA
*	param-out:  char *, the first pointer of entry buffer that hold the result
*	return:  	the lenth of result;
***********************************************************/
int getONUDSNList(char* pList)
{
	unsigned char onu_index;

	memset(pList, 0, MAX_ONU_NUM*MAX_ONUDSN_LEN);
	
	for(onu_index=1; onu_index<=MAX_ONU_NUM; onu_index++)
	{
		if(ONUDsn[onu_index] == 0) 
			continue;

		memcpy(pList + (onu_index-1)*MAX_ONUDSN_LEN, ONUDsnFull[onu_index], MAX_ONUDSN_LEN);
	}
	
	return (MAX_ONU_NUM*MAX_ONUDSN_LEN);
}

/***********************************************************
*	setFSAAdmin: set the FSA card and port admin state
*	param-in:  index 0: FSA card, 1~2 GE port
*	param-in:  admin_state 2: disable, 1: enable
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setFSAAdmin(unsigned char index, unsigned char admin_state)
{
	struct ip18_rw_struct regstateTX , regstateRX;
	int offset;
	
	if(index == 0)
		simAdminState[0] = admin_state;
	else 
	{
		//simFSAPort[index-1].admin_state = admin_state;

		regstateTX.reg = 0xD9;//transimit 
		regstateRX.reg = 0x1F;//receive
		if ( (ip18_read_reg(&regstateTX) != 0 ) || (ip18_read_reg(&regstateRX) != 0 ))
			return -1;
		logger(LOG_DEBUG, "reg 0xD9  and 0x1F's  value is %x , %x \n", regstateTX.value , regstateRX.value);

		offset = 7 + index;
		if ( 1 == admin_state )
		{
			regstateTX.value = regstateTX.value | (1 << offset);
			regstateRX.value = regstateRX.value | (1 << offset);
		}
		else
		{
			regstateTX.value  &= ~(1 << offset);
			regstateRX.value  &= ~(1 << offset);

		}
		//set.
		regstateTX.reg = 0xD9;//transimit 
		regstateRX.reg = 0x1F;//receive
		if ( (ip18_write_reg( &regstateTX ) != 0 ) || (ip18_write_reg( &regstateRX) != 0 ))
			return -1;
		logger(LOG_DEBUG, "reg 0xD9 and 0x1F's set value is %x , %x \n", regstateTX.value, regstateRX.value );			
				
	}
	
	return 0;
}


int setFSAstate( CARD_SET_FSA *state )
{

	struct ip18_rw_struct regstate, regstate2;

	// set port clock delay
	if(setPortClockDelay(state->igmp_snooping) !=0)
	{
		logger (LOG_ERR, "Set 1826 Port clock delay failed, set value is 0x%02x\n", state->igmp_snooping);
		return -1;
	}

	//set igmp snooping
	regstate.reg = 0xC0;
	if ( 0 != (ip18_read_reg(&regstate)) )
		return -1;
	if ( 1 == (state->igmp_snooping&0x03) )	
		regstate.value = regstate.value | (1<<0);
	else
		regstate.value &= ~(1<<0);
	regstate.reg = 0xC0;
	if ( 0 != ip18_write_reg(&regstate))
		return -1;
	logger (LOG_DEBUG, "set fsa state igmp snooping successfull, set value is %d\n", regstate.value  );

	//set broadcast storm
	regstate.reg = 0x42;
	regstate2.reg = 0x43;
	if ( 1 == state->broadcaststorm_ctrl )
	{
		regstate.value = 0xFFFF;
		regstate2.value = 0xFFFF;
		if ( (0 != ip18_write_reg( &regstate )) || ( 0 != ip18_write_reg(&regstate2)) )
			return -1;
		logger (LOG_DEBUG, "set fsa state broadcast storm successful, set value is %d, %d\n",regstate.value, regstate2.value);
		
	}
	else
	{
		regstate.value = 0x0000;
		regstate2.value = 0x0000;
		if ( (0 != ip18_write_reg( &regstate )) || ( 0 != ip18_write_reg(&regstate2)) )
			return -1;
	}

	//set broadcast storm threshold.
	regstate.reg = 0x44;
	if ( 0 != (ip18_read_reg(&regstate)) )
		return -1;
	logger (LOG_DEBUG, "raw 0x44 register value is %d\n", regstate.value);
	//regstate.value &= ~(0x3F<<10);//clear
	regstate.reg = 0x44;
	regstate.value = (regstate.value & 0x3FF);
	regstate.value = regstate.value |(state->storm_threshold << 10);//set.
	if ( 0 != ip18_write_reg(&regstate))
		return -1;
	logger (LOG_DEBUG, "set fsa broadcast storm threshold successful, set value is %d\n", regstate.value);

	return 0;
		
}

/***********************************************************
*	getONUInfo: get the ONU node data
*	param-in:  onu id
*	param-out:  ONU_LIST_ENTRY* , the  pointer of  buffer that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int getONUInfo(unsigned char onu_id, ONU_INFO * onu_info)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	
	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;

	onu_info->node_id = ONUStatusList[onu_id].node_id;
	onu_info->node_sn =  ONUStatusList[onu_id].node_sn;
	strncpy(onu_info->onu_name, ONUStatusList[onu_id].onu_name, MAX_DISPLAYNAME_LEN);

#ifndef DAT_CARD
	/* get the admin state */
	msg_type = CMD_FPGA_ADMIN; /* reg read, fsa->onu, cmd=002*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, NULL, 0, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get ONU Admin state error !\n");
		return ERROR_COMM_FIBER;
	} else {
		if(buffer[0] == 1)
			onu_info->admin_state = 1; // enabled 
		else
			onu_info->admin_state = 2; //disabled 
	}	 
#endif
	/* get SFP-N state*/
	msg_type = CMD_FIBER_STATE | TYPE_DEV_FIBER1; /* reg read, fsa->onu*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, NULL, 0, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get Fiber 1  state error !\n");
		return(ERROR_COMM_FIBER);
	} else {
		if(buffer[0] == 0)
			onu_info->spf_n_state = 2; // up 
		else if(buffer[0] == 6)
			onu_info->spf_n_state = 3; //down
		else if(buffer[0] == 7)
			onu_info->spf_n_state = 1; //removed
		else
			onu_info->spf_n_state = 4; //error

	}	 

	/* get SFP-F state*/
	msg_type = CMD_FIBER_STATE | TYPE_DEV_FIBER2; /* reg read, fsa->onu*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, NULL, 0, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get Fiber 2  state error !\n");
		return ERROR_COMM_FIBER;
	} else {
		if(buffer[0] == 0)
			onu_info->spf_f_state = 2; // up 
		else if(buffer[0] == 6)
			onu_info->spf_f_state = 3; //down
		else if(buffer[0] == 7)
			onu_info->spf_f_state = 1; //removed
		else
			onu_info->spf_f_state = 4; //error

	}	 

#ifndef DAT_CARD
	/* get broadcaststorm_ctrl*/
	buffer[0] = 0x10;
	buffer[1] = 0;	
	buffer[2] = 0;		
	buffer[3] = 0;
	msg_type = TYPE_DEV_KS8995;	/* reg read, fsa->onu*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 4, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get broadcaststorm_ctrl error !\n");
		return ERROR_COMM_FIBER;
	} else {
		if(buffer[1] < 0x80)
		{
			onu_info->broadcaststorm_ctrl= 2; // 2:disable
			onuInfo[onu_id].broadcaststorm_ctrl = 2;
		}
		else
		{
			onu_info->broadcaststorm_ctrl= 1; // 1:enable
			onuInfo[onu_id].broadcaststorm_ctrl = 1;
		}
	}

	/* get storm_threshold*/
	buffer[0] = 0x06;
	buffer[1] = 0;
	buffer[2] = 0x07;
	buffer[3] = 0;	
	buffer[4] = 0;		
	buffer[5] = 0;
	msg_type = TYPE_DEV_KS8995;	/* reg read, fsa->onu*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 6, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get storm_threshold error !\n");
		return ERROR_COMM_FIBER;
	}
	else
	{
		onu_info->storm_threshold = htons(((buffer[1] & 0x07) << 8) + buffer[3]);
		onuInfo[onu_id].storm_threshold = onu_info->storm_threshold;
	}
#endif	
	onu_info->node_sn =  ONUStatusList[onu_id].node_sn;

	return 0;
}


/***********************************************************
*	getONUUartInfo: get the ONU node Uart Info
*	param-in:  onu id
*	param-out:  ONU_UART_CONF* , the  pointer of  buffer that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int getONUUartInfo(unsigned char onu_id, ONU_UART_CONF *onuUartConfInfo, unsigned char * num)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	
	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;

	/* get the uart info */

	msg_type = CMD_FPGA_DATA; /* reg read, fsa->onu, cmd=110*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, NULL, 0, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get ONU Uart info error !\n");
		return ERROR_COMM_FIBER;
	} else {
		logger(LOG_DEBUG, "buffer[0]=%02x,buffer[1]=%02x,buffer[2]=%02x, buffer[3]=%02x,buffer[4]=%02x,buffer[5]=%02x\n",
				buffer[0],buffer[1],buffer[2], buffer[3],buffer[4],buffer[5]);

#ifdef DAT_CARD
		*num = 5; /* support 5 uart port per DAU */
		onuUartConfInfo[0].port = 0;
		onuUartConfInfo[0].function = 0x03 & buffer[0];
		onuUartConfInfo[0].baudrate = 0x07 & buffer[1];
		onuUartConfInfo[0].time = buffer[2];
//		onuUartConfInfo[0].control= buffer[3];
		onuUartInfo[onu_id][0].baudrate = onuUartConfInfo[0].baudrate;
		onuUartInfo[onu_id][0].function = onuUartConfInfo[0].function;
		onuUartInfo[onu_id][0].time = onuUartConfInfo[0].time;
//		onuUartInfo[onu_id][0].control = onuUartConfInfo[0].control;
		
		onuUartConfInfo[1].port = 1;
		onuUartConfInfo[1].function = 0x03 & buffer[4];
		onuUartConfInfo[1].baudrate = 0x07 & buffer[5];
		onuUartConfInfo[1].time = buffer[6];
//		onuUartConfInfo[1].control= buffer[7];
		onuUartInfo[onu_id][1].baudrate = onuUartConfInfo[1].baudrate;
		onuUartInfo[onu_id][1].function = onuUartConfInfo[1].function;
		onuUartInfo[onu_id][1].time = onuUartConfInfo[1].time;
//		onuUartInfo[onu_id][1].control = onuUartConfInfo[1].control;

		onuUartConfInfo[2].port = 2;
		onuUartConfInfo[2].baudrate = 0x07 & buffer[9];
		onuUartConfInfo[2].time = buffer[10];
//		onuUartConfInfo[2].control= buffer[11];
		if(buffer[11]&0x80)
			onuUartConfInfo[2].function = 0x80;
		else
			onuUartConfInfo[2].function = 0;			
		onuUartInfo[onu_id][2].baudrate = onuUartConfInfo[2].baudrate;
		onuUartInfo[onu_id][2].time = onuUartConfInfo[2].time;
//		onuUartInfo[onu_id][2].control = onuUartConfInfo[2].control;

		onuUartConfInfo[3].port = 3;
		onuUartConfInfo[3].baudrate = 0x07 & buffer[13];
		onuUartConfInfo[3].time = buffer[14];
//		onuUartConfInfo[3].control= buffer[15];
		if(buffer[15]&0x80)
			onuUartConfInfo[3].function = 0x80;
		else
			onuUartConfInfo[3].function = 0;			
		onuUartInfo[onu_id][3].baudrate = onuUartConfInfo[3].baudrate;
		onuUartInfo[onu_id][3].time = onuUartConfInfo[3].time;
//		onuUartInfo[onu_id][3].control = onuUartConfInfo[3].control;

		onuUartConfInfo[4].port = 4;
		onuUartConfInfo[4].baudrate = 0x07 & buffer[17];
		onuUartConfInfo[4].time = buffer[18];
//		onuUartConfInfo[4].control= buffer[19];
		if(buffer[19]&0x80)
			onuUartConfInfo[4].function = 0x80;
		else
			onuUartConfInfo[4].function = 0;			
		onuUartInfo[onu_id][4].baudrate = onuUartConfInfo[4].baudrate;
		onuUartInfo[onu_id][4].time = onuUartConfInfo[4].time;
//		onuUartInfo[onu_id][4].control = onuUartConfInfo[4].control;

#else
		*num = 2; /* support 2 uart port per ONU */

		onuUartConfInfo[0].port = 0;
		onuUartConfInfo[0].function = 0x03 & buffer[0];
		onuUartConfInfo[0].baudrate = 0x07 & buffer[1];
		onuUartConfInfo[0].time = buffer[2];
		onuUartInfo[onu_id][0].baudrate = onuUartConfInfo[0].baudrate;
		onuUartInfo[onu_id][0].function = onuUartConfInfo[0].function;
		onuUartInfo[onu_id][0].time = onuUartConfInfo[0].time;
		onuUartConfInfo[1].port = 1;
		onuUartConfInfo[1].function = 0x03 & buffer[3];
		onuUartConfInfo[1].baudrate = 0x07 & buffer[4];
		onuUartConfInfo[1].time = buffer[5];
		onuUartInfo[onu_id][1].baudrate = onuUartConfInfo[1].baudrate;
		onuUartInfo[onu_id][1].function = onuUartConfInfo[1].function;
		onuUartInfo[onu_id][1].time = onuUartConfInfo[1].time;
#endif		
	}

	return 0;
}

/***********************************************************
*	getONUIO: get the ONU IO Info
*	param-in:  onu id
*	param-out:  ONU_IO* , the  pointer of  buffer that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int getONUIO(unsigned char onu_id, ONU_IO *onu_io)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	
	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;

	/* get the uart info */

	msg_type = TYPE_DEV_DATAP|CMD_DATA_IO_QUERY; /* reg read, fsa->onu, cmd=110*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, NULL, 0, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get ONU IO info error !\n");
		return(ERROR_COMM_FIBER);
	} else {
		logger(LOG_DEBUG, "buffer[0]=%02x\n",	buffer[0]);
		onu_io->in = buffer[0];
		onu_io->out_select = 0;
		onu_io->out_value= 0;
	}

	return 0;
}



/***********************************************************
*	getONUPortMirror: get the ONU node Port Mirror
*	param-in:  onu id
*	param-out:  ONU_PORTMIRROR_CONF* , the  pointer of  buffer that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int getONUPortMirror(unsigned char onu_id, ONU_PORTMIRROR_CONF *onuPortMirrorConfInfo)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	
	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;
	
	/* get Port Mirror*/
	buffer[0] = 0x11;
	buffer[1] = 0;	
	buffer[2] = 0x21;		
	buffer[3] = 0;
	buffer[4] = 0x31;
	buffer[5] = 0;
	buffer[6] = 0x41;
	buffer[7] = 0;
	buffer[8] = 0x51;
	buffer[9] = 0;

	buffer[10] = 0x03;
	buffer[11] = 0;
	buffer[12] = 0x05;
	buffer[13] = 0;
	buffer[14] = 0;
	buffer[15] = 0;
	msg_type = TYPE_DEV_KS8995;	/* reg read, fsa->onu*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 16, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get port mirror error !\n");
		return(ERROR_COMM_FIBER);
	}
	else
	{
		if((buffer[1] & 0x80) == 0x80)
			onuPortMirrorConfInfo->port1[0] = 0x01;
		else
			onuPortMirrorConfInfo->port1[0] = 0x00;
		if((buffer[1] & 0x40) == 0x40)
			onuPortMirrorConfInfo->port1[1] = 0x01;
		else
			onuPortMirrorConfInfo->port1[1] = 0x00;
		if((buffer[1] & 0x20) == 0x20)
			onuPortMirrorConfInfo->port1[2] = 0x01;
		else
			onuPortMirrorConfInfo->port1[2] = 0x00;

		if((buffer[3] & 0x80) == 0x80)
			onuPortMirrorConfInfo->port2[0] = 0x01;
		else
			onuPortMirrorConfInfo->port2[0] = 0x00;
		if((buffer[3] & 0x40) == 0x40)
			onuPortMirrorConfInfo->port2[1] = 0x01;
		else
			onuPortMirrorConfInfo->port2[1] = 0x00;
		if((buffer[3] & 0x20) == 0x20)
			onuPortMirrorConfInfo->port2[2] = 0x01;
		else
			onuPortMirrorConfInfo->port2[2] = 0x00;

		if((buffer[5] & 0x80) == 0x80)
			onuPortMirrorConfInfo->port3[0] = 0x01;
		else
			onuPortMirrorConfInfo->port3[0] = 0x00;
		if((buffer[5] & 0x40) == 0x40)
			onuPortMirrorConfInfo->port3[1] = 0x01;
		else
			onuPortMirrorConfInfo->port3[1] = 0x00;
		if((buffer[5] & 0x20) == 0x20)
			onuPortMirrorConfInfo->port3[2] = 0x01;
		else
			onuPortMirrorConfInfo->port3[2] = 0x00;

		if((buffer[7] & 0x80) == 0x80)
			onuPortMirrorConfInfo->port4[0] = 0x01;
		else
			onuPortMirrorConfInfo->port4[0] = 0x00;
		if((buffer[7] & 0x40) == 0x40)
			onuPortMirrorConfInfo->port4[1] = 0x01;
		else
			onuPortMirrorConfInfo->port4[1] = 0x00;
		if((buffer[7] & 0x20) == 0x20)
			onuPortMirrorConfInfo->port4[2] = 0x01;
		else
			onuPortMirrorConfInfo->port4[2] = 0x00;

		if((buffer[9] & 0x80) == 0x80)
			onuPortMirrorConfInfo->port5[0] = 0x01;
		else
			onuPortMirrorConfInfo->port5[0] = 0x00;
		if((buffer[9] & 0x40) == 0x40)
			onuPortMirrorConfInfo->port5[1] = 0x01;
		else
			onuPortMirrorConfInfo->port5[1] = 0x00;
		if((buffer[9] & 0x20) == 0x20)
			onuPortMirrorConfInfo->port5[2] = 0x01;
		else
			onuPortMirrorConfInfo->port5[2] = 0x00;

		if((buffer[11] & 0x80) == 0x80)
			onuPortMirrorConfInfo->badframes = 0x01;
		else
			onuPortMirrorConfInfo->badframes = 0x00;

		if((buffer[13] & 0x01) == 0x01)
			onuPortMirrorConfInfo->sourceanddestination = 0x01;
		else
			onuPortMirrorConfInfo->sourceanddestination = 0x00;
	}
	return 0;
}


/***********************************************************
*	getONUQoS: get the ONU node QoS
*	param-in:  onu id
*	param-out:  ONU_QoS_CONF* , the  pointer of  buffer that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int getONUQoS(unsigned char onu_id, ONU_QoS_CONF *onuQoSConfInfo)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	
	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;

	/* get QoS*/
	buffer[0] = 0x10;
	buffer[1] = 0;	
	buffer[2] = 0x20;		
	buffer[3] = 0;
	buffer[4] = 0x30;
	buffer[5] = 0;
	buffer[6] = 0x40;
	buffer[7] = 0;
	buffer[8] = 0x50;
	buffer[9] = 0;

	buffer[10] = 0x04;
	buffer[11] = 0;
	buffer[12] = 0x05;
	buffer[13] = 0;
	buffer[14] = 0;
	buffer[15] = 0;
	msg_type = TYPE_DEV_KS8995;	/* reg read, fsa->onu*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 16, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get QoS error !\n");
		return(ERROR_COMM_FIBER);
	}
	else
	{
		logger(LOG_DEBUG, "buffer[11], %x!\n", buffer[11]);
		logger(LOG_DEBUG, "buffer[13], %x!\n", buffer[13]);
		
		if((buffer[1] & 0x01) == 0x01)
			onuQoSConfInfo->priorityEnable[0] = 0x01;
		else
			onuQoSConfInfo->priorityEnable[0] = 0x00;

		if((buffer[3] & 0x01) == 0x01)
			onuQoSConfInfo->priorityEnable[1] = 0x01;
		else
			onuQoSConfInfo->priorityEnable[1] = 0x00;

		if((buffer[5] & 0x01) == 0x01)
			onuQoSConfInfo->priorityEnable[2] = 0x01;
		else
			onuQoSConfInfo->priorityEnable[2] = 0x00;

		if((buffer[7] & 0x01) == 0x01)
			onuQoSConfInfo->priorityEnable[3] = 0x01;
		else
			onuQoSConfInfo->priorityEnable[3] = 0x00;

		if((buffer[9] & 0x01) == 0x01)
			onuQoSConfInfo->priorityEnable[4] = 0x01;
		else
			onuQoSConfInfo->priorityEnable[4] = 0x00;

		if((buffer[13] & 0x0C) == 0x00)
			onuQoSConfInfo->prioritySchemeSelect = 0x00;
		else if((buffer[13] & 0x0C) == 0x04)
			onuQoSConfInfo->prioritySchemeSelect = 0x01;
		else if((buffer[13] & 0x0C) == 0x08)
			onuQoSConfInfo->prioritySchemeSelect = 0x10;
		else if((buffer[13] & 0x0C) == 0x0C)
			onuQoSConfInfo->prioritySchemeSelect = 0x11;
		
	}
	return 0;
}


/***********************************************************
*	getONUVLAN: get the ONU node VLAN
*	param-in:  onu id
*	param-out:  ONU_VLAN_CONF* , the  pointer of  buffer that hold the result
*	param-out:  unsigned char* , the  pointer of  VLAN Table Number that hold the result
*	return:  	0  - success 
*			-1 - fail
***********************************************************/

int getONUVLAN(unsigned char onu_id, ONU_VLAN_CONF *onuVLANConfInfo, unsigned char *num)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	unsigned char write_buffer[FIFO_MSG_LEN];
	unsigned char read_buffer;
	unsigned char entry;
	
	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;

	//fill the reg address data
	//msg_type = TYPE_DEV_KS8995 | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, KS8995A, cmd=000*/
	/*buffer[0] = 0x76;
	buffer[1] = 0x1F;
	buffer[2] = 0x77;
	buffer[4] = 0x78;
	buffer[5] = 0x01;
	buffer[6] = 0x6E;
	buffer[7] = 0x04;
	buffer[8] = 0x6F;
	buffer[10] = 0x00;
	buffer[11] = 0x00;
	
	for(entry = 1;entry <= 16;entry++)
	{
		buffer[3] = (entry - 1) << 4;
		buffer[9] = entry - 1;

		send_fifomsg_waitrsp(onu_id, msg_type, buffer, 12, buffer, FIFO_MSG_TIMEOUT);
		
	}*/



	//fill the reg address data
	msg_type = TYPE_DEV_KS8995 | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, KS8995A, cmd=000*/
	buffer[0] = 0x76;
	buffer[1] = 0x1F;
	buffer[2] = 0x00;
	buffer[3] = 0x00;
		
	send_fifomsg_waitrsp(onu_id, msg_type, buffer, 4, buffer, FIFO_MSG_TIMEOUT);



	
	/* get 802.1Q VLAN Enable or disable*/
	/*buffer[0] = 0x05;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;
	msg_type = TYPE_DEV_KS8995;	// reg read, fsa->onu
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 4, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get reg 0x05 error !\n");
		return -1;
	}
	else
	{
		logger(LOG_DEBUG, "reg 0x05:, %x!\n", buffer[1]);
		read_buffer = buffer[1];
	}*/

	write_buffer[0] = 0x6E;
	write_buffer[1] = 0x14;
	write_buffer[2] = 0x6F;
	write_buffer[4] = 0;		
	write_buffer[5] = 0;

	buffer[0] = 0x76;		
	buffer[1] = 0;
	buffer[2] = 0x77;
	buffer[3] = 0;
	buffer[4] = 0x78;
	buffer[5] = 0;
	buffer[6] = 0;
	buffer[7] = 0;
	for(entry = 1;entry <= 4;entry++)
	{
		/*Indirect Reg acess, write Reg 0x6E, 0x6F first */ 		
		write_buffer[3] = entry - 1;
				
		msg_type = TYPE_DEV_KS8995 | TYPE_MASK_WRITE; 
		if( send_fifomsg_waitrsp(onu_id, msg_type, write_buffer, 6, write_buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			//logger(LOG_ERR, "Get ONU port VLAN error!write reg. ONU id=%d\n", onu_id);
			return(ERROR_COMM_FIBER);
		}

		/* get VLAN*/

		msg_type = TYPE_DEV_KS8995;	/* reg read, fsa->onu*/
		if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 8, buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			//logger(LOG_ERR, "FIFO: Get VLAN error !\n");
			return(ERROR_COMM_FIBER);
		}
		else
		{
			/*logger(LOG_DEBUG, "buffer[1], %x!\n", buffer[1]);
			logger(LOG_DEBUG, "buffer[3], %x!\n", buffer[3]);
			logger(LOG_DEBUG, "buffer[5], %x!\n", buffer[5]);*/

			//if((buffer[1] & 0x20) == 0x20)
			//{
				onuVLANConfInfo[entry - 1].setFlag = 0x00;
				onuVLANConfInfo[entry - 1].valid = (buffer[1] & 0x20) >> 5;
				onuVLANConfInfo[entry - 1].membership = buffer[1] & 0x1F;
				onuVLANConfInfo[entry - 1].id = (buffer[3] & 0xF0) >> 4;
				onuVLANConfInfo[entry - 1].vlan_id = htons(((buffer[3] & 0x0F) << 8) | buffer[5]);
				//(*num) = 4;
			//}
			/*logger(LOG_DEBUG, "valid, %x!\n", onuVLANConfInfo[entry - 1].valid);
			logger(LOG_DEBUG, "membership, %x!\n", onuVLANConfInfo[entry - 1].membership);
			logger(LOG_DEBUG, "id, %x!\n", onuVLANConfInfo[entry - 1].id);
			logger(LOG_DEBUG, "vlan_id, %x!\n", onuVLANConfInfo[entry - 1].vlan_id);*/
		}
	}
	(*num) = entry;
	return 0;
}


/***********************************************************
*	getONUFEPort: get the ONU port data
*	param-in:  onu id
*                      port index
*	param-out:  FE_PORT_STATE* , the  pointer of  buffer that hold the result
*	return:  	0  - success 
*			other - fail
***********************************************************/
int getONUFEPort(unsigned char onu_id, unsigned char index, FE_PORT_STATE * port_info)
{
	int regindex;
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	unsigned char swap;
	
	if( onu_id == 0 || index >3  || onu_id >MAX_ONU_NUM)
		return -1;

	regindex= 0;
	buffer[regindex++] = 0xFF;
	buffer[regindex++] = 0x10 * index + 0x10;
	buffer[regindex++] = 0;		
	buffer[regindex++] = 0;
	
	msg_type = TYPE_DEV_KS8995; /* reg read, fsa->onu, cmd=002*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Get ONU info FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}

	regindex = 2; /* skip the first 2 byte for 0xFF and Reg start address */
	logger(LOG_DEBUG, "8995 REG: 0x12=%0x, 0x1C=%0x, 0x1B=%0x, 0x1E=%0x!\n", buffer[regindex + 0x02], buffer[regindex+ 0x0C], buffer[regindex + 0x0b], buffer[regindex + 0x0e]);

	port_info->portIndex = index + 1;

	/* working mode  Reg 0x1C */
	swap = buffer[regindex+0x0C] & 0x80;
	if(swap == 0)
		port_info->portWorkingMode = 1; //auto(1)
	else
	{
		swap = (buffer[regindex+0x0C] & 0xE0)>>5;
		switch(swap)
		{
			case 7:
				port_info->portWorkingMode = 2;
				break;	//m100-full(2)
			case 6:
				port_info->portWorkingMode = 3;
				break;	//m100-half(3)
			case 5:
				port_info->portWorkingMode = 4;
				break;	//m10-full(4)
			case 4:
				port_info->portWorkingMode = 5;
				break; //m10-half(5)
			default:
				break;
		}
	}

	/* learning mode Reg 0x12 */
	swap = buffer[regindex+0x02] & 0x01;
	if(swap != 0)
		port_info->portMode = 2; //bounded-mac(2)
	else
		port_info->portMode = 1; //learning-mode(1)

	/* admin state  Reg 0x12 */
	swap = buffer[regindex+0x02] & 0x06;
	if(swap == 0x06 )
		port_info->admin_state = 1; //enabled(1)
	else
		port_info->admin_state = 2; //disabled(2)
		
	/* bandwidth control, Reg 0x1B, 0x15~1A*/
	swap = buffer[regindex+0x0B] & 0x60;
	if(swap == 0x60)
		port_info->bw_alloc_recv = htons(((buffer[regindex+0x0A]&0x0F)<<8)|buffer[regindex+0x08]); /* Reg 0x18, 0x1A*/
	else
		port_info->bw_alloc_recv = htons(0x0C80 );/*no contol, > 100M */
	
	swap = buffer[regindex+0x0B] & 0x03;
	if(swap == 0x03)
		port_info->bw_alloc_send= htons(((buffer[regindex+0x07]&0x0F)<<8)|buffer[regindex+0x05]); /* Reg 0x15, 0x17*/
	else
		port_info->bw_alloc_send = htons(0x0C80) ;/*no contol, > 100M */

	/* port state, Reg 0x1E*/
	swap = buffer[regindex+0x0E] & 0x20;
	if(swap==0x20)
		port_info->port_state = 1; //up(1)
	else
		port_info->port_state = 2; //down(2)
		

	/* process the MAC address */
	if(port_info->portMode == 2)
	{
		//bounded-mac(2)
		/*Indirect Reg acess, write Reg 0x6E, 0x6F first */ 
		regindex= 0;
		buffer[regindex++] = 0x6E;
		buffer[regindex++] = 0x10;
		buffer[regindex++] = 0x6F;		
		buffer[regindex++] = index;
		buffer[regindex++] = 0;		
		buffer[regindex++] = 0;
			
		msg_type = TYPE_DEV_KS8995 | TYPE_MASK_WRITE; 
		if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			logger(LOG_ERR, "Get ONU port mac error!write reg. ONU id=%d, port=%d\n", onu_id, index+1);
			return(ERROR_COMM_FIBER);
		}
		/* read Reg from  */
		regindex= 0;
		buffer[regindex++] = 0xFF;
		buffer[regindex++] = 0x71;
		buffer[regindex++] = 0;		
		buffer[regindex++] = 0;
		
		msg_type = TYPE_DEV_KS8995; 
		if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			logger(LOG_ERR, "Get ONU port mac error!read reg. ONU id=%d, port=%d\n", onu_id, index+1);
			return(ERROR_COMM_FIBER);
		}
			
		port_info->bounded_mac[0] = buffer[4];
		port_info->bounded_mac[1] = buffer[5];
		port_info->bounded_mac[2] = buffer[6];
		port_info->bounded_mac[3] = buffer[7];
		port_info->bounded_mac[4] = buffer[8];
		port_info->bounded_mac[5] = buffer[9];

	} else {
	 	//learning-mode(1)
		port_info->bounded_mac[0] = 0;
		port_info->bounded_mac[1] = 0;
		port_info->bounded_mac[2] = 0;
		port_info->bounded_mac[3] = 0;
		port_info->bounded_mac[4] = 0;
		port_info->bounded_mac[5] = 0;
	}

	return 0;
}

int getSFPInfo(unsigned char onu_id, unsigned char index, SFP_INFO * sfp_info)
{
	int regindex;
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	unsigned short word_tmp;
	
	if( onu_id == 0 || onu_id >MAX_ONU_NUM || index >2 || index <1  )
		return -1;

	/* get the first 64 bytes */
	regindex= 0;
	buffer[regindex++] = 0xFF;
	buffer[regindex++] = 0xA0;
	buffer[regindex++] = 0x00;
	buffer[regindex++] = 0;		
	buffer[regindex++] = 0;

	if(index == 1 )
		msg_type = TYPE_DEV_FIBER1; 
	else
		msg_type = TYPE_DEV_FIBER2; 
	
	logger(LOG_DEBUG, "FIFO:Get SFP info-1st 64 bytes! ONU=%d, Index=%d\n", onu_id, index);
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Get ONU info FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}

	memset(sfp_info, 0, sizeof(SFP_INFO));
	sfp_info->sfpIndex = index;

	regindex = 3; /* skip the first 2 byte for 0xFF and Reg start address */

	if(buffer[regindex] != 3)
	{
		sfp_info->sfpCompliance = 0xFF;
		return 0;
	}

	memcpy(sfp_info->sfpVendor, buffer + regindex + 20, 16);
	memcpy(sfp_info->sfpPartNumber, buffer + regindex + 40, 16);
	sfp_info->sfpConnector = buffer[regindex + 2];
	
	sfp_info->sfpTransCode = buffer[regindex + 9] & 0x0F;
	if(sfp_info->sfpTransCode & 0x01)		
		sfp_info->sfpSmLength = buffer[regindex + 14];
	if(sfp_info->sfpTransCode & 0x40)
		sfp_info->sfpMmLength = buffer[regindex + 16];
	else if (sfp_info->sfpTransCode & 0x80)
		sfp_info->sfpMmLength = buffer[regindex + 17];
	sfp_info->sfpCopperLength = buffer[regindex + 18];
	
	sfp_info->sfpBrSpeed = buffer[regindex + 12];	
	
	/* get the reg 94: SFF-8472 Compliance and wave length: reg 60,61 */
	regindex= 0;
	buffer[regindex++] = 94; /* Compliance */
	buffer[regindex++] = 0;
	buffer[regindex++] = 60; /* wave length */
	buffer[regindex++] = 0;
	buffer[regindex++] = 61;
	buffer[regindex++] = 0;
	buffer[regindex++] = 84; /*data code*/
	buffer[regindex++] = 0;
	buffer[regindex++] = 85;
	buffer[regindex++] = 0;
	buffer[regindex++] = 0;		
	buffer[regindex++] = 0;

	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Get ONU info FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}
	logger(LOG_DEBUG, "FIFO:Get SFP SFF-8472 Compliance! ONU=%d, Index=%d, result=%d\n", onu_id, index, buffer[1]);
	
	sfp_info->sfpCompliance = buffer[1];
	
	word_tmp = buffer[3] *0x100 + buffer[5];
	sfp_info->sfpWavelength = htons(word_tmp);

	if(sfp_info->sfpCompliance == 1)
	{
		regindex= 0;
		buffer[regindex++] = 0xFF;
		buffer[regindex++] = 0xA2;
		buffer[regindex++] = 96;
		buffer[regindex++] = 0;
//		buffer[regindex++] = 97;
		buffer[regindex++] = 0;
/*
//		buffer[regindex++] = 102;
		buffer[regindex++] = 0;
		buffer[regindex++] = 103;
		buffer[regindex++] = 0;		
		buffer[regindex++] = 104;
		buffer[regindex++] = 0;		
		buffer[regindex++] = 105;
		buffer[regindex++] = 0;
		
		buffer[regindex++] = 0;		
		buffer[regindex++] = 0;
*/
		logger(LOG_DEBUG, "FIFO:Get SFP  Diagnostic Monitoring data! ONU=%d, Index=%d\n", onu_id, index);
		if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			logger(LOG_ERR, "Get ONU info FIFO message error !\n");
			return(ERROR_COMM_FIBER);
		}
		/* get Diagnostic Monitoring data */
		sfp_info->sfpTemperature = buffer[3]; 
		word_tmp = buffer[9] *0x100 + buffer[10];
		sfp_info->sfpTranPower = htons(word_tmp);
		word_tmp = buffer[11] *0x100 + buffer[12];
		sfp_info->sfpRecvPower = htons(word_tmp);
	}

	return 0;
}

/***********************************************************
*	setONUName: set the ONU node name
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUName(unsigned char onu_id, char * name)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];

	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;

	//fill the reg address data
	msg_type = CMD_FPGA_ONUNAME | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, fpga, cmd=001*/

	if(dumpFifo)
		logger(LOG_DEBUG, "Set ONU Name, %d!\n", onu_id);
	
	if( send_fifomsg_waitrsp(onu_id, msg_type, name, MAX_DISPLAYNAME_LEN, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Set ONU Name FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}

	strncpy(ONUStatusList[onu_id].onu_name, name, MAX_DISPLAYNAME_LEN);
	
	return 0;
}


/***********************************************************
*	setONUStorm: set the ONU storm
*	param-in:  onu id
*	param-in:  broadcaststorm_ctrl 1:enable, 2: disable
*	param-in:  storm_threshold
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
//int setONUStorm(unsigned char onu_id, unsigned char broadcaststorm_ctrl, unsigned char storm_threshold)
int setONUStorm(unsigned char onu_id, ONU_SET * onuSet)

{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	unsigned char read_buffer[FIFO_MSG_LEN];
	unsigned char i = 0;

	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;

	/* get broadcaststorm_ctrl*/
	read_buffer[0] = 0x10;
	read_buffer[1] = 0;
	read_buffer[2] = 0x06;
	read_buffer[3] = 0;
	read_buffer[4] = 0;		
	read_buffer[5] = 0;
	msg_type = TYPE_DEV_KS8995;	/* reg read, fsa->onu*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, read_buffer, 6, read_buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get broadcaststorm_ctrl error !\n");
		return(ERROR_COMM_FIBER);
	}
	
	if(onuSet->setFlag & MD_ONU_STORM)
	{
		for(i = 0;i < 4;i++)
		{
			buffer[0 + 2 * i] = 0x10 + 0x10 * i;
			if(onuSet->broadcaststorm_ctrl == 1) //1:enable
				buffer[1 + 2 * i] = 0x80 | read_buffer[1];
			else						//2:disable
				buffer[1 + 2 * i] = (0x80 | read_buffer[1]) & 0x7F;
		}

		if(onuSet->broadcaststorm_ctrl == 1) //1:enable
		{
			buffer[8] = 0x07;
			//buffer[9] = onuSet->storm_threshold;
			buffer[9] = ntohs(onuInfo[onu_id].storm_threshold) & 0xFF;
			buffer[10] = 0x06;
			buffer[11] = ((ntohs(onuInfo[onu_id].storm_threshold) >> 8) & 0x07) | (read_buffer[3] & 0xF8);
			buffer[12] = 0;
			buffer[13] = 0;
			i = 14;
		}
		else
		{
			buffer[8] = 0;
			buffer[9] = 0;
			i = 10;
		}
	}

	if(onuSet->setFlag & MD_ONU_STMTHRESHOLD)
	{
		for(i = 0;i < 4;i++)
		{
			buffer[0 + 2 * i] = 0x10 + 0x10 * i;
			//if(onuSet->broadcaststorm_ctrl == 1) //1:enable
			if(onuInfo[onu_id].broadcaststorm_ctrl == 1)
				buffer[1 + 2 * i] = 0x80 | read_buffer[1];
			else						//2:disable
				buffer[1 + 2 * i] = (0x80 | read_buffer[1]) & 0x7F;
		}

		//if(onuSet->broadcaststorm_ctrl == 1) //1:enable
		if(onuInfo[onu_id].broadcaststorm_ctrl == 1)
		{
			buffer[8] = 0x07;
			buffer[9] = ntohs(onuSet->storm_threshold) & 0xFF;
			buffer[10] = 0x06;
			buffer[11] = ((ntohs(onuSet->storm_threshold) >> 8) & 0x07) | (read_buffer[3] & 0xF8);
			buffer[12] = 0;
			buffer[13] = 0;
			i = 14;
		}
		else
		{
			buffer[8] = 0;
			buffer[9] = 0;
			i = 10;
		}
	}
		
	if(onuSet->setFlag == 0xFF)
	{
		for(i = 0;i < 4;i++)
		{
			buffer[0 + 2 * i] = 0x10 + 0x10 * i;
			if(onuSet->broadcaststorm_ctrl == 1) //1:enable
				buffer[1 + 2 * i] = 0x80 | read_buffer[1];
				//buffer[1 + 2 * i] = 0x80;
			else						//2:disable
				buffer[1 + 2 * i] = (0x80 | read_buffer[1]) & 0x7F;
				//buffer[1 + 2 * i] = 0x00;
		}

		if(onuSet->broadcaststorm_ctrl == 1) //1:enable
		{
			buffer[8] = 0x07;
			buffer[9] = ntohs(onuSet->storm_threshold) & 0xFF;
			buffer[10] = 0x06;
			buffer[11] = ((ntohs(onuSet->storm_threshold) >> 8) & 0x07) | (read_buffer[3] & 0xF8);
			buffer[12] = 0;
			buffer[13] = 0;
			i = 14;
		}
		else
		{
			buffer[8] = 0;
			buffer[9] = 0;
			i = 10;
		}
	}
	//fill the reg address data
		msg_type = TYPE_DEV_KS8995|TYPE_MASK_WRITE;

	if(dumpFifo)
		logger(LOG_DEBUG, "Set ONU port storm, %d!\n", onu_id);
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, i, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "set ONU FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}
	else
		return 0;
	
}


/***********************************************************
*	setONUAdmin: set the ONU node and port admin state
*	param-in:  onu id
*	param-in:  index 0: ONU self, 1~4 ONU FE port
*	param-in:  admin_state 0: disable, 1: enable
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUAdmin(unsigned char onu_id,  unsigned char index, unsigned char admin_state)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];

	if(  index>4 || onu_id == 0 || onu_id >MAX_ONU_NUM)
		return -1;
	
#ifdef DAT_CARD
	return 0;
#endif

	if(index == 0)
	{
//		return 0;
		//fill the reg address data
		msg_type = CMD_FPGA_ADMIN  | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, fpga, cmd=010*/

		if( admin_state == 1)
			buffer[0] = 1; /*enabled*/
		else 
			buffer[0] = 0; /*disabled*/
	
		if(dumpFifo)
			logger(LOG_DEBUG, "Set ONU Admin, %d!\n", onu_id);
		
		if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 1, buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			logger(LOG_ERR, "Set ONU Name FIFO message error !\n");
			return(ERROR_COMM_FIBER);
		}

	} else {
		unsigned char reg_offset;
		unsigned char reg_0x12;
		int regindex;

		reg_offset = 0x10 * (index-1);

		/* read the reg first */
		buffer[0] = 0x12 + reg_offset;
		buffer[1] = 0;	
		buffer[2] = 0;		
		buffer[3] = 0;
	
		msg_type = TYPE_DEV_KS8995; 
		if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 4, buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			logger(LOG_ERR, "Read ONU FIFO message error !\n");
			return(ERROR_COMM_FIBER);
		}

		reg_0x12 = buffer[1];
		
		if(dumpFifo)
			logger(LOG_DEBUG, "Got ONU 8995 Reg, %d-%d, reg_0x12=%02x!\n", onu_id, index, reg_0x12);
		
		regindex= 0;
		buffer[regindex++] = 0x12 + reg_offset;
		if( admin_state == 1)
			buffer[regindex++] = reg_0x12 | 0x06;	/*enabled*/
		else 
			buffer[regindex++] = reg_0x12 & 0xF9;	 /*disabled*/
		
		buffer[regindex++] = 0;		
		buffer[regindex++] = 0;
	
		msg_type = TYPE_DEV_KS8995|TYPE_MASK_WRITE; 
		if(dumpFifo)
			logger(LOG_DEBUG, "Set ONU port admin, %d-%d!\n", onu_id, index);
		if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			logger(LOG_ERR, "Read ONU FIFO message error !\n");
			return(ERROR_COMM_FIBER);
		}
	}
	return 0;
}

/***********************************************************
*	setONUUart: set the ONU node Uart
*	param-in:  onu id
*	param-in:  bus_Baudrate bit(7):232/485  (0:232,1:485),bit(2...0):"111"~"000" baudrate:115200,57600,38400,19200,9600,4800,2400,1200
*	param-in:  Function 0x00:debug port,0x01:data comm
*	param-in:  Time RX,1ms~256ms
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUUart(unsigned char onu_id, ONU_UART_CONF *onuUartConf )
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];

	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;
	
#ifdef DAT_CARD
	if( onuUartConf->port >= 5)
#else
	if( onuUartConf->port >= 2)
#endif
	{
		logger(LOG_ERR, "Un suported ONU uart port: %u!\n", onuUartConf->port);
		return -1;
	}

	//fill the reg address data
	msg_type = CMD_FPGA_DATA | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, fpga, cmd=110*/

	logger(LOG_DEBUG, "flag, 0x%02x!\n", onuUartConf->setFlag);

	//fill initial value
	buffer[0] = onuUartInfo[onu_id][onuUartConf->port].baudrate;
	buffer[1] = onuUartInfo[onu_id][onuUartConf->port].function;
	buffer[2] = onuUartInfo[onu_id][onuUartConf->port].time;
//	buffer[3] = onuUartInfo[onu_id][onuUartConf->port].control;

	if ( onuUartConf->setFlag & MD_DATACH_BRAUDRATE )
		buffer[0] = onuUartConf->baudrate& 0x07;

	if ( onuUartConf->setFlag & MD_DATACH_PORTSEL )
		buffer[1] = onuUartConf->function;

	if ( onuUartConf->setFlag & MD_DATACH_RECVBUFF )
		buffer[2] = onuUartConf->time;
	
#ifdef DAT_CARD
//	if ( onuUartConf->setFlag & MD_DATACH_CONTROL)
//		buffer[3] = onuUartConf->control;
	buffer[3] = 0x00;
#else
	buffer[3] = 0x00;
#endif

	switch(onuUartConf->port)
	{
		case 0:
			break;
		case 1:
			buffer[0] |= 0x40;
			break;			
		case 2:
			buffer[0] |= 0x80;
			break;			
		case 3:
			buffer[0] |= 0x90;
			break;			
		case 4:
			buffer[0] |= 0xa0;
			break;			
	}	
	
	if(dumpFifo)
		logger(LOG_DEBUG, "Set ONU Uart, %d!\n", onu_id);
	
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 4, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Set ONU Uart FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}
	else 
	{	
		// update cache
		if ( onuUartConf->setFlag & MD_DATACH_BRAUDRATE )
			onuUartInfo[onu_id][onuUartConf->port].baudrate = onuUartConf->baudrate;

		if ( onuUartConf->setFlag & MD_DATACH_PORTSEL )
			onuUartInfo[onu_id][onuUartConf->port].function= onuUartConf->function;

		if ( onuUartConf->setFlag & MD_DATACH_RECVBUFF )
			onuUartInfo[onu_id][onuUartConf->port].time = onuUartConf->time;

//		if ( onuUartConf->setFlag & MD_DATACH_CONTROL)
//			onuUartInfo[onu_id][onuUartConf->port].control = onuUartConf->control;

		if(dumpFifo)
			logger(LOG_DEBUG, "Set ONU Uart done, baudrate=%d, function=%d, time=%d\n", onuUartConf->baudrate, onuUartConf->function, onuUartConf->time);

		return 0;
	}
}


/***********************************************************
*	setONUIO: set the ONU node IO out
*	param-in:  onu id
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUIO(unsigned char onu_id, ONU_IO *onu_io )
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];

	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;
	

	//fill the reg address data
	msg_type = TYPE_DEV_DATAP | CMD_DATA_IO  | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, fpga, cmd=110*/

	//fill initial value
	buffer[0] = ~onu_io->out_select;
	buffer[1] = onu_io->out_value;
		
	if(dumpFifo)
		logger(LOG_DEBUG, "Set ONU IO, %d!\n", onu_id);
	
	if( send_fifomsg(onu_id, msg_type, buffer, 2) !=0 )
	{
		logger(LOG_ERR, "Set ONU IO message error !\n");
		return -1;
	}
	
	return 0;
}


/***********************************************************
*	setONUID: set the ONU node new ID
*	param-in:  onu id
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUID(unsigned char onu_id, ONU_SETID *onu_setid )
{
	unsigned char buffer[FIFO_MSG_LEN];

	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;
	
	if(onu_id == onu_setid->new_onuid)
		return 0;
	
	if( onu_setid->new_onuid == 0|| onu_setid->new_onuid >MAX_ONU_NUM)
		return(ERROR_WRONG_ARGS);

	if(memcmp(ONUDsnFull[onu_id], onu_setid->onu_dsn, MAX_ONUDSN_LEN) != 0)
		return(ERROR_DSN_NOTMATCH);

	if( (ONUStatusList[onu_setid->new_onuid].node_state == ONU_STATE_ONLINED) 
		|| (ONUStatusList[onu_setid->new_onuid].node_state == ONU_STATE_ONLINEE))
		return(ERROR_NEWID_EXIST);
	
	buffer[0] = onu_setid->new_onuid;
	memcpy(buffer + 1, onu_setid->onu_dsn, MAX_ONUDSN_LEN);
		
	if(dumpFifo)
		logger(LOG_DEBUG, "Set ONU ID, OldID:%d, new ID:%d!\n", onu_id, onu_setid->new_onuid);
	
	if(send_fifomsg(0xFF, CMD_FPGA_SETID, buffer, MAX_ONUDSN_LEN+1) != 0)
	{
		logger(LOG_ERR, "Set ONU IO message error !\n");
		return -1;
	}
	
	return 0;
}


/***********************************************************
*	setONUPortMirror: set the ONU node Port Mirror
*	param-in:  onu id
*	param-in:  onuPortMirrorConf
*	return:  	0  - success 
*			-1 - fail
***********************************************************/

int setONUPortMirror(unsigned char onu_id, ONU_PORTMIRROR_CONF *onuPortMirrorConf )
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];

	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;

	//fill the reg address data
	msg_type = TYPE_DEV_KS8995 | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, KS8995A, cmd=000*/

	if ( onuPortMirrorConf->setFlag == 0xFF )
	{
		buffer[0] = 0x11;
		buffer[1] = (onuPortMirrorConf->port1[0] << 7) | (onuPortMirrorConf->port1[1] << 6) | (onuPortMirrorConf->port1[2] << 5) | 0x1F;
		buffer[2] = 0x21;
		buffer[3] = (onuPortMirrorConf->port2[0] << 7) | (onuPortMirrorConf->port2[1] << 6) | (onuPortMirrorConf->port2[2] << 5) | 0x1F;
		buffer[4] = 0x31;
		buffer[5] = (onuPortMirrorConf->port3[0] << 7) | (onuPortMirrorConf->port3[1] << 6) | (onuPortMirrorConf->port3[2] << 5) | 0x1F;
		buffer[6] = 0x41;
		buffer[7] = (onuPortMirrorConf->port4[0] << 7) | (onuPortMirrorConf->port4[1] << 6) | (onuPortMirrorConf->port4[2] << 5) | 0x1F;
		buffer[8] = 0x51;
		buffer[9] = (onuPortMirrorConf->port5[0] << 7) | (onuPortMirrorConf->port5[1] << 6) | (onuPortMirrorConf->port5[2] << 5) | 0x1F;
		buffer[10] = 0x03;
		buffer[11] = (onuPortMirrorConf->badframes << 7) | 0x04;
		buffer[12] = 0x05;
		buffer[13] = onuPortMirrorConf->sourceanddestination;
		buffer[14] = 0x00;
		buffer[15] = 0x00;
	}

	if(dumpFifo)
		logger(LOG_DEBUG, "Set ONU Port Mirror, %d!\n", onu_id);
	
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 16, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Set ONU Port Mirror FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}
	else
		return 0;
}


/***********************************************************
*	setONUQoS: set the ONU node QoS
*	param-in:  onu id
*	param-in:  onuQoSConf
*	return:  	0  - success 
*			-1 - fail
***********************************************************/

int setONUQoS(unsigned char onu_id, ONU_QoS_CONF *onuQoSConf)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	unsigned char read_buffer[FIFO_MSG_LEN];

	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;

	/* get QoS_ctrl*/
	read_buffer[0] = 0x04;
	read_buffer[1] = 0;
	read_buffer[2] = 0x05;
	read_buffer[3] = 0;
	read_buffer[4] = 0x10;		
	read_buffer[5] = 0;
	read_buffer[6] = 0x20;		
	read_buffer[7] = 0;
	read_buffer[8] = 0x30;		
	read_buffer[9] = 0;
	read_buffer[10] = 0x40;		
	read_buffer[11] = 0;
	read_buffer[12] = 0x50;		
	read_buffer[13] = 0;
	read_buffer[14] = 0;		
	read_buffer[15] = 0;
	msg_type = TYPE_DEV_KS8995;	/* reg read, fsa->onu*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, read_buffer, 16, read_buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get QoS_ctrl error !\n");
		return(ERROR_COMM_FIBER);
	}

	logger(LOG_DEBUG, "read_buffer[1], %x!\n", read_buffer[1]);
	logger(LOG_DEBUG, "read_buffer[3], %x!\n", read_buffer[3]);

	logger(LOG_DEBUG, "onuQoSConf->prioritySchemeSelect, %x!\n", onuQoSConf->prioritySchemeSelect);

	buffer[0] = 0x04;
	buffer[1] = read_buffer[1] | 0x01;
	buffer[2] = 0x05;
	if(onuQoSConf->prioritySchemeSelect == 0x00)
		buffer[3] = read_buffer[3] & 0xF3;
	else if(onuQoSConf->prioritySchemeSelect == 0x01)
		buffer[3] = (read_buffer[3] & 0xF3) | 0x04;
	else if(onuQoSConf->prioritySchemeSelect == 0x10)
		buffer[3] = (read_buffer[3] & 0xF3) | 0x08;
	else if(onuQoSConf->prioritySchemeSelect == 0x11)
		buffer[3] = read_buffer[3] | 0x0C;
	
	buffer[4] = 0x10;
	if(onuQoSConf->priorityEnable[0] == 0x01)
		buffer[5] = read_buffer[5] | 0x01;
	else
		buffer[5] = read_buffer[5] & 0xFE;
	buffer[6] = 0x20;
	if(onuQoSConf->priorityEnable[1] == 0x01)
		buffer[7] = read_buffer[7] | 0x01;
	else
		buffer[7] = read_buffer[7] & 0xFE;
	buffer[8] = 0x30;
	if(onuQoSConf->priorityEnable[2] == 0x01)
		buffer[9] = read_buffer[9] | 0x01;
	else
		buffer[9] = read_buffer[9] & 0xFE;
	buffer[10] = 0x40;
	if(onuQoSConf->priorityEnable[3] == 0x01)
		buffer[11] = read_buffer[11] | 0x01;
	else
		buffer[11] = read_buffer[11] & 0xFE;
	buffer[12] = 0x50;
	if(onuQoSConf->priorityEnable[4] == 0x01)
		buffer[13] = read_buffer[13] | 0x01;
	else
		buffer[13] = read_buffer[13] & 0xFE;

	buffer[14] = 0x00;
	buffer[15] = 0x00;
	
	//fill the reg address data
	msg_type = TYPE_DEV_KS8995 | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, KS8995A, cmd=000*/

	if(dumpFifo)
		logger(LOG_DEBUG, "Set ONU QoS, %d!\n", onu_id);

	logger(LOG_DEBUG, "After & | !\n");
	logger(LOG_DEBUG, "buffer[1], %x!\n", buffer[1]);
	logger(LOG_DEBUG, "buffer[3], %x!\n", buffer[3]);
	
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 16, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "set ONU FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}
	else
		return 0;
}


/***********************************************************
*	saveONUVLAN: save the ONU node VLAN
*	param-in:  onu id
*	param-in:  onuVLANSave
*	return:  	0  - success 
*			-1 - fail
***********************************************************/

int saveONUVLAN(unsigned char onu_id, ONU_VLAN_CONF *onuVLANSave)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	unsigned char read_buffer[FIFO_MSG_LEN];

	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;

	/* get 802.1Q VLAN Enable or disable*/
	read_buffer[0] = 0x05;
	read_buffer[1] = 0;
	read_buffer[2] = 0;
	read_buffer[3] = 0;
	msg_type = TYPE_DEV_KS8995;	/* reg read, fsa->onu*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, read_buffer, 4, read_buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "FIFO: Get reg 0x05 error !\n");
		return(ERROR_COMM_FIBER);
	}
	else
	{
		logger(LOG_DEBUG, "reg 0x05:, %x!\n", read_buffer[1]);
	}

	buffer[0] = 0x76;
	buffer[1] = (onuVLANSave->membership & 0x1F) | 0x20;
	buffer[2] = 0x77;
	buffer[3] = (onuVLANSave->id << 4) | ((ntohs(onuVLANSave->vlan_id) >> 8) & 0x0F);
	buffer[4] = 0x78;
	buffer[5] = ntohs(onuVLANSave->vlan_id) & 0xFF;
	buffer[6] = 0x6E;
	buffer[7] = 0x04;
	buffer[8] = 0x6F;
	buffer[9] = onuVLANSave->id;
	buffer[10] = 0x05;
	buffer[11] = read_buffer[1] | 0x80;
	buffer[12] = 0x00;
	buffer[13] = 0x00;
	
	//fill the reg address data
	msg_type = TYPE_DEV_KS8995 | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, KS8995A, cmd=000*/

	if(dumpFifo)
		logger(LOG_DEBUG, "Save ONU VLAN, %d!\n", onu_id);

	logger(LOG_DEBUG, "After & | !\n");
	logger(LOG_DEBUG, "buffer[1], %x!\n", buffer[1]);
	logger(LOG_DEBUG, "buffer[3], %x!\n", buffer[3]);
	logger(LOG_DEBUG, "buffer[5], %x!\n", buffer[5]);
	logger(LOG_DEBUG, "buffer[9], %x!\n", buffer[9]);
	
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 14, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "set ONU FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}
	else
		return 0;
	
}

/***********************************************************
*	delONUVLAN: delete the ONU node VLAN table
*	param-in:  onu id
*	param-in:  onuVLANDel
*	return:  	0  - success 
*			-1 - fail
***********************************************************/

int delONUVLAN(unsigned char onu_id, ONU_VLAN_CONF *onuVLANDel)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	unsigned char read_buffer[FIFO_MSG_LEN];
	
	if( onu_id == 0|| onu_id >MAX_ONU_NUM)
		return -1;
	
		/* get 802.1Q VLAN Enable or disable*/
		read_buffer[0] = 0x05;
		read_buffer[1] = 0;
		read_buffer[2] = 0;
		read_buffer[3] = 0;
		msg_type = TYPE_DEV_KS8995; /* reg read, fsa->onu*/
		if( send_fifomsg_waitrsp(onu_id, msg_type, read_buffer, 4, read_buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			logger(LOG_ERR, "FIFO: Get reg 0x05 error !\n");
			return(ERROR_COMM_FIBER);
		}
		else
		{
			logger(LOG_DEBUG, "reg 0x05:, %x!\n", read_buffer[1]);
		}
	
		buffer[0] = 0x76;
		buffer[1] = onuVLANDel->membership & 0x1F;
		buffer[2] = 0x77;
		buffer[3] = (onuVLANDel->id << 4) | ((ntohs(onuVLANDel->vlan_id) >> 8) & 0x0F);
		buffer[4] = 0x78;
		buffer[5] = ntohs(onuVLANDel->vlan_id) & 0xFF;
		buffer[6] = 0x6E;
		buffer[7] = 0x04;
		buffer[8] = 0x6F;
		buffer[9] = onuVLANDel->id;
		buffer[10] = 0x05;
		buffer[11] = read_buffer[1] | 0x80;
		buffer[12] = 0x00;
		buffer[13] = 0x00;
		
		//fill the reg address data
		msg_type = TYPE_DEV_KS8995 | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, KS8995A, cmd=000*/
	
		if(dumpFifo)
			logger(LOG_DEBUG, "Delete ONU VLAN Table, %d!\n", onu_id);
	
		logger(LOG_DEBUG, "After & | !\n");
		logger(LOG_DEBUG, "buffer[1], %x!\n", buffer[1]);
		logger(LOG_DEBUG, "buffer[3], %x!\n", buffer[3]);
		logger(LOG_DEBUG, "buffer[5], %x!\n", buffer[5]);
		logger(LOG_DEBUG, "buffer[9], %x!\n", buffer[9]);
		
		if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 14, buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			logger(LOG_ERR, "set ONU FIFO message error !\n");
			return(ERROR_COMM_FIBER);
		}
		else
			return 0;
		
	}



/***********************************************************
*	setONUBW: set the ONU node bandwidth
*	param-in:  onu id
*	param-in:  index 1~4 ONU FE port
*	param-in:  bw 
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUBW(unsigned char onu_id, unsigned char index, unsigned short bw_send, unsigned short bw_recv)
{
	int regindex;
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	unsigned char reg_offset;

	if( index == 0 || index>4 || onu_id >MAX_ONU_NUM)
		return -1;

	reg_offset = 0x10 * (index-1);
	
	regindex= 0;
	buffer[regindex++] = 0x1B + reg_offset;

	if(bw_send >= 0x0C80) /*100M*/
	{
		if(bw_recv >= 0x0C80)
			buffer[regindex++] = 0; /*disable send&recv bandwidth control*/
		else
		{
			/*enable recv bandwidth control*/
			buffer[regindex++] = 0x78; 
			buffer[regindex++] = 0x18 + reg_offset;		
			buffer[regindex++] = bw_recv & 0xFF;
			buffer[regindex++] = 0x19 + reg_offset;		
			buffer[regindex++] = bw_recv & 0xFF;
			buffer[regindex++] = 0x1A + reg_offset;		
			buffer[regindex++] =  ((bw_recv >> 8 ) & 0x0F) | ((bw_recv >> 4 ) & 0xF0);
		}
	} else {
		if(bw_recv >= 0x0C80)
		{
			/*enable send bandwidth control*/
			buffer[regindex++] = 0x03; 
			buffer[regindex++] = 0x15 + reg_offset;		
			buffer[regindex++] = bw_send & 0xFF;
			buffer[regindex++] = 0x16 + reg_offset;		
			buffer[regindex++] = bw_send & 0xFF;
			buffer[regindex++] = 0x17 + reg_offset;		
			buffer[regindex++] =  ((bw_send >> 8 ) & 0x0F) | ((bw_send >> 4 ) & 0xF0);
		} else {
			/* enable send&recv bandwidth control*/
			buffer[regindex++] = 0x7B; 
			buffer[regindex++] = 0x15 + reg_offset;		
			buffer[regindex++] = bw_send & 0xFF;
			buffer[regindex++] = 0x16 + reg_offset;		
			buffer[regindex++] = bw_send & 0xFF;
			buffer[regindex++] = 0x17 + reg_offset;		
			buffer[regindex++] =  ((bw_send >> 8 ) & 0x0F) | ((bw_send >> 4 ) & 0xF0);
			buffer[regindex++] = 0x18 + reg_offset;		
			buffer[regindex++] = bw_recv & 0xFF;
			buffer[regindex++] = 0x19 + reg_offset;		
			buffer[regindex++] = bw_recv & 0xFF;
			buffer[regindex++] = 0x1A + reg_offset;		
			buffer[regindex++] =  ((bw_recv >> 8 ) & 0x0F) | ((bw_recv >> 4 ) & 0xF0);
		}
	}

	buffer[regindex++] = 0;		
	buffer[regindex++] = 0;
	
	msg_type = TYPE_DEV_KS8995|TYPE_MASK_WRITE; /* reg read, fsa->onu, cmd=002*/

	if(dumpFifo)
		logger(LOG_DEBUG, "Set ONU port bandwidth, %d-%d, recv=%d, send=%d!\n", onu_id, index, bw_recv, bw_send);

	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Set ONU FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}

	return 0;
}

int buildFEPortSetCmd(unsigned char index, unsigned char * buffer, ETHER_PORT_SET* port_set, unsigned char reg_0x12, unsigned char reg_0x1B )
{
	int regindex=0;
	unsigned char reg_offset;
	unsigned char bw_ctrl;

	reg_offset = 0x10*index;
	
	/*set the bandwidth control*/
	bw_ctrl = 0;
	if(port_set->setFlag & MD_ETHERPORT_BWMALLOCSEND) 
	{
		bw_ctrl = 1;
		if (port_set->bw_alloc_send < 0x0C80)
		{
			reg_0x1B |= 0x03;
			buffer[regindex++] = 0x15 + reg_offset;		
			buffer[regindex++] = port_set->bw_alloc_send & 0xFF;
			buffer[regindex++] = 0x16 + reg_offset;		
			buffer[regindex++] = port_set->bw_alloc_send & 0xFF;
			buffer[regindex++] = 0x17 + reg_offset;		
			buffer[regindex++] =  ((port_set->bw_alloc_send >> 8 ) & 0x0F) | ((port_set->bw_alloc_send >> 4 ) & 0xF0);
		} 
		else
			reg_0x1B &= 0xF8 ; /*disable send bw ctrl*/
	}

	if(port_set->setFlag & MD_ETHERPORT_BWMALLOCRECV)
	{
		bw_ctrl = 1;
		if(port_set->bw_alloc_recv < 0x0C80)
		{
			reg_0x1B |= 0x78;
			buffer[regindex++] = 0x18 + reg_offset;		
			buffer[regindex++] = port_set->bw_alloc_recv & 0xFF;
			buffer[regindex++] = 0x19 + reg_offset;		
			buffer[regindex++] = port_set->bw_alloc_recv & 0xFF;
			buffer[regindex++] = 0x1A + reg_offset;		
			buffer[regindex++] =  ((port_set->bw_alloc_recv >> 8 ) & 0x0F) | ((port_set->bw_alloc_recv >> 4 ) & 0xF0);
		}
		else
			reg_0x1B &= 0x07 ; /*disable recv bw ctrl*/
	}

	if(bw_ctrl)
	{
		buffer[regindex++] = 0x1B + reg_offset;
		buffer[regindex++] = reg_0x1B; 
	}

	/*set the admin*/
	if(port_set->setFlag & MD_ETHERPORT_ADMIN) 
	{
		if( port_set->admin_state == 1)
			reg_0x12 |= 0x06;		/*enabled*/
		else 
			reg_0x12 &= 0xF9;	 /*disabled*/
	}

	/*set work mode */
	if(port_set->setFlag & MD_ETHERPORT_WORKINGMODE) 
	{
		switch(port_set->portWorkingMode)
		{
			case 1:		//auto(1)
				buffer[regindex++] = 0x1C + reg_offset;
				buffer[regindex++] = 0x7F;
				break;
			case 2:		//m100-full(2)
				buffer[regindex++] = 0x1C + reg_offset;
				buffer[regindex++] = 0xF8;
				break;
			case 3:		//m100-half(3)
				buffer[regindex++] = 0x1C + reg_offset;
				buffer[regindex++] = 0xD4;
				break;
			case 4:		//m10-full(4)
				buffer[regindex++] = 0x1C + reg_offset;
				buffer[regindex++] = 0xB2;
				break;
			case 5:		//m10-half(5)
				buffer[regindex++] = 0x1C + reg_offset;
				buffer[regindex++] = 0x91;
				break;
			default:	
				logger(LOG_WARNING, "Unknow port working mode:%d !\n", port_set->portWorkingMode);
				break;
		}
	}

	/*set learning mode*/
	if(port_set->setFlag & MD_ETHERPORT_MODE) 
	{
		int i;
		if(port_set->portMode == 1)
		{
			/*learning mode*/
			reg_0x12  &= 0xFE;
			
			/* set admin and learning mode*/
			buffer[regindex++] = 0x12 + reg_offset;
			buffer[regindex++] = reg_0x12;

			/* remove the static MAC table*/
			buffer[regindex++] = 0x71;
			buffer[regindex++] = 0;
			buffer[regindex++] = 0x72;
			buffer[regindex++] = 0;
			for(i=0x73; i<=0x78; i++)
			{
				buffer[regindex++] = i;
				buffer[regindex++] = 0;
			}
			buffer[regindex++] = 0x6E;
			buffer[regindex++] = 0;
			buffer[regindex++] = 0x6F;
			buffer[regindex++] = index;
		} else {
			/* mac binding*/
			int dir_port;
			
			reg_0x12  |= 0x01;
		
			/* set admin and learning mode*/
			buffer[regindex++] = 0x12 + reg_offset;
			buffer[regindex++] = reg_0x12;

			/* add the static MAC table*/
			dir_port = 0x01;
			dir_port <<= index;
			
			buffer[regindex++] = 0x71;
			buffer[regindex++] = 0;
			buffer[regindex++] = 0x72;
			buffer[regindex++] = dir_port |0x20;
			for(i=0; i<6; i++)
			{
				buffer[regindex++] = 0x73 + i;
				buffer[regindex++] = port_set->bounded_mac[i];
			}
			buffer[regindex++] = 0x6E;
			buffer[regindex++] = 0;
			buffer[regindex++] = 0x6F;
			buffer[regindex++] = index;

		} 
	}

	
	buffer[regindex++] = 0;		
	buffer[regindex++] = 0;

	return(regindex);

}

/***********************************************************
*	setONUFEPort: set the ONU node FE port
*	param-in:  onu id
*	param-in:  index 0~3 ONU FE port
*	param-in:  ETHER_PORT_SET* 
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int setONUFEPort(unsigned char onu_id, unsigned char index, ETHER_PORT_SET* port_set)
{
	int cmd_len;
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	unsigned char reg_offset;
	unsigned char reg_0x12;
	unsigned char reg_0x1B;

	if( index>3 || onu_id >MAX_ONU_NUM)
		return -1;

	reg_offset = 0x10 * index;

	/* read the reg first */
	buffer[0] = 0x12 + reg_offset;
	buffer[1] = 0;	
	buffer[2] = 0x1B + reg_offset;		
	buffer[3] = 0;
	buffer[4] = 0;		
	buffer[5] = 0;
	
	msg_type = TYPE_DEV_KS8995; 
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, 6, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Read ONU FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}

	reg_0x12 = buffer[1];
	reg_0x1B = buffer[3];

	if(dumpFifo)
		logger(LOG_DEBUG, "Got ONU 8995 Reg, %d-%d, reg_0x12=%02x, reg_0x1B=%02x\n", onu_id, index, reg_0x12, reg_0x1B);

	port_set->bw_alloc_send = ntohs(port_set->bw_alloc_send);
	port_set->bw_alloc_recv = ntohs(port_set->bw_alloc_recv);
	
	 cmd_len = buildFEPortSetCmd(index, buffer, port_set, reg_0x12, reg_0x1B);
	
	if(dumpFifo)
		logger(LOG_DEBUG, "Set ONU port, %d-%d\n", onu_id, index);
	
	msg_type = TYPE_DEV_KS8995|TYPE_MASK_WRITE; /* reg read, fsa->onu, cmd=002*/
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, cmd_len, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Set ONU FIFO message error !\n");
		return(ERROR_COMM_FIBER);
	}

	updateONUPortCfgInDB(ONUDsn[onu_id], index, port_set);

	return 0;
}

void initSetONUFEPort(unsigned char onu_id, unsigned char index, ETHER_PORT_SET* port_set)
{
	int cmd_len;
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];
	unsigned char reg_0x12;
	unsigned char reg_0x1B;

	if( index>3 || onu_id >MAX_ONU_NUM)
		return;

	reg_0x12 = 0x06;
	reg_0x1B = 0;

	port_set->setFlag = 0xFF;
	
	cmd_len = buildFEPortSetCmd(index, buffer, port_set, reg_0x12, reg_0x1B);
	
	if(dumpFifo)
		logger(LOG_DEBUG, "Initial Set ONU port, %d-%d. bwSnd:%d,bwRcv:%d\n", onu_id, index, port_set->bw_alloc_send, port_set->bw_alloc_recv);
	
	msg_type = TYPE_DEV_KS8995|TYPE_MASK_WRITE; /* reg read, fsa->onu, cmd=002*/
	send_fifomsg(onu_id, msg_type, buffer, cmd_len);

}

void initSetONUUART(unsigned char onu_id, ONU_UART_CONF* uart_set)
{
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];

#ifdef DAT_CARD
	if( uart_set->port >= 5 || onu_id >MAX_ONU_NUM)
		return;
#else
	if( uart_set->port >= 2 || onu_id >MAX_ONU_NUM)
		return;
#endif

	buffer[0] = uart_set->baudrate& 0x07;
	buffer[1] = uart_set->function;
	buffer[2] = uart_set->time;
#ifdef DAT_CARD
	//buffer[3] = 0x03;
	buffer[3] = 0x00;
#else
	buffer[3] = 0x00;
#endif
	
	switch(uart_set->port)
	{
		case 0:
			break;
		case 1:
			buffer[0] |= 0x40;
			break;			
		case 2:
			buffer[0] |= 0x80;
			break;			
		case 3:
			buffer[0] |= 0x90;
			break;			
		case 4:
			buffer[0] |= 0xa0;
			break;			
	}	

	if(dumpFifo)
		logger(LOG_DEBUG, "Initial Set ONU UART, %d-%d. boud:%d, time:%d, func:%d\n", onu_id, uart_set->port, uart_set->baudrate, uart_set->time, uart_set->function);
	
	msg_type = CMD_FPGA_DATA | TYPE_MASK_WRITE ; /* reg write,  fsa->onu, fpga, cmd=110*/
	send_fifomsg(onu_id, msg_type, buffer, 4);

}

int readPerfReg(unsigned char onu_id, unsigned char regaddr, unsigned long *data)
{
	int regindex;
	unsigned char msg_type;
	unsigned char buffer[FIFO_MSG_LEN];

	/*Indirect Reg acess, write Reg 0x6E, 0x6F first */ 
	regindex= 0;
	buffer[regindex++] = 0x6E;
	buffer[regindex++] = 0x1C;
	buffer[regindex++] = 0x6F;		
	buffer[regindex++] = regaddr;
	buffer[regindex++] = 0;		
	buffer[regindex++] = 0;
			
	msg_type = TYPE_DEV_KS8995 | TYPE_MASK_WRITE; 
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Get ONU Perf error!write reg. ONU id=%d, regaddr=%02x\n", onu_id, regaddr);
		return(ERROR_COMM_FIBER);
	}

	/* read Reg from  */
	regindex= 0;
	buffer[regindex++] = 0x75;
	buffer[regindex++] = 0;		
	buffer[regindex++] = 0x76;
	buffer[regindex++] = 0;
	buffer[regindex++] = 0x77;
	buffer[regindex++] = 0;
	buffer[regindex++] = 0x78;
	buffer[regindex++] = 0;
			
	msg_type = TYPE_DEV_KS8995; 
	if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
	{
		logger(LOG_ERR, "Get ONU Perf error!read reg. ONU id=%d, regaddr=%02x\n", onu_id, regaddr);
		return(ERROR_COMM_FIBER);
	}


	if((buffer[1] & 0x40) == 0)
	{
		/* bit 30 = 0, re-read the register */
		logger(LOG_DEBUG, "Get ONU Perf restart! ONU id=%d, regaddr=%02x\n", onu_id, regaddr);

		regindex= 0;
		buffer[regindex++] = 0x75;
		buffer[regindex++] = 0;		
		buffer[regindex++] = 0x76;
		buffer[regindex++] = 0;
		buffer[regindex++] = 0x77;
		buffer[regindex++] = 0;
		buffer[regindex++] = 0x78;
		buffer[regindex++] = 0;
				
		msg_type = TYPE_DEV_KS8995; 
		if( send_fifomsg_waitrsp(onu_id, msg_type, buffer, regindex, buffer, FIFO_MSG_TIMEOUT) !=0 )
		{
			logger(LOG_ERR, "Get ONU Perf error!Re-read reg. ONU id=%d, regaddr=%02x\n", onu_id, regaddr);
			return(ERROR_COMM_FIBER);
		}

		if((buffer[1] & 0x40) == 0)
		{
			logger(LOG_ERR, "Get ONU Perf error!Re-read failed! ONU id=%d, regaddr=%02x\n", onu_id, regaddr);
			return -1;
		}

	}

	*data = 0x3F & buffer[1];
	*data = ((*data) << 8) + buffer[3];
	*data = ((*data) << 8) + buffer[5];
	*data = ((*data) << 8) + buffer[7];
	
	if(buffer[1] & 0x80)
	{
		logger(LOG_DEBUG, "Get ONU Perf overflow! ONU id=%d, regaddr=%02x\n", onu_id, regaddr);
		*data = *data |0x40;
	}	

	logger(LOG_DEBUG, "Get Perf reg data! ONU id=%d, regaddr=%02x, data=%d\n", onu_id, regaddr, *data);
	
	return 0;
}

int getPortPerf(unsigned char node_id, int index, PORT_PERF* perf)
{
	int memDataIndex;
	unsigned long reg_data, perfPrev;
	struct ip18_rw_struct reg, regvalue1,regvalue2;
	int offset;
	
	if( index>3 || node_id >MAX_ONU_NUM)
		return -1;

	//geport perf.
	if ( 0 == node_id )//fsa itself.
	{	
		index = index + 1;
		reg.reg = 0x01;
		if (0 !=  ip18_read_reg(&reg))
			return -1;

		reg.reg= 0x01;
		reg.value = reg.value |(1<<10);//enable.
		reg.value &= ~(1 << 9);//set mode
		if ( 0 != ip18_write_reg(&reg))
			return -1;
		logger (LOG_DEBUG, "set reg 01's value is %x\n", reg.value );

		//set mode.
		reg.reg = 0x3C;
		if ( 0 != ip18_read_reg(&reg))
			return -1;
		
		reg.reg = 0x3C;
		offset = 7 + index;
		reg.value &= ~(1 << offset );
		if ( 0 != ip18_write_reg(&reg))
			return -1;
		logger (LOG_DEBUG, "set reg 3C's value is %x\n", reg.value );

		//0x38register read address and enable.
		reg.reg = 0x38;
		if ( 1 == index )
			reg.value = 0x132;//port24(ge port 1)transimit counter.
		else
			reg.value = 0x134;//port25(ge port 2)transimit counter.

		if ( 0 != ip18_write_reg(&reg))
			return -1;
			
		regvalue1.reg = 0x39;//high
		regvalue2.reg = 0x3A;//low
		if ( (0 != ip18_read_reg(&regvalue1))  || (0 != ip18_read_reg(&regvalue2)))
			return -1;
		logger (LOG_DEBUG, "read reg 39 and 3A 's value are %x, %x\n", regvalue1.value, regvalue2.value);
		perf->perfTxByteHi = regvalue1.value;//fix left shift.
		perf->perfTxByteLo = regvalue2.value;
		logger (LOG_DEBUG, "ge port %d 's statics counter transimit high %d --transimit low %d\n",index, perf->perfTxByteHi, perf->perfTxByteLo);
		//0x38register read address and enable.
		reg.reg = 0x38;
		if ( 1 == index )
			reg.value = 0x133;//port24(ge port 1)receive counter.
		else
			reg.value = 0x135;//port25(ge port 2)receive counter.

		if ( 0 != ip18_write_reg(&reg))
			return -1;
			
		regvalue1.reg = 0x39;//high
		regvalue2.reg = 0x3A;//low
		if ( (0 != ip18_read_reg(&regvalue1))  || (0 != ip18_read_reg(&regvalue2)))
			return -1;
		logger (LOG_DEBUG, "read reg 39 and 3A 's value are %x, %x\n", regvalue1.value, regvalue2.value);
		perf->perfRxByteHi = regvalue1.value;//fix left shift.
		perf->perfRxByteLo = regvalue2.value;
		logger (LOG_DEBUG, "ge port %d 's statics counter receive high %d --receive low %d\n",index, perf->perfRxByteHi, perf->perfRxByteLo);
		logger (LOG_DEBUG, "ge port %d 's statics counter transimit %d --receive %d\n ", index , perf->perfTxByteHi + perf->perfTxByteLo ,perf->perfRxByteHi + perf->perfRxByteLo );
		return 0;
	}
	
	memDataIndex = (node_id -1)*4 + index;

	/* read the Rx lo-priority*/
	if(readPerfReg(node_id, index*0x20, &reg_data) != 0)
		return -1;

	perfPrev = perfData[memDataIndex].perfRxByteLo;
	perfData[memDataIndex].perfRxByteLo += reg_data;
	if(perfPrev > perfData[memDataIndex].perfRxByteLo) /*low 32 bits overflow*/
		perfData[memDataIndex].perfRxByteHi++;

	/* read the Rx hi-priority*/
	if(readPerfReg(node_id, index*0x20 + 0x01, &reg_data) != 0 )
		return -1;

	perfPrev = perfData[memDataIndex].perfRxByteLo;
	perfData[memDataIndex].perfRxByteLo += reg_data;
	if(perfPrev > perfData[memDataIndex].perfRxByteLo) /*low 32 bits overflow*/
		perfData[memDataIndex].perfRxByteHi++;
		
	/* read the Tx lo-priority*/
	if(readPerfReg(node_id, index*0x20 + 0x14, &reg_data) != 0 )
		return -1;

	perfPrev = perfData[memDataIndex].perfTxByteLo;
	perfData[memDataIndex].perfTxByteLo += reg_data;
	if(perfPrev > perfData[memDataIndex].perfTxByteLo) /*low 32 bits overflow*/
		perfData[memDataIndex].perfTxByteHi++;
	
	/* read the Tx hi-priority*/
	if(readPerfReg(node_id, index*0x20 + 0x14, &reg_data) != 0 )
		return -1;

	perfPrev = perfData[memDataIndex].perfTxByteLo;
	perfData[memDataIndex].perfTxByteLo += reg_data;
	if(perfPrev > perfData[memDataIndex].perfTxByteLo) /*low 32 bits overflow*/
		perfData[memDataIndex].perfTxByteHi++;

	logger(LOG_DEBUG, "Get Perf data! ONU=%d, port=%d, cache_index=%d, rxhi=%d, rxlo=%d, txhi=%d, txlo=%d\n", 
		node_id, index, memDataIndex, perfData[memDataIndex].perfRxByteHi, perfData[memDataIndex].perfRxByteLo, perfData[memDataIndex].perfTxByteHi, perfData[memDataIndex].perfTxByteLo);

	perf->perfRxByteHi = htonl(perfData[memDataIndex].perfRxByteHi);
	perf->perfRxByteLo = htonl(perfData[memDataIndex].perfRxByteLo);
	perf->perfTxByteHi = htonl(perfData[memDataIndex].perfTxByteHi);
	perf->perfTxByteLo = htonl(perfData[memDataIndex].perfTxByteLo);
	
	return 0;
}

int getPortCount( PORT_COUNT *portcount )
{
	int i;
	struct ip18_rw_struct reg, regvalue1,regvalue2;
	//unsigned char index = statisticcount[0].countType - 1;
	//enable and set mode 
	reg.reg = 0x01;
	reg.value = reg.value |(1<<10);//enable.
	if ( (3 == statisticcount.countType ) || (4 == statisticcount.countType) )
		reg.value = reg.value | (1 << 9);//set mode
	else 
		reg.value &= ~(1 << 9);//set mode
		
	if ( 0 != ip18_write_reg(&reg))
		return -1;
	logger (LOG_DEBUG, "set reg 01's value is %x\n", reg.value );

	reg.reg = 0x3B;
	regvalue1.reg = 0x3C;
	if ((2 == statisticcount.countType) || (4 == statisticcount.countType ))
	{
		reg.value = 0xFFFF;
		regvalue1.value = 0xFFFF;
	}
	else
	{
		reg.value = 0x00;
		regvalue1.value = 0x00;
	}
	if ((0 != ip18_write_reg(&reg)) || (0 != ip18_write_reg(&regvalue1)) )
		return -1;
	//portcount->countType = 1;
	portcount->countType = statisticcount.countType ;
	
	for ( i = 0; i <= 52; i = i+2 )//port0 ~ port26
	{
		if ( 48 == i )//skip port26.
			continue;
		
		//counter1 transimit
		reg.reg = 0x38;
		reg.value = 0x100 | i;
		if ( 0 != ip18_write_reg(&reg))
			return -1;
		regvalue1.reg = 0x39;//high
		regvalue2.reg = 0x3A;//low
		if ( (0 != ip18_read_reg(&regvalue1))  || (0 != ip18_read_reg(&regvalue2)))
			return -1;
		logger (LOG_DEBUG, "reg 39 and 3A 's value is %d--%d\n", regvalue1.value, regvalue2.value );
		if ( (50 == i ) || ( 52 == i ) )//geport1 or geport2
		{
			//tmp = regvalue1.value<<16;
			//tmp |= regvalue2.value;
			portcount->portCount1[(i/2) - 1] = (regvalue1.value<<16) |regvalue2.value ;
			logger (LOG_DEBUG, "port count1 value is %u\n", portcount->portCount1[(i/2) - 1]);
			logger(LOG_DEBUG, "before change statistic count value is %u\n", statisticcount.portCount1[(i/2) - 1]);
			statisticcount.portCount1[(i/2) - 1] += portcount->portCount1[(i/2) - 1];
			logger (LOG_DEBUG, "after  change statistic count value is %u\n ", statisticcount.portCount1[(i/2) - 1]);
			portcount->portCount1[(i/2) - 1] = htonl(statisticcount.portCount1[(i/2) - 1]);
			
		}
		else
		{
			//tmp = regvalue1.value<<16;
			//tmp |= regvalue2.value;
			portcount->portCount1[i/2] = (regvalue1.value<<16) |regvalue2.value;
			logger (LOG_DEBUG, "portcount1 value is %u\n", portcount->portCount1[i/2] );
			logger(LOG_DEBUG, "before change statistic count value is %u\n", statisticcount.portCount1[i/2]);
			statisticcount.portCount1[i/2] += portcount->portCount1[i/2];
			logger (LOG_DEBUG, "after  change statistic count value is %u\n ", statisticcount.portCount1[i/2]);
			portcount->portCount1[i/2] = htonl(statisticcount.portCount1[i/2]);
			
		}
		
		//counter2 receive
		reg.reg = 0x38;
		reg.value = 0x100 | (i + 1);
		if ( 0 != ip18_write_reg(&reg))
			return -1;
		regvalue1.reg = 0x39;//high
		regvalue2.reg = 0x3A;//low
		if ( (0 != ip18_read_reg(&regvalue1))  || (0 != ip18_read_reg(&regvalue2)))
			return -1;
		logger (LOG_DEBUG, "reg 39 and 3A 's value is %d--%d\n", regvalue1.value, regvalue2.value );
		if ( (50 == i ) || ( 52 == i ) )//geport1 or geport2
		{
			//tmp = regvalue1.value<<16;
			//tmp |= regvalue2.value;
			portcount->portCount2[i/2 - 1] = (regvalue1.value<<16) |regvalue2.value;
			logger (LOG_DEBUG, "port count2 value is %u\n", portcount->portCount2[i/2 - 1]);
			logger(LOG_DEBUG, "before change statistic count value is %u\n", statisticcount.portCount2[i/2 -1]);
			statisticcount.portCount2[i/2 -1] += portcount->portCount2[i/2 - 1];
			logger (LOG_DEBUG, "after  change statistic count value is %u\n ", statisticcount.portCount2[i/2 -1]);
			portcount->portCount2[i/2 -1 ] = htonl(statisticcount.portCount2[i/2 -1]);
		}
		else
		{
			//tmp = regvalue1.value<<16;
			//tmp |= regvalue2.value;
			portcount->portCount2[i/2] = (regvalue1.value<<16) |regvalue2.value;
			logger (LOG_DEBUG, "port count2 value is %u\n", portcount->portCount2[i/2]);
			logger(LOG_DEBUG, "before change statistic count value is %u\n", statisticcount.portCount2[i/2]);
			statisticcount.portCount2[i/2] += portcount->portCount2[i/2];
			logger (LOG_DEBUG, "after  change statistic count value is %u\n ", statisticcount.portCount2[i/2]);
			portcount->portCount2[i/2] = htonl(statisticcount.portCount2[i/2]);
		}
		
	}

	return 0;

}

int resetPortCount ()
{
	int i;
	struct ip18_rw_struct reg, regvalue1,regvalue2;
	unsigned char tmp_type = statisticcount.countType;
	//unsigned char index = statisticcount[0].countType - 1;
	//enable and set mode 
	reg.reg = 0x01;
	reg.value = reg.value |(1<<10);//enable.
	//reg.value &= ~(1 << 9);//set mode
	if ( (3 == statisticcount.countType ) || (4 == statisticcount.countType) )
		reg.value = reg.value | (1 << 9);//set mode
	else 
		reg.value &= ~(1 << 9);//set mode
		
	if ( 0 != ip18_write_reg(&reg))
		return -1;
	logger (LOG_DEBUG, "set reg 01's value is %x\n", reg.value );

	reg.reg = 0x3B;
	regvalue1.reg = 0x3C;
	if ((2 == statisticcount.countType) || (4 == statisticcount.countType ))
	{
		reg.value = 0xFFFF;
		regvalue1.value = 0xFFFF;
	}
	else
	{
		reg.value = 0x00;
		regvalue1.value = 0x00;
	}
	
	if ((0 != ip18_write_reg(&reg)) || (0 != ip18_write_reg(&regvalue1)) )
		return -1;

	for ( i = 0; i <= 52; i = i+2 )//port0 ~ port26
	{
		if ( 48 == i )//skip port26.
			continue;
		
		//counter1 transimit
		reg.reg = 0x38;
		reg.value = 0x180 | i;
		if ( 0 != ip18_write_reg(&reg))
			return -1;
		regvalue1.reg = 0x39;//high
		regvalue2.reg = 0x3A;//low
		if ( (0 != ip18_read_reg(&regvalue1))  || (0 != ip18_read_reg(&regvalue2)))
			return -1;
		logger (LOG_DEBUG, "reg 39 and 3A 's value is %d--%d\n", regvalue1.value, regvalue2.value );

		
		//counter2 receive
		reg.reg = 0x38;
		reg.value = 0x100 | (i + 1);
		if ( 0 != ip18_write_reg(&reg))
			return -1;
		regvalue1.reg = 0x39;//high
		regvalue2.reg = 0x3A;//low
		if ( (0 != ip18_read_reg(&regvalue1))  || (0 != ip18_read_reg(&regvalue2)))
			return -1;
		logger (LOG_DEBUG, "reg 39 and 3A 's value is %d--%d\n", regvalue1.value, regvalue2.value );

		
	}
	

	memset (&statisticcount, 0 , sizeof(PORT_COUNT));//clear.
	statisticcount.countType = tmp_type;
	return 0;


}

int resetPortPerf(unsigned char node_id, int index )
{
	int memDataIndex;
	PORT_PERF 	temp_perf;
	
	if( index>3 || node_id >MAX_ONU_NUM)
		return -1;

	/* read to clear */
	getPortPerf(node_id, index, &temp_perf);

	memDataIndex = (node_id -1)*4 + index;

	perfData[memDataIndex].perfRxByteHi = 0;
	perfData[memDataIndex].perfRxByteLo = 0;
	perfData[memDataIndex].perfTxByteHi = 0;
	perfData[memDataIndex].perfTxByteLo = 0;
	logger(LOG_DEBUG, "Clear Perf data! ONU=%d, port=%d, cache_index=%d, rxhi=%d, rxlo=%d, txhi=%d, txlo=%d\n", 
		node_id, index, memDataIndex, perfData[memDataIndex].perfRxByteHi, perfData[memDataIndex].perfRxByteLo, perfData[memDataIndex].perfTxByteHi, perfData[memDataIndex].perfTxByteLo);

	return 0;
}



/***********************************************************
*	resetONU: reset the ONU node
*	return:  	0  - success 
*			-1 - fail
***********************************************************/
int resetONU(unsigned char onu_id)
{
	return(send_fifomsg(onu_id, CMD_FPGA_RESET, NULL, 0));
}


/***********************************************************
*	getAlarm: get the  FSA and ONU current alarm information
*	param-out:  ONET_ALARM_INFO* , the first pointer of entry buffer that hold the result
*	param-in:  max_len, the maximun number of alarms that  ONET_ALARM_INFO* can hold
*	return:  	-1 - fail
*			0 - no alarm
*			other - the number alarms
***********************************************************/
int getAlarm(ONET_ALARM_INFO* alarmlist, int max_len)
{
	int alarmcount=0;
	
	if(alarm_wait_flag)
		return(0);
	
	pthread_mutex_lock(&mtx_alarmqueue);
	while((alarmcount <= max_len ) && (alarm_queue_tail != alarm_queue_head))
	{
		memcpy(alarmlist + alarmcount, alarm_queue + alarm_queue_tail, sizeof(ONET_ALARM_INFO));
		alarmcount++;
		alarm_queue_tail++;
		if(alarm_queue_tail >= MAX_ALARM_QUEUE_LEN)
			alarm_queue_tail = 0;
		reduce_flag = 0;
	}
	pthread_mutex_unlock(&mtx_alarmqueue);

	if(alarmcount != 0 )
		logger(LOG_DEBUG, "Get alarm count:%d\n", alarmcount);
	
	return(alarmcount);
}



/***********************************************************
*	sendRSData: send  serial data to the ONU RS232 port
*	param-in:  onu id
*	param-in:  data, the raw data need to be sent
*	param-in:  data_len, the lenth of sending data
*	return:  	0  - success 
*			-1 - fail
********************************************************
* 	this function is similar with send_fifomsg()
***********************************************************/
int sendRSData(unsigned char onu_id, unsigned char port, char * data, int data_len)
{
	int ret;
	int sent_len, len, flowctrl_count;
	unsigned char buffer[FIFO_MSG_LEN];
	FIFO_MSG_HEAD *header;
	
	header = (FIFO_MSG_HEAD*)buffer;

	memset(buffer, 0, FIFO_MSG_LEN);
	memcpy(header, fifoMsgStartFlag, 4);
	header->dst = onu_id;
	switch(port) {
		case 0:
			header->type = CMD_DATA_PASSTHROUGH;
			break;
		case 1:
			header->type = CMD_DATA_PORT2;
			break;
		case 2:
			header->type = CMD_DATA_PORT3;
			break;
		case 3:
			header->type = CMD_DATA_PORT4;
			break;
		case 4:
			header->type = CMD_DATA_PORT5;
			break;
		default:
			return(-1);
	}
	header->type |= TYPE_DEV_DATAP;
	header->src = 0; /*message is from olt*/
	
	sent_len = 0;
	flowctrl_count=0;
	while(data_len > sent_len && data != NULL)
	{
#ifndef DAT_CARD
		flowctrl_count++;
		if((flowctrl_count == 11)||(flowctrl_count==21))
		{
//			logger(LOG_DEBUG, "Write FIFO Sleep\n");
			usleep(10000);
		}
#endif		
		if((data_len-sent_len) > MAX_DATAP_LEN)
			len = MAX_DATAP_LEN;
		else
			len = data_len - sent_len;

		buffer[sizeof(FIFO_MSG_HEAD)] = len;	
		memcpy(buffer + sizeof(FIFO_MSG_HEAD) + 1, data+sent_len, len);
		sent_len += len;
		
		pthread_mutex_lock(&mtx_dev);
		ret = write(fdFpga, buffer, FIFO_MSG_LEN); 
	       pthread_mutex_unlock(&mtx_dev);
		if(ret < 0) {
			logger(LOG_ERR, "Write FIFO error\n");
			return -1;
		}
		
		if(dumpFifo && rstcplog)
		{
			struct timeval checkpoint;

			gettimeofday(&checkpoint, NULL);
			logger(LOG_DEBUG, "%d-RS: Len=%d, dst=%d\n", checkpoint.tv_usec/1000, len, header->dst);
			if(dumpFifo >1)
				DumpMsg(buffer, FIFO_MSG_LEN);
		}

		/* debug  
		if(data_len>0 && data != NULL)
		{
			static unsigned char last_byte;
			static unsigned int last_len;

			if(last_len != data_len)
				logger(LOG_ERR, "FIFO: RS SND LEN=0x%0x\n", data_len);

			last_len = data_len;

			last_byte++;
			if(last_byte != data[0])
				logger(LOG_ERR, "FIFO: RS RCVD=0x%0x, EXP=0x%0x\n", data[0], last_byte);
				
			last_byte = data[data_len -1];
		}
	 	*/
		// count++;
	}
	return 0;	

}


int getIOReport(ONU_IO_REPORT* iolist, int max_len)
{
	int count=0;
	pthread_mutex_lock(&mtx_ioqueue);
	while((count <= max_len ) && (ioreport_queue_tail != ioreport_queue_head))
	{
		memcpy(iolist + count, ioreport_queue + ioreport_queue_tail, sizeof(ONU_IO_REPORT));
		count++;
		ioreport_queue_tail++;
		if(ioreport_queue_tail >= MAX_IOREP_QUEUE_LEN)
			ioreport_queue_tail = 0;
	}
	pthread_mutex_unlock(&mtx_ioqueue);
	return(count);
}

/*****************************************************************
* setPortClockDelay() parameter: 
* bit 7: set flag
* bit 6, 5, 4: 1- set clock delay for port group3, 2, 1; 0-no clock delay       
****************************************************************/
int setPortClockDelay(int cfg_val)
{
	struct ip18_rw_struct reg1826;
	
	if(cfg_val&0x80)
	{
		reg1826.reg = 0xFA;
		reg1826.value = 0x4100;

		if(cfg_val&0x10)
			reg1826.value |= 0x01;
		if(cfg_val&0x20)
			reg1826.value |= 0x04;
		if(cfg_val&0x40)
			reg1826.value |= 0x10;
		
		return(ip18_write_reg( &reg1826));
	}

	return 0;
}



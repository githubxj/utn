
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <termios.h> /* POSIX terminal control definitions */
#include <fcntl.h>   /* File control definitions */
#include <pthread.h>
#include <sys/socket.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/logger.h"
#include "../common/iniFile.h"
#include "syscall_wrapper.h"
#include "proc_alarm.h"

#define MAX_IOLIST_LEN		16
#define PORT_7000 7000  /*listening port*/

static int _ser=MIN_ALM_SER;
static unsigned int almf=1;

AlarmParam Alarm_Config[2];
unsigned char OnuIOState[MAX_ONU_NUM + 1];
unsigned char OnuIOStateN[MAX_ONU_NUM + 1]; // new io state
unsigned char OnuIOMask[MAX_ONU_NUM + 1];

unsigned int ChgCount[MAX_ONU_NUM + 1]; 

unsigned short alarm_statistic[MAX_ONU_NUM + 1][6];

AlarmQueue AlarmList[2];

extern int getIOReport(ONU_IO_REPORT* iolist, int max_len);

void get_io_statistics(FSA_LOWSPD_STATE *fsa_lowspd_state, unsigned char onu_id)
{
	int i;

	for(i=0; i<6; i++)
	{
		fsa_lowspd_state->alarm_dev_id[i] = htons(Alarm_Config[0].devid + onu_id + i*Alarm_Config[0].devid_step);
		fsa_lowspd_state->alarm_count[i] = htons(alarm_statistic[onu_id][i]);
	}

}

void reset_io_statistics()
{
	int i, j;

	for(i=0; i<=MAX_ONU_NUM; i++)
	{
		for(j=0; j<6; j++)
			alarm_statistic[i][j] = 0;
	}
}


int getAlarmParam(AlarmParam *param, int index)
{
	char section[16];
	
	if(NULL == param)
		return -1;
	
	if(0 != access(IO_CONF_FILE, F_OK))
    		return -1;

	IniFilePtr pIni = iniFileOpen(IO_CONF_FILE);
	if(NULL == pIni)
		return -1;

	if(index == 0)
		sprintf(section, "ALARM");
	else
		sprintf(section, "ALARM2");
	
	memset(param, 0, sizeof(AlarmParam));
	const char *str;
	if(NULL != (str = iniGetStr(pIni, section, "SERVERIP")))
    		strncpy(param->svrip, str, 16);
	else
		return -1;
 
	param->svrport = iniGetInt(pIni, section, "SERVERPORT");
	str = iniGetStr(pIni, section, "INSET1");
	if(str == NULL)
		param->io_flag1 = 0;
	else
		sscanf(str, "%x", &(param->io_flag1));
	str = iniGetStr(pIni, section, "INSET2");
	if(str == NULL)
		param->io_flag2 = 0;
	else
		sscanf(str, "%x", &(param->io_flag2));
	str = iniGetStr(pIni, section, "INSET3");
	if(str == NULL)
		param->io_flag3 = 0;
	else
		sscanf(str, "%x", &(param->io_flag3));
	str = iniGetStr(pIni, section, "INSET4");
	if(str == NULL)
		param->io_flag4 = 0;
	else
		sscanf(str, "%x", &(param->io_flag4));
	str = iniGetStr(pIni, section, "INSET5");
	if(str == NULL)
		param->io_flag5 = 0;
	else
		sscanf(str, "%x", &(param->io_flag5));
	str = iniGetStr(pIni, section, "INSET6");
	if(str == NULL)
		param->io_flag6 = 0;
	else
		sscanf(str, "%x", &(param->io_flag6));
	param->io_mode= iniGetInt(pIni, section, "MODE");
	param->rpt_format = iniGetInt(pIni, section, "FORMAT");

	param->devid = iniGetInt(pIni, section, "DEVID");
	param->devid_step = iniGetInt(pIni, section, "DEVID_STEP");
	if(param->devid_step <= 0)
		param->devid_step = MAX_ONU_NUM;
	param->nodeid = iniGetInt(pIni, section, "NODEID");
	param->alarm_lev= iniGetInt(pIni, section, "ALMLEV");
	param->area= iniGetInt(pIni, section, "AREA");
	if(param->area <= 0)
		param->area = 21;
	if(param->area > 255)
		param->area = 255;

	iniFileFree(pIni);
	return 0;
}

//recv from net(port 7000),send to alarm_out
void* recv_alarm_7000(void *data)
{
    char buf[1];
    int size;
    int connector_handler;
    int acceptor_handler;
    connector_handler = acceptor_open(PORT_7000);  //create a server 
    while(1)
    {
        acceptor_handler = acceptor_get(connector_handler);//waitting client  connect
 //       fprintf(stderr, "alarm connect accepted!\n");
        if(acceptor_handler == -1)
            continue;

        memset(buf, 0 , 1);
        if((size = recv(acceptor_handler, buf, sizeof(buf), 0)) < 0) {
            fprintf(stderr, "error! alarm connect closed!\n");
            close(acceptor_handler);
            continue;
        }

        fprintf(stdout, "Alarm Recieved: %x\n",buf[0]);

	//to do 

        close(acceptor_handler);
        fprintf(stderr, "alarm connect closed!\n");
    }
    close(connector_handler);
    return NULL;
}

//recv from alarm_in,send to net(port 7000)
//read extern alarm
//read motion detection
//read video lost

int send_alarmdata(int socket_fd,  unsigned short alarm_type, unsigned int dev_id, int rpt_format)
{
	char * buff;
	int send_len;
	int trytimes = 3;
	int r, n = 0;
	AlarmToMu almmu;
	AlarmShort almshort;

	//printf("send alarm, format=%d\n", rpt_format);
	if(rpt_format)
	{	
		//Covond Alarm format
		struct tm * currentime;
		time_t time_now;

		time_now = time(NULL);
		currentime = localtime(&time_now);
		memset(&almmu , 0 , sizeof(AlarmToMu));
		memcpy(almmu.start_flag, startflag, 16);
		almmu.version =1;
		almmu.serno=htonl(_ser);
		 _ser++;
		if(_ser>=MAX_ALM_SER)
			_ser=MIN_ALM_SER;

		almmu.msgtype=htons(13);
		almmu.subtype=htons(3);
		almmu.msglenth=htonl(288);
		almmu.result=htonl(0);
		almmu.nodeId=htonl(0);

		almmu.almtype=htons(alarm_type);
		almmu.devId=htonl(dev_id);

		almmu.almId=htonl(almf);
		almf++;
		almmu.almRank=htonl(0);
		almmu.almdescriptionId=htons(0);
		almmu.year=htons(currentime->tm_year + 1900);
		almmu.almtime[1]=htons(currentime->tm_mon + 1);
		almmu.almtime[3]=htons(currentime->tm_mday);
		almmu.almtime[5]=htons(currentime->tm_hour);
		almmu.almtime[7]=htons(currentime->tm_min);
		almmu.almtime[9]= htons(currentime->tm_sec);
		//printf("...........devid=%d, type=%d, date:%d-%d-%d %d:%d:%d\n",dev_id, alarm_type, currentime->tm_year + 1900,currentime->tm_mon+1,currentime->tm_mday,currentime->tm_hour,currentime->tm_min,currentime->tm_sec);
		memset(almmu.des, 0, 256);

		buff = (char *) &almmu;
		send_len = sizeof(AlarmToMu);
		logger(LOG_DEBUG, "Send ONU IO Report - len:%d, Devid:%03d, Type:%d\n", send_len, dev_id, alarm_type);
	} else {
		//short alarm format
		char tmp[16];		
		if(dev_id >999)
			dev_id = 999;
		sprintf(tmp, "%03d", dev_id);
		
		almshort.start_flag = '$';
		if(alarm_type == ALARM_TYPE_IO)
			almshort.msg_type = 'A';
		else
			almshort.msg_type = 'R';
		almshort.lb = '(';
		memcpy(almshort.id, tmp, 3);
		almshort.rb = ')';
		almshort.stop_flag = '#';
		
		buff = (char *) &almshort;
		send_len = sizeof(AlarmShort);
		logger(LOG_DEBUG, "Send ONU IO Report - len:%d, Devid:%03d, Type:%c\n", send_len, dev_id, almshort.msg_type);
	}

	while(trytimes>0 && n<send_len)
	{
		r = send(socket_fd, buff+n, send_len-n, MSG_NOSIGNAL);

		if((-1 == r && EAGAIN == errno) || 0 == r)//send again? max to 3 times
		{
			usleep(1000);
			trytimes --;
		}
		else if(-1 == r)
		{
			//send error
			return -1;
		}
		else// if(r+n < sizeof(almData[i].data))//send again
		{
			n += r;
		}
	}
	
	return 0;

}

int check_alarm()
{
	int i, j, idx, ret;
	unsigned short alarm_type;

	ret = 0;	
	for(i=1; i<=MAX_ONU_NUM; i++)
	{
		if(OnuIOState[i] == OnuIOStateN[i])
		{
			ChgCount[i] = 0;
		} else {
			ChgCount[i]++;
			if(ChgCount[i] > ALARM_THRESHOLD)
			{
				// the io change has last 100ms, the alarm is come up
				for(idx=0; idx<6; idx++)
				{
					if(((OnuIOState[i] >> idx) & 1) == ((OnuIOStateN[i] >> idx) & 1))
						continue;
					
					if((OnuIOStateN[i] >> idx) & 1)
					{
						alarm_statistic[i][idx]++;
						alarm_type = ALARM_TYPE_IO;
					}
					else
						alarm_type = ALARM_TYPE_CLEAR;
				
				        //add alarm list
					for(j=0; j<2; j++)
					{
						AlarmList[j].queue[AlarmList[j].tail].type = alarm_type;
						AlarmList[j].queue[AlarmList[j].tail].devid = Alarm_Config[0].devid + i + idx*Alarm_Config[0].devid_step;
						AlarmList[j].queue[AlarmList[j].tail].format = Alarm_Config[j].rpt_format;
						if((AlarmList[j].tail - AlarmList[j].head) == 255)
						{
							logger(LOG_ERR, "IO: Alarm list full! Con-%d, ONU-%d IO-%d!\n", j, i, idx+1);
							AlarmList[j].head++; //discard the oldest
						}
						AlarmList[j].tail++;
					}
					
					ret = 1;
				}

				ChgCount[i] = 0;
				OnuIOState[i] = OnuIOStateN[i];
			}
		}		
	 }

	return(ret);
}

void* alarm2Net()
{
	char encIP[16];
	unsigned short port;

	ONU_IO_REPORT iostat_list[MAX_IOLIST_LEN];
	int iostat_list_no;
	
	int snd_tcpsocket[2] = {-1, -1};
	int tag_tcpsocket[2] = {0, 0};
	unsigned int mask;
	int loop_count = CON_IDLE;
	int alarm_sent;

	int i;
	fd_set ofds;
	int maxfd = 0;
	struct timeval tv;

	int policy;
	struct sched_param param;

	if(pthread_getschedparam(pthread_self(), &policy, &param)==0)
		printf("THD:ounio, policy:%d, priority:%d\n", policy, param.sched_priority);

	logger(LOG_INFO, "ONU IO Report Thread created: %d\n", getpid());

	if(getAlarmParam(&Alarm_Config[0], 0) !=0)
	{
		logger(LOG_ERR, "No ONU IO Report server configured.\n");
		return ;
	}
	getAlarmParam(&Alarm_Config[1], 1);

	mask = 1;
	for(i=1; i <= MAX_ONU_NUM; i++)
	{
		if(mask & Alarm_Config[0].io_flag1)
			OnuIOMask[i] = 0x01;
		else
			OnuIOMask[i] = 0;

		if(mask & Alarm_Config[0].io_flag2)
			OnuIOMask[i] |= 0x02;
		if(mask & Alarm_Config[0].io_flag3)
			OnuIOMask[i] |= 0x04;
		if(mask & Alarm_Config[0].io_flag4)
			OnuIOMask[i] |= 0x08;
		if(mask & Alarm_Config[0].io_flag5)
			OnuIOMask[i] |= 0x10;
		if(mask & Alarm_Config[0].io_flag6)
			OnuIOMask[i] |= 0x20;

		OnuIOState[i] = 0;

//		printf("ONU%d mask=%x\n", i, OnuIOMask[i]);
		
		mask <<= 1;	
		ChgCount[i] == 0; 
	}

	for(i=0; i<2; i++)
	{
		AlarmList[i].head = 0;
		AlarmList[i].tail = 0;
	}

    while(1)
    {
		
        //update alarm state into memory
	do 
	{
	 	iostat_list_no  = getIOReport(iostat_list, MAX_IOLIST_LEN);

		 for(i=0; i< iostat_list_no; i++)
		 {
		 	if((iostat_list[i].node_id > MAX_ONU_NUM) || (iostat_list[i].node_id == 0))
			{
				logger(LOG_ERR, "IO: Invalide ONU id: %d\n", iostat_list[i].node_id);
				continue;
		 	}

			//逻辑电平转换
			if(Alarm_Config[0].io_mode == 0)
				iostat_list[i].io_state = ~iostat_list[i].io_state;
			
			OnuIOStateN[iostat_list[i].node_id] = iostat_list[i].io_state & OnuIOMask[iostat_list[i].node_id];
		 }
	}
	while (iostat_list_no == MAX_IOLIST_LEN);

	//scan the ONU io states
	if(check_alarm() == 0)
	{
		// no new alarm 
		loop_count++;
		if(loop_count> CON_IDLE)
		{
		 	//关闭发送tcp连接
			for(i=0; i<2; i++)
			{
			 	if(snd_tcpsocket[i] != -1) 
			 	{
					close(snd_tcpsocket[i]);    
					snd_tcpsocket[i] = -1;
	               		logger(LOG_INFO, "ALARMUPLOAD: Connection %d is idle and closed!\n", i);
			 	}
				tag_tcpsocket[i] = 0;

				//clear all the alarms that are not sent
				AlarmList[i].head = 0;
				AlarmList[i].tail = 0;
			}
			usleep(CHECK_INTERVAL);
		 	loop_count = CON_IDLE;
			continue;			
		}
	}
	else
		loop_count = 0;

	// check and start TCP connetction
	for(i=0; i<2; i++)
	{
		if(-1 == snd_tcpsocket[i] && tag_tcpsocket[i] <= 0)
		{
			if(Alarm_Config[i].svrip[0] == 0)
			{
				AlarmList[i].head = 0;
				AlarmList[i].tail = 0;
				continue;
			}
			
			strncpy(encIP, Alarm_Config[i].svrip, 16);
			port = Alarm_Config[i].svrport;
			logger(LOG_INFO, "ALARMUPLOAD: Try to connect remote server %s:%d\n", encIP, port);
			snd_tcpsocket[i] = opentcpclient(port, encIP, 0);
			tag_tcpsocket[i] = TAG_CHKPOINT;
		}
	}

	// check TCP connection ready to send data
	FD_ZERO(&ofds);
	tv.tv_sec  = 0;
	tv.tv_usec = CHECK_INTERVAL;
	maxfd = 0;
	for(i=0; i<2; i++)
	{
		if(-1 != snd_tcpsocket[i] && tag_tcpsocket[i] != 1) 
        	{
            		FD_SET(snd_tcpsocket[i], &ofds);
	     		if(snd_tcpsocket[i] > maxfd)
            			maxfd = snd_tcpsocket[i];
        	}
        }

        int r = select(maxfd+1, NULL, &ofds, NULL, &tv);
        if(r < 0)
        {
		logger(LOG_ERR,"ALARMUPLOAD: select error!");
		for(i=0; i<2; i++)
		{
			close(snd_tcpsocket[i]);    
			snd_tcpsocket[i] = -1;
			tag_tcpsocket[i] = 0;

			//clear all the alarms that are not sent
			AlarmList[i].head = 0;
			AlarmList[i].tail = 0;
		}	

		usleep(CHECK_INTERVAL);
		continue;
        }
        else if(0 != r)
        {
        	for(i=0; i<2; i++)
        	{
	            if(snd_tcpsocket[i] != -1 && FD_ISSET(snd_tcpsocket[i], &ofds))
	            {
	                if(TAG_CHKPOINT <= tag_tcpsocket[i] && tag_tcpsocket[i] <= TAG_TIMEDOUT)//从未连接到已连接
	                {
	                    int p = 0;
	                    socklen_t len = sizeof(int);
	                    r = getsockopt(snd_tcpsocket[i], SOL_SOCKET, SO_ERROR,  (char *)&p, &len);
	                    if(0 > r || p != 0)//connect failed
	                    {
					logger(LOG_INFO, "ALARMUPLOAD: Connect to remote server %s:%d failed!\n", Alarm_Config[i].svrip, Alarm_Config[i].svrport);
					close(snd_tcpsocket[i]);
					snd_tcpsocket[i] = -1;
					tag_tcpsocket[i] = TAG_WAITTIME; 
					
					//clear all the alarms that are not sent
					AlarmList[i].head = 0;
					AlarmList[i].tail = 0;
	                    }
	                    else
	                    {
	                        tag_tcpsocket[i] = 1;//已连接
	                        logger(LOG_INFO, "ALARMUPLOAD: Connect to remote server %s:%d ok...!\n", Alarm_Config[i].svrip, Alarm_Config[i].svrport);
	                    }
	                }
	                else
	                {
	                    tag_tcpsocket[i] = 1;//可写
	                    logger(LOG_INFO, "ALARMUPLOAD: Ready to write to remote server %s:%d ok!\n", Alarm_Config[i].svrip, Alarm_Config[i].svrport);
	                }
	            }
        	}
        }

        //check upload socket status, update wait timer
        for(i=0; i<2; i++)
        {
	        if(-1 != snd_tcpsocket[i])
	        {
			if(TAG_TIMEDOUT <= tag_tcpsocket[i])
			{
				close(snd_tcpsocket[i]);     //关闭发送tcp连接
				snd_tcpsocket[i] = -1;
				logger(LOG_INFO, "ALARMUPLOAD: Connect to remote server %s:%d timeout...!\n", Alarm_Config[i].svrip, Alarm_Config[i].svrport);
				tag_tcpsocket[i] = TAG_WAITTIME;

				//clear all the alarms that are not sent
				AlarmList[i].head = 0;
				AlarmList[i].tail = 0;
			}
			else if(tag_tcpsocket[i] >= TAG_CHKPOINT)
	                	tag_tcpsocket[i] ++;
	       }
		else if(tag_tcpsocket[i] > 0)
	       {
	        	//waiting for the next connect operation
			tag_tcpsocket[i] --;
	       }
        }

	//start to send the alarms
	alarm_sent = 0;
        for(i=0; i<2; i++)
        {
         	if((tag_tcpsocket[i] != 1) || (snd_tcpsocket[i] == -1))
			continue;
			
	      	while(AlarmList[i].tail != AlarmList[i].head)
        	{
			if(-1 == send_alarmdata(snd_tcpsocket[i],
				  AlarmList[i].queue[AlarmList[i].head].type, 
				  AlarmList[i].queue[AlarmList[i].head].devid, 
				  AlarmList[i].queue[AlarmList[i].head].format))
			{
				//错误，关闭socket
				close(snd_tcpsocket[i]);
				snd_tcpsocket[i] = -1;
				tag_tcpsocket[i] = 0;
				logger(LOG_ERR, "IO: Send Alarm DEVID-%d failed!\n", AlarmList[i].queue[AlarmList[i].head].devid);
				break;
			} else {
				AlarmList[i].head++;
				alarm_sent = 1;
			}
        	}
        }	

	if((alarm_sent == 0) && (tv.tv_usec > 0))
		usleep(tv.tv_usec);

    }

    return NULL;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/stat.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/alarm_type.h"
#include "alarm_proc.h"
#include "message_proc.h"
#include "../common/serial_comm.h"
#include "../common/logger.h"
#include "../common/iniFile.h"
#include "refresh_check.h"
#include "sw_ctrl.h"

extern int fd485;
unsigned char fan_state=0, temperature=0, power_state=0;
unsigned char fan_control=0;
unsigned char fan_states[3];
unsigned char flag_fail[3];
SHELF_INFO	shelfCfg;
int 	check_fanfail_delay=0;

int getShelfParam(SHELF_INFO *param)
{
	char file[64];
	struct stat conf_stat;

	snprintf(file, 64, "%s", SHELF_CONFIG);

	/* check config file exist first */
	if(stat(file, &conf_stat) !=0)
		return -1;
	
	IniFilePtr pIni = iniFileOpen(file);
	if(NULL == pIni)
		return -1;

	if(NULL == param)
	{
		iniFileFree(pIni);
		return -1;
	}

	memset(param, 0, sizeof(SHELF_INFO));
	param->fan_config= iniGetInt(pIni, "FAN", "FAN_CNTL");
	param->t_threshod_low[0] = iniGetInt(pIni, "FAN", "FAN1_TL");
	param->t_threshod_high[0] = iniGetInt(pIni, "FAN", "FAN1_TH");
	param->t_threshod_low[1] = iniGetInt(pIni, "FAN", "FAN2_TL");
	param->t_threshod_high[1] = iniGetInt(pIni, "FAN", "FAN2_TH");
	param->t_threshod_low[2] = iniGetInt(pIni, "FAN", "FAN3_TL");
	param->t_threshod_high[2] = iniGetInt(pIni, "FAN", "FAN3_TH");
	
	iniFileFree(pIni);
	return 0;
}

int saveShelfParam(SHELF_INFO *param)
{
	char file[64];
	snprintf(file, 64, "%s", SHELF_CONFIG);

	IniFilePtr pIni = iniFileOpen(file);
	if(NULL == pIni)
		return -1;

	iniSetInt(pIni, "FAN", "FAN_CNTL", param->fan_config);
	iniSetInt(pIni, "FAN", "FAN1_TL", param->t_threshod_low[0]);
	iniSetInt(pIni, "FAN", "FAN1_TH", param->t_threshod_high[0]);
	iniSetInt(pIni, "FAN", "FAN2_TL", param->t_threshod_low[1]);
	iniSetInt(pIni, "FAN", "FAN2_TH", param->t_threshod_high[1]);
	iniSetInt(pIni, "FAN", "FAN3_TL", param->t_threshod_low[2]);
	iniSetInt(pIni, "FAN", "FAN3_TH", param->t_threshod_high[2]);

	iniFileSave(pIni);
	iniFileFree(pIni);
	return 0;
}


/******control fan ********/
int fan_ctl(unsigned char fanctrl)
{
	char     sendbuff[MAX_485MSG_FULLLEN];
	char     recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 *headReq,*headRsp;
	FAN_CTL       *fan_ctl;

printf("Send fan control:%d\n", fanctrl);

	memset(sendbuff,0,sizeof(MSG_HEAD_485));
	headReq=(MSG_HEAD_485*)sendbuff;
	headRsp=(MSG_HEAD_485*)recvbuff;
	
	fan_ctl=(FAN_CTL*)(sendbuff+sizeof(MSG_HEAD_485));
	fan_ctl->fan_state= fanctrl&0x07;
	headReq->code   = C_BACK_SET;
	headReq->dst    = BACK_CARDNO;
	headReq->src    = CTRL_CARDNO;
	headReq->length = sizeof(FAN_CTL);

	if(!msg485_sendwaitrsp(fd485,sendbuff,recvbuff,TIMEOUT_485MSG))
	{
		if(headRsp->rsp!=0)
		{
			logger(LOG_ERR,"Error:Set back error response,error:%d\n",headRsp->rsp);
			return (headRsp->rsp);
		}
		if(headRsp->length!=0)
		{
			logger(LOG_ERR,"Error:Set Back response length error,length:%d,expect:%d\n",headRsp->length,0);

			return ERROR_COMM_PARSE;
		}
		
		check_fanfail_delay = 5;
		return(0);
	}

	return(ERROR_COMM_485);
}

void initShelfConfig()
{
	int i;
	if(getShelfParam(&shelfCfg) != 0)
	{
		shelfCfg.fan_config = 8; //auto control

		//first fan threshold
		shelfCfg.t_threshod_low[0] = 33;
		shelfCfg.t_threshod_high[0] = 35;
		
		//second fan threshold
		shelfCfg.t_threshod_low[1] = 28;
		shelfCfg.t_threshod_high[1] =30 ;

		//third fan threshold
		shelfCfg.t_threshod_low[2] = 38 ;
		shelfCfg.t_threshod_high[2] = 40;
	}
	
	fan_control = shelfCfg.fan_config;

	// set fan control if config manual
	if((shelfCfg.fan_config&0x08) == 0)
		fan_ctl(shelfCfg.fan_config);

	for(i=0; i<3; i++)
	{
		fan_states[i] = FAN_OFF;
		flag_fail[i] = 0;
	}
	
}

int GetShelfInfo(SHELF_INFO *shelf)
{
	int i;
	
	shelf->power = power_state;
	shelf->temperature = temperature;
	for(i=0; i<3; i++)
	{
		shelf->fan_state[i] = fan_states[i];
		shelf->t_threshod_low[i] = shelfCfg.t_threshod_low[i];
		shelf->t_threshod_high[i] = shelfCfg.t_threshod_high[i];
		
	}
	shelf->fan_config = shelfCfg.fan_config;

	return 0;
}

int SetShelf(SHELF_INFO *shelf)
{
	int ret, i;

	if(shelf->fan_config & 0x08)
	{
		if(temperature >= shelf->t_threshod_high[0])
			fan_control |= 0x01;				
		if(temperature <= shelf->t_threshod_low[0])
			fan_control &= 0xFE;

		if(temperature >= shelf->t_threshod_high[1])
			fan_control |= 0x02;				
		if(temperature <= shelf->t_threshod_low[1])
			fan_control &= 0xFD;

		if(temperature >= shelf->t_threshod_high[2])
			fan_control |= 0x04;				
		if(temperature <= shelf->t_threshod_low[2])
			fan_control &= 0xFB;

		ret = fan_ctl(fan_control);	
	}
	else
		ret = fan_ctl(shelf->fan_config);
	
	if( ret == 0)
	{
		//update configuration data
		for(i=0; i<3; i++)
		{
			shelfCfg.t_threshod_low[i] = shelf->t_threshod_low[i];
			shelfCfg.t_threshod_high[i] = shelf->t_threshod_high[i];
		}
		shelfCfg.fan_config = shelf->fan_config;

		// update control state if manual
		if((shelf->fan_config & 0x08) == 0)
			fan_control = shelf->fan_config;
		
		saveShelfParam(&shelfCfg);		
	
	}
	return (ret);
}


void sendLCDNetwork()
{
	char file[64];
	struct stat conf_stat;
	IniFilePtr pIni;
	char * rslt;

	char 			sendbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq;
	LCD_NET			*param;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	param = (LCD_NET*)(sendbuff + sizeof(MSG_HEAD_485));
	
	snprintf(file, 64, "%s", IP_CONFIG);

	/* check config file exist first */
	if(stat(file, &conf_stat) !=0)
		return;
	
	pIni = iniFileOpen(file);
	if(NULL == pIni)
		return;

	rslt = iniGetStrByKey(pIni,  "IPADDR");
	if(rslt != 0)
		strncpy((char *)param->ipaddr, rslt, 16);
	else
		strcpy((char *)param->ipaddr, "0.0.0.0");

	rslt = iniGetStrByKey(pIni,  "NETMASK");
	if(rslt != 0)
		strncpy((char *)param->mask, rslt, 16);
	else
		strcpy((char *)param->mask, "0.0.0.0");

	rslt = iniGetStrByKey(pIni,  "GATEWAY");
	if(rslt != 0)
		strncpy((char *)param->gateway, rslt, 16);
	else
		strcpy((char *)param->gateway, "0.0.0.0");

	iniFileFree(pIni);


	headReq->code = C_LCD_NET;
	headReq->src = CTRL_CARDNO;
	headReq->dst = LCD_CARDNO;
	headReq->length = sizeof(LCD_NET);
		
	if(msg485_send(fd485, sendbuff, headReq->length+sizeof(MSG_HEAD_485)) == -1)
	{
		logger(LOG_ERR, "Send message failed\n");
	}
	
}

void sendLCDFan()
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq;
	LCD_FAN			*param;
	unsigned char 		state,conf;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	param = (LCD_FAN*)(sendbuff + sizeof(MSG_HEAD_485));

	sprintf((char *)param->temperature, " %02d", temperature );

	param->fan_states[0] = 0;
	param->fan_states[1] = 0;
	param->fan_states[2] = 0;
	
	if(fan_state & 0x01)
		param->fan_states[0] = 2;
	if(fan_state & 0x02)
		param->fan_states[1] = 2;
	if(fan_state & 0x04)
		param->fan_states[2] = 2;
	
	headReq->code = C_LCD_FAN;
	headReq->src = CTRL_CARDNO;
	headReq->dst = LCD_CARDNO;
	headReq->length = sizeof(LCD_FAN);
		
	if(msg485_send(fd485, sendbuff, headReq->length+sizeof(MSG_HEAD_485)) == -1)
	{
		logger(LOG_ERR, "Send message failed\n");
	}
	
}

void sendLCDPower()
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq;
	LCD_PWR			*param;
	unsigned char 		pow;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	param = (LCD_PWR*)(sendbuff + sizeof(MSG_HEAD_485));

	pow=power_state;
	param->pwr1 = 1;
	param->pwr2 = 1;
	if((pow&1)==0)
		param->pwr1=0;
	if(((pow>>1)&1)==0)
		param->pwr2=0;
	
	headReq->code = C_LCD_PWR;
	headReq->src = CTRL_CARDNO;
	headReq->dst = LCD_CARDNO;
	headReq->length = sizeof(LCD_PWR);
		
	if(msg485_send(fd485, sendbuff, headReq->length+sizeof(MSG_HEAD_485)) == -1)
	{
		logger(LOG_ERR, "Send message failed\n");
	}
	
}

/****get the information of back******/
int GetBack(BACK_INFO *backinfo)
{
	char  sendbuff[MAX_485MSG_FULLLEN];
	char  recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 *headRsp,*headReq;
	BACK_INFO	   *back_rsp;
	
	headReq=(MSG_HEAD_485*)sendbuff;
	headRsp=(MSG_HEAD_485*)recvbuff;
	headReq->code=C_BACK_GET;
	headReq->dst=BACK_CARDNO;
	headReq->src=CTRL_CARDNO;
	headReq->length=0;

	if(!msg485_sendwaitrsp(fd485,sendbuff,recvbuff,TIMEOUT_485MSG))
	{
		if(headRsp->rsp!=0)
		{
			logger(LOG_ERR,"Error:Get Back error response,error:%d\n",headRsp->rsp);
			return headRsp->rsp;
		}

		if(headRsp->length!=sizeof(BACK_INFO))
		{
			logger(LOG_ERR,"Error:Get Back response length error,length:%d,expect:%d\n",headRsp->length,sizeof(BACK_INFO));

			return ERROR_COMM_PARSE;
		}
		back_rsp = (BACK_INFO*)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(backinfo,back_rsp,sizeof(BACK_INFO));
		return 0;
	}
	return ERROR_COMM_485;
}


/* backplane check status routine */
void check_backstate()
{
	char state,temp,power;
	BACK_INFO   backinfo;
	int ret, i, mask, config_flag;

	ret = GetBack(&backinfo);
	if(ret != 0)
	{
		logger(LOG_WARNING,"Warning:Get Back error response in check backplane state loop,error:%d\n", ret);
		return;	
	}
	
	temp = backinfo.temperature;
	power = backinfo.power;
	
	state = 0;
	if(backinfo.fan_speed[0]>500)
		state |= 0x01;
	if(backinfo.fan_speed[1]>500)
		state |= 0x02;
	if(backinfo.fan_speed[2]>500)
		state |= 0x04;

	if((state != fan_state) || (temp != temperature) || (power != power_state))
	{
//		logger(LOG_DEBUG, "BACK State changed: pwr=%d, fan=%d, temp=%d; old:%d,%d,%d \n", power, state, temp, power_state, fan_state, temperature);
		printf("BACK State changed: pwr=%d, fan=%d, temp=%d; old:%d,%d,%d \n", power, state, temp, power_state, fan_state, temperature);
		printf("fan speed: 1-%d, 2-%d, 3-%d\n", backinfo.fan_speed[0], backinfo.fan_speed[1], backinfo.fan_speed[2]);

		//automatic fan control		
		if(temp != temperature)
		{
			if(shelfCfg.fan_config&0x08)
			{
				if(temp >= shelfCfg.t_threshod_high[0])
					fan_control |= 0x01;				
				if(temp <= shelfCfg.t_threshod_low[0])
					fan_control &= 0xFE;

				if(temp >= shelfCfg.t_threshod_high[1])
					fan_control |= 0x02;				
				if(temp <= shelfCfg.t_threshod_low[1])
					fan_control &= 0xFD;

				if(temp >= shelfCfg.t_threshod_high[2])
					fan_control |= 0x04;				
				if(temp <= shelfCfg.t_threshod_low[2])
					fan_control &= 0xFB;

				fan_ctl(fan_control);	
			}
		}
		
		//update fan state
		if(state != fan_state)
		{
			for(i=0; i<3; i++)
			{
				mask = 0x01 << i;
				if( (state&mask) != (fan_state&mask))
				{
					if(state&mask)
						fan_states[i] = FAN_ON;
					else
						fan_states[i] = FAN_OFF;
				}
			}
		}
		
		fan_state = state;
		temperature = temp;
		power_state = power;
		setRefreshState(CHANGE_FLAG_FRAMEV);
	}
	
	/***************************************
	/*    check fan fail
	/***************************************/
	if(check_fanfail_delay>0)
		check_fanfail_delay--;
	else
	{
		sqlite3 *db;  
		char *zErrMsg = 0;
		char sql_str[256];
		int rc;

		db = NULL;
		config_flag=0;
		for(i=0; i<3; i++)
		{
			if(fan_states[i] == FAN_FAIL)
				continue;

			mask = 0x01 << i;
			if((mask & fan_control) != 0)
			{
				if((mask & fan_state) == 0)
				{
					flag_fail[i]++;
					config_flag = 1;
					if(flag_fail[i] >1)
					{
						flag_fail[i] = 0;
						fan_states[i] = FAN_FAIL;
						setRefreshState(CHANGE_FLAG_FRAMEV);

						/* generate the alarm */
						if(db == NULL)
						{
							if( sqlite3_open(DYNAMIC_DB, &db) ){
								printf( "DB Error: Can't open database: %s\n", sqlite3_errmsg(db));
								continue;
							}
						}
						genAlarm(db, ATYPE_FANFAIL , getServerity(ATYPE_FANFAIL), CTRL_CARDNO, i+1, 0, 0);
					}
					
				}
			} else {
				/* if fan is configed OFF but checked ON, the MPU is restart, 
					it needs to resend the fan control command */
				if(mask & fan_state)
				{
					logger(LOG_WARNING,"Warning: fan is configed OFF but checked ON\n");
					config_flag = 1;
				}
			}
		}
		
		if( db != NULL )
			sqlite3_close(db);	

		if(config_flag)
		{
			fan_ctl(fan_control);	
		}
	}
	
}

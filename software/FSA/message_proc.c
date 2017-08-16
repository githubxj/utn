#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sqlite3.h>
#include "../common/const.h"
#include "../common/msg.h"
#include "../common/logger.h"
#include "../common/serial_comm.h"
#include "../common/iniFile.h"

#include "fsalib.h"
#include "refresh_check.h"
#include "message_proc.h"


#define	VERSION_HW0 	'0'
#define	VERSION_HW1 	'1'
#define	VERSION_FW0 	'0'
#define	VERSION_FW1 	'1'
#define	VERSION_SW0 	'2'
#ifdef DAT_CARD
#define	VERSION_SW1 	'5'
#else
#define	VERSION_SW1 	'5'
#endif

char 	OSversion[MAX_DISPLAYNAME_LEN];
char 	CardName[MAX_DISPLAYNAME_LEN];
unsigned char 	adminState = 1;
extern CARD_SET_FSA FSAConfig;
extern int alarmlog;
extern int dump485;
extern PORT_COUNT statisticcount;
extern unsigned long	ONUDsn[];
extern int rstcp_base_port;
extern int rstcp_port_step;


#define SAFE_ATOI(data)		(data?atoi(data):0)

extern void get_rs_statistics(FSA_LOWSPD_STATE *fsa_lowspd_state, unsigned char onu_id);
extern void reset_rs_statistics();
extern void get_io_statistics(FSA_LOWSPD_STATE *fsa_lowspd_state, unsigned char onu_id);
extern void reset_io_statistics();

int GetOSVersion()
{
	int filefd;
	char buffer[256];
	char *ptr;
	int i;

	filefd = open("/proc/version", O_RDONLY);
	if(filefd == -1) 
	{
		strcpy(OSversion, "Unknow");
		return 0;
	}

	memset(buffer, 0, 256);
	read(filefd, buffer, 256);
	buffer[255] = '\0';

	ptr = buffer;
	while(*ptr != '\0' && (*ptr < '0'  || *ptr > '9'))
		ptr++;

	for(i=0; i<15; i++)
	{
		OSversion[i] = ptr[i];
		if(ptr[i] == '\0' || ptr[i] == ' ')
			break;
	}

	OSversion[i] = '\0';
	
	close(filefd);

	return 0;

}

int GetCardInfo(CARD_INFO *	card_info)
{
	
	card_info->admin_state = adminState;
	memcpy(card_info->card_name, CardName, MAX_DISPLAYNAME_LEN);
#ifdef DAT_CARD
	card_info->card_type = CARD_TYPE_DAT;
#else
	card_info->card_type = CARD_TYPE_FSA;
#endif
	card_info->version_HW[0] = VERSION_HW0;
	card_info->version_HW[1] = VERSION_HW1;
	card_info->version_FW[0] = VERSION_FW0;
	card_info->version_FW[1] = VERSION_FW1;
	card_info->version_SW[0] = VERSION_SW0;
	card_info->version_SW[1] = VERSION_SW1;
	memcpy(card_info->version_OS, OSversion, MAX_DISPLAYNAME_LEN);
	return 0;	
}

int GetNetwork(FSA_NETWORK *fsa_net)
{
	char file[64];
	struct stat conf_stat;
	IniFilePtr pIni;
	char * rslt;

	memset(fsa_net, 0, sizeof(FSA_NETWORK));
	
	snprintf(file, 64, "%s", FSA_IP_CONFIG);

	/* check config file exist first */
	if(stat(file, &conf_stat) !=0)
		return(-1);
	
	pIni = iniFileOpen(file);
	if(NULL == pIni)
		return(-1);

	rslt = iniGetStrByKey(pIni,  "IPADDR");
	if(rslt != 0)
		strncpy(fsa_net->ip, rslt, 16);
	else
		strcpy(fsa_net->ip, "0.0.0.0");

	rslt = iniGetStrByKey(pIni,  "NETMASK");
	if(rslt != 0)
		strncpy(fsa_net->mask, rslt, 16);
	else
		strcpy(fsa_net->mask, "0.0.0.0");

	rslt = iniGetStrByKey(pIni,  "GATEWAY");
	if(rslt != 0)
		strncpy(fsa_net->gateway, rslt, 16);
	else
		strcpy(fsa_net->gateway, "0.0.0.0");

	iniFileFree(pIni);
	return 0;
}

int SetNetwork(FSA_NETWORK *fsa_net)
{
	char file[64];
	struct stat conf_stat;
	IniFilePtr pIni;
	char * rslt;
	struct ifreq ifr;
	int sockfd;			/* socket fd we use to manipulate stuff with */	
	int ret;
	struct in_addr ipaddr, mask, broadcast;
 	struct sockaddr_in saddr;
	char cmd_buff[64];

	fsa_net->ip[15]=0;
	fsa_net->mask[15]=0;
	fsa_net->gateway[15]=0;	

	snprintf(file, 64, "%s", FSA_IP_CONFIG);

	/* check config file exist first */
	if(stat(file, &conf_stat) !=0)
		return(-1);
	
	/* configure IP address live */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd == -1)
	{
		logger(LOG_ERR, "Socket creat error!\n");
		return(-1);
	}

	memset(&ifr, 0, sizeof(struct ifreq));
	sprintf(ifr.ifr_name, "eth0");
	if(inet_aton(fsa_net->ip, &ipaddr) == 0)
		return -1;

	if(inet_aton(fsa_net->mask,  &mask) == 0)
		return -1;


	// ifconfig ethx ip, mask and up
	ret = -1;
	saddr.sin_family = AF_INET;
	saddr.sin_addr = ipaddr;
	memcpy(&(ifr.ifr_addr), &saddr, sizeof(struct sockaddr_in));

	if(ioctl(sockfd, SIOCSIFADDR, &ifr) != -1) /* ip address */
	{
		saddr.sin_addr = mask;
		memcpy(&(ifr.ifr_addr), &saddr, sizeof(struct sockaddr_in));
		if(ioctl(sockfd, SIOCSIFNETMASK, &ifr) != -1) /* net mask */
		{
			/* set broadcast addr */
			broadcast.s_addr = ipaddr.s_addr | (~mask.s_addr);
			saddr.sin_addr = broadcast;
			memcpy(&(ifr.ifr_addr), &saddr, sizeof(struct sockaddr_in));
			ioctl(sockfd, SIOCSIFBRDADDR, &ifr); 
					
			/* bring the interface up*/
			if(ioctl(sockfd, SIOCGIFFLAGS, &ifr) != -1)
			{
				ifr.ifr_flags |= IFF_UP;		
				if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) != -1)
					ret = 0;	
			}
		}						
		else
			logger(LOG_ERR, "Set netmask error!\n");
	}
	else 
		logger(LOG_ERR, "Set ip addr error\n");
			
	close(sockfd);

	if(ret != 0)
		return(-1);

	/*set default route */
	system("/sbin/route del default");
	sprintf(cmd_buff, "/sbin/route add default gw %s", fsa_net->gateway);
	system(cmd_buff);
	
	pIni = iniFileOpen(file);
	if(NULL == pIni)
		return(-1);

	iniSetStr(pIni, NULL, "IPADDR", fsa_net->ip);
	iniSetStr(pIni, NULL, "NETMASK", fsa_net->mask);
	iniSetStr(pIni, NULL, "GATEWAY", fsa_net->gateway);

	iniFileSave(pIni);
	iniFileFree(pIni);

	return(0);


	
}

void GetLowspdState(FSA_LOWSPD_STATE *fsa_lowspd_state, unsigned char onu_id)
{
	memset(fsa_lowspd_state, 0, sizeof(FSA_LOWSPD_STATE));
	
	if(onu_id > MAX_ONU_NUM)
		return;
	
	get_rs_statistics(fsa_lowspd_state, onu_id);
	get_io_statistics(fsa_lowspd_state, onu_id);
}

void GetLowspdConfig(FSA_LOWSPD_CONFIG *fsa_lowspd_config)
{
	struct stat conf_stat;
	IniFilePtr pIni;
	char * rslt;
	int value, i;
	char key[16];

	memset(fsa_lowspd_config, 0, sizeof(FSA_LOWSPD_CONFIG));
	fsa_lowspd_config->rs_port = htons(rstcp_base_port);
	fsa_lowspd_config->rs_port_step = htons(rstcp_port_step);
		
	if(0 != access(IO_CONF_FILE, F_OK))
    		return;

	pIni = iniFileOpen(IO_CONF_FILE);
	if(NULL == pIni)
		return;
	
	value = iniGetInt(pIni, "SerialServer", "PORT");
	if(value >0)
		fsa_lowspd_config->rs_port = htons(value);
 
	value = iniGetInt(pIni, "SerialServer", "STEP");
	if(value >0)
		fsa_lowspd_config->rs_port_step = htons(value);
	
	
	if(NULL != (rslt = iniGetStr(pIni, "ALARM", "SERVERIP")))
    		strncpy(fsa_lowspd_config->as_ip, rslt, 16);
 
	fsa_lowspd_config->as_port = htons(iniGetInt(pIni, "ALARM", "SERVERPORT"));

	for(i=0; i<6; i++)
	{
		sprintf(key, "INSET%d", i+1);		
		rslt = iniGetStr(pIni, "ALARM", key);
		if(rslt == NULL)
			fsa_lowspd_config->alarm_mask[i] = 0;
		else
		{
			sscanf(rslt, "%x", &value);
			fsa_lowspd_config->alarm_mask[i] =htonl(value);
		}
	}

	fsa_lowspd_config->alarm_mode= iniGetInt(pIni, "ALARM", "MODE");
	fsa_lowspd_config->alarm_format = iniGetInt(pIni, "ALARM", "FORMAT");
	fsa_lowspd_config->dev_id = htons(iniGetInt(pIni, "ALARM", "DEVID"));
	fsa_lowspd_config->dev_id_step = htons(iniGetInt(pIni, "ALARM", "DEVID_STEP"));

	if(NULL != (rslt = iniGetStr(pIni, "ALARM2", "SERVERIP")))
    		strncpy(fsa_lowspd_config->as_ip2, rslt, 16);
 
	fsa_lowspd_config->as_port2 = htons(iniGetInt(pIni, "ALARM2", "SERVERPORT"));
	fsa_lowspd_config->alarm_format2 = iniGetInt(pIni, "ALARM2", "FORMAT");

	iniFileFree(pIni);
}

int SetLowspdConfig(FSA_LOWSPD_CONFIG *fsa_lowspd_config)
{
	IniFilePtr pIni;
	int i;
	char key[16];
	char value[16];

	fsa_lowspd_config->as_ip[15]=0;
	fsa_lowspd_config->as_ip2[15]=0;

	if(0 != access(IO_CONF_FILE, F_OK))
    		return(-1);

	pIni = iniFileOpen(IO_CONF_FILE);
	if(NULL == pIni)
		return(-1);

	iniSetInt(pIni, "SerialServer", "PORT", ntohs(fsa_lowspd_config->rs_port));
	iniSetInt(pIni, "SerialServer", "STEP", ntohs(fsa_lowspd_config->rs_port_step));
 
	iniSetStr(pIni, "ALARM", "SERVERIP", fsa_lowspd_config->as_ip);
 	iniSetInt(pIni, "ALARM", "SERVERPORT", ntohs(fsa_lowspd_config->as_port));
 	iniSetInt(pIni, "ALARM", "MODE", fsa_lowspd_config->alarm_mode);
 	iniSetInt(pIni, "ALARM", "FORMAT", fsa_lowspd_config->alarm_format);
 	iniSetInt(pIni, "ALARM", "DEVID", ntohs(fsa_lowspd_config->dev_id));
 	iniSetInt(pIni, "ALARM", "DEVID_STEP", ntohs(fsa_lowspd_config->dev_id_step));

	for(i=0; i<6; i++)
	{
		sprintf(key, "INSET%d", i+1);
		sprintf(value, "%06x", ntohl(fsa_lowspd_config->alarm_mask[i]));
		iniSetStr(pIni, "ALARM", key, value);
	}

	iniSetStr(pIni, "ALARM2", "SERVERIP", fsa_lowspd_config->as_ip2);
 	iniSetInt(pIni, "ALARM2", "SERVERPORT", ntohs(fsa_lowspd_config->as_port2));
 	iniSetInt(pIni, "ALARM2", "FORMAT", fsa_lowspd_config->alarm_format2);

	iniFileSave(pIni);
	iniFileFree(pIni);

	return(0);
	
}

int GetCardFsa(unsigned char card_no, FSA_CARD_STATE * card_state)
{
	return 0;	
}

int updatePortForSnmp(unsigned char card_no ,unsigned char portindex, ETHER_PORT_SET *etherport_set)
{
	sqlite3 *db;
	char *zErrMsg =0 ;
	int rc , nrow, ncol;
	char **result;
	char sql_str[256];
	char mac_str[256];

	rc = sqlite3_open(CONF_DB, &db);
	if (rc)
	{
		logger  (LOG_ERR ,   "can not open fsaconfiig database:%s\n", sqlite3_errmsg(db));
		return ERROR;
	}

	sprintf(sql_str, "select CardNo from FSACardPort where CardNo=%d and PortNo=%d ", card_no, portindex);
	rc = sqlite3_get_table(db,
					sql_str,
					&result,
					&nrow,
					&ncol,
					&zErrMsg);

	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR,  "select cardno err for updata etherport from snmp:%s\n%s\n", sql_str, zErrMsg);
		sqlite3_close(db);
		return ERROR;
	}
	if (nrow < 1)
	{
		sqlite3_free_table(result);
		if (etherport_set->setFlag & MD_ETHERPORT_ADMIN)
		{
			sprintf(sql_str, "insert into FSACardPort (CardNo ,PortNo, adminState ) values (%d,%d,%d)", card_no, portindex, etherport_set->admin_state);
		}
				
		if (etherport_set->setFlag & MD_ETHERPORT_MODE)
		{
			sprintf(sql_str, "insert into FSACardPort (CardNo ,PortNo, portMode  ) values (%d,%d,%d) ", card_no, portindex, etherport_set->portMode);
		}
		if (etherport_set->setFlag & MD_ETHERPORT_WORKINGMODE)
		{
			sprintf(sql_str, "insert into FSACardPort (CardNo ,PortNo, portWM  ) values (%d,%d,%d) ", card_no, portindex, etherport_set->portWorkingMode);
		}
		
		if (etherport_set->setFlag &	MD_ETHERPORT_BWMALLOCSEND)
		{
			sprintf(sql_str, "insert into FSACardPort (CardNo ,PortNo, bwSend  ) values (%d,%d,%d) ", card_no, portindex, etherport_set->bw_alloc_send);
		}
		if (etherport_set->setFlag & MD_ETHERPORT_BWMALLOCRECV)
		{
			sprintf(sql_str, "insert into FSACardPort (CardNo ,PortNo, bwRecv  ) values (%d,%d,%d) ", card_no, portindex, etherport_set->bw_alloc_recv);
		}
		if (etherport_set->setFlag & MD_ETHERPORT_BOUNDEDMAC)
		{
			sprintf(mac_str, "%02X%02X%02X%02X%02X%02X", etherport_set->bounded_mac[0], etherport_set->bounded_mac[1], etherport_set->bounded_mac[2], etherport_set->bounded_mac[3], etherport_set->bounded_mac[4], etherport_set->bounded_mac[5]);
			sprintf(sql_str, "insert into FSACardPort (CardNo ,PortNo, boundMac  ) values (%d,%d,'%s') ", card_no, portindex, mac_str);
		}

		rc = sqlite3_exec(db,
					sql_str,
					0,
					0,
					&zErrMsg);
		if (rc != SQLITE_OK)
		{
			logger (LOG_ERR,  "insert into fsacardport err:%s\n%s\n", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;
		}
		sqlite3_close(db);
		return 0;
		

		
	}
	else
	{
		sqlite3_free_table(result);
		if (etherport_set->setFlag & MD_ETHERPORT_ADMIN)
		{
			sprintf (sql_str, "update FSACardPort set adminState=%d where CardNo=%d and PortNo=%d ", etherport_set->admin_state, card_no, portindex);
		}
				
		if (etherport_set->setFlag & MD_ETHERPORT_MODE)
		{
			sprintf (sql_str, "update FSACardPort set portMode =%d where CardNo=%d and PortNo=%d ", etherport_set->portMode, card_no, portindex);
		}
		if (etherport_set->setFlag & MD_ETHERPORT_WORKINGMODE)
		{
			sprintf (sql_str, "update FSACardPort set portWM =%d where CardNo=%d and PortNo=%d ", etherport_set->portWorkingMode, card_no, portindex);
		}
		
		if (etherport_set->setFlag &	MD_ETHERPORT_BWMALLOCSEND)
		{
			sprintf (sql_str, "update FSACardPort set bwSend =%d where CardNo=%d and PortNo=%d ", etherport_set->bw_alloc_send, card_no, portindex);
		}
		if (etherport_set->setFlag & MD_ETHERPORT_BWMALLOCRECV)
		{
			sprintf (sql_str, "update FSACardPort set bwRecv =%d where CardNo=%d and PortNo=%d ", etherport_set->bw_alloc_recv, card_no, portindex);
		}
		if (etherport_set->setFlag & MD_ETHERPORT_BOUNDEDMAC)
		{
			sprintf(mac_str, "%02X%02X%02X%02X%02X%02X", etherport_set->bounded_mac[0], etherport_set->bounded_mac[1], etherport_set->bounded_mac[2], etherport_set->bounded_mac[3], etherport_set->bounded_mac[4], etherport_set->bounded_mac[5]);
			sprintf(sql_str, "update FSACardPort set boundMac='%s' where CardNo=%d and PortNo=%d"  , mac_str, card_no, portindex);
		}

		rc = sqlite3_exec(db,
					sql_str,
					0,
					0,
					&zErrMsg);
		if (rc != SQLITE_OK)
		{
			logger (LOG_ERR,  "update  fsacardport err:%s\n%s\n", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;
		}
		sqlite3_close(db);
		return 0;
		

	}
	

	

}



int updateFSAName(unsigned char card_no, CARD_SET *cardset)
{
	sqlite3 *db;
	char *zErrMsg =0 ;
	int rc , nrow, ncol;
	char **result;
	char sql_str[256];

	rc = sqlite3_open(CONF_DB, &db);
	if (rc)
	{
		logger  (LOG_ERR,  "can not open fsaconfiig database:%s\n", sqlite3_errmsg(db));
		return ERROR;
	}

	sprintf(sql_str, "select CardNo from FSACard where CardNo=%d", card_no);
	rc = sqlite3_get_table(db,
					sql_str,
					&result,
					&nrow,
					&ncol,
					&zErrMsg);

	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR,  "select cardno err for updata fsaname:%s\n%s\n", sql_str, zErrMsg);
		sqlite3_close(db);
		return ERROR;
	}
	if (nrow < 1)
	{
		sqlite3_free_table(result);
		sprintf(sql_str, "insert into FSACard (CardNo, CardName)  values (%d, '%s')", card_no, cardset->card_name);
		rc = sqlite3_exec(db,
					sql_str,
					0,
					0,
					&zErrMsg);
		if (rc != SQLITE_OK)
		{
			logger (LOG_ERR,  "insert into fsacard err:%s\n%s\n", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;
		}
		sqlite3_close(db);
		return 0;
	}
	else{

		sqlite3_free_table(result);
		sprintf(sql_str, "update FSACard set CardName='%s' where CardNo=%d", cardset->card_name, card_no);
		rc = sqlite3_exec(db,
						sql_str,
						0,
						0,
						&zErrMsg);
		if (rc != SQLITE_OK)
		{
			logger  (LOG_ERR,  "update fsacard name err:%s\n%s\n", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;

		}
		sqlite3_close(db);
		return 0;


	}
	


}

void macStr2Byte(char * mac_str, unsigned char *mac)
{
	int i;
	unsigned int val;
	char tmp[4];

	for(i=0; i<6; i++)
		mac[i] = 0;

	if(mac_str == NULL)
		return;

	for(i=0; i<6; i++)
	{
		tmp[0] = mac_str[i*2];
		tmp[1] = mac_str[i*2 +1];
		tmp[2] = 0;
		val = 0;
		sscanf(tmp, "%x", &val);
		mac[i] = val;
		
		if((mac_str[i*2] == 0) ||(mac_str[i*2+1]) == 0)
			break;
	}
	
}


void downloadONUCfg(unsigned char node, unsigned long onu_dsn)
{
	sqlite3 *db;
	char *zErrMsg =0 ;
	int rc , nrow, ncol;
	char **result;
	char sql_str[256];
	
	ETHER_PORT_SET  etherport_data;
	ONU_UART_CONF 	uart_data;
	int i,offset;
	unsigned char portIndex;

	rc = sqlite3_open(CONF_DB, &db);
	if (rc)
	{
		logger  (LOG_ERR, "Can not open fsaconfig database:%s\n", sqlite3_errmsg(db));
		return;
	}

	sprintf(sql_str, "select adminState, portMode, portWM, bwSend, bwRecv, boundMac, PortNo from ONUPort where OnuDsn=%u", onu_dsn);
	rc = sqlite3_get_table(db,
					sql_str,
					&result,
					&nrow,
					&ncol,
					&zErrMsg);

	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR,  "DB error:sql=%s\n error=%s\n", sql_str, zErrMsg);
		sqlite3_close(db);
		return;
	}

	for(i=0; i<nrow; i++)
	{
		offset = ncol*(i+1);
		etherport_data.admin_state = SAFE_ATOI(result[offset]);
		etherport_data.portMode  = SAFE_ATOI(result[offset + 1]);
		etherport_data.portWorkingMode = SAFE_ATOI(result[offset + 2]);
		etherport_data.bw_alloc_send  = SAFE_ATOI(result[offset + 3]);
		etherport_data.bw_alloc_recv  = SAFE_ATOI(result[offset + 4]);
		macStr2Byte(result[offset + 5], etherport_data.bounded_mac);
		portIndex =  SAFE_ATOI(result[offset + 6]);
		initSetONUFEPort(node, portIndex, &etherport_data);
	}

	// set the uart configuration
	sprintf(sql_str, "select baudrate, function, time, PortNo from ONUUART where OnuDsn=%u", onu_dsn);
	rc = sqlite3_get_table(db,
					sql_str,
					&result,
					&nrow,
					&ncol,
					&zErrMsg);

	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR,  "DB error:sql=%s\n error=%s\n", sql_str, zErrMsg);
		sqlite3_close(db);
		return;
	}

	for(i=0; i<nrow; i++)
	{
		offset = ncol*(i+1);
		uart_data.baudrate = SAFE_ATOI(result[offset]);
		uart_data.function= SAFE_ATOI(result[offset + 1]);
		uart_data.time = SAFE_ATOI(result[offset + 2]);
		uart_data.port  = SAFE_ATOI(result[offset + 3]);

		initSetONUUART(node, &uart_data);
	}
	
	sqlite3_free_table (result);
	sqlite3_close(db);
	return;

}

int updateONUPortCfgInDB(unsigned long onu_dsn ,unsigned char portindex, ETHER_PORT_SET *etherport_set)
{
	sqlite3 *db;
	char *zErrMsg =0 ;
	int rc , nrow, ncol;
	char **result;
	char sql_str[256];
	int record_exist;
	char mac_str[16];
	int i;
	
	ETHER_PORT_SET setValue;

	/* check the previous setting in the database */
	rc = sqlite3_open(CONF_DB, &db);
	if (rc)
	{
		logger  (LOG_ERR, "Can not open fsaconfig database:%s\n", sqlite3_errmsg(db));
		return ERROR;
	}

	sprintf(sql_str, "select adminState, portMode, portWM, bwSend, bwRecv, boundMac from ONUPort where OnuDsn=%u and PortNo=%d", onu_dsn, portindex);
	rc = sqlite3_get_table(db,
					sql_str,
					&result,
					&nrow,
					&ncol,
					&zErrMsg);

	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR,  "DB error:sql=%s\n error=%s\n", sql_str, zErrMsg);
		sqlite3_close(db);
		return ERROR;
	}
	if (nrow < 1)
	{
		record_exist = 0;

		// set the default value
		setValue.admin_state = 1;
		setValue.portMode = 1;
		setValue.portWorkingMode = 1;
		setValue.bw_alloc_send = 0x0C80;
		setValue.bw_alloc_recv = 0x0C80;
		for(i=0;i<6;i++)
			setValue.bounded_mac[i] = 0;
	}
	else
	{
		record_exist = 1;
		setValue.admin_state = SAFE_ATOI(result[ncol]);
		setValue.portMode  = SAFE_ATOI(result[ncol + 1]);
		setValue.portWorkingMode = SAFE_ATOI(result[ncol + 2]);
		setValue.bw_alloc_send  = SAFE_ATOI(result[ncol + 3]);
		setValue.bw_alloc_recv  = SAFE_ATOI(result[ncol + 4]);
		macStr2Byte(result[ncol + 5], setValue.bounded_mac);
	}
	
	sqlite3_free_table (result);

	// update the value with the new one 
	if(etherport_set->setFlag & MD_ETHERPORT_BWMALLOCSEND) 
		setValue.bw_alloc_send = etherport_set->bw_alloc_send;

	if(etherport_set->setFlag & MD_ETHERPORT_BWMALLOCRECV) 
		setValue.bw_alloc_recv = etherport_set->bw_alloc_recv;

	if(etherport_set->setFlag & MD_ETHERPORT_ADMIN) 
		setValue.admin_state = etherport_set->admin_state;

	if(etherport_set->setFlag & MD_ETHERPORT_WORKINGMODE) 
		setValue.portWorkingMode = etherport_set->portWorkingMode;

	if(etherport_set->setFlag & MD_ETHERPORT_MODE) 
	{
		setValue.portMode = etherport_set->portMode;
		memcpy(setValue.bounded_mac, etherport_set->bounded_mac, 6);
	}

	sprintf(mac_str, "%02X%02X%02X%02X%02X%02X", 
		setValue.bounded_mac[0], setValue.bounded_mac[1], setValue.bounded_mac[2], 
		setValue.bounded_mac[3], setValue.bounded_mac[4], setValue.bounded_mac[5]);

	if(record_exist)			
		sprintf (sql_str, "update ONUPort set adminState=%d, portMode=%d, portWM=%d, bwSend=%d, bwRecv=%d, boundMac='%s' where OnuDsn=%u and PortNo=%d ", 
		setValue.admin_state, setValue.portMode, setValue.portWorkingMode, setValue.bw_alloc_send, setValue.bw_alloc_recv, mac_str, onu_dsn, portindex);
	else
		sprintf(sql_str, "insert into ONUPort (OnuDsn, PortNo, adminState, portMode, portWM, bwSend, bwRecv, boundMac ) values (%d,%d,%d,%d, %d,%d,%d, '%s')", 
		onu_dsn, portindex, setValue.admin_state, setValue.portMode, setValue.portWorkingMode, setValue.bw_alloc_send, setValue.bw_alloc_recv, mac_str);

	rc = sqlite3_exec(db,
					sql_str,
					0,
					0,
					&zErrMsg);
	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR,  "update ONUPort table err:%s\n%s\n", sql_str, zErrMsg);
		sqlite3_close(db);
		return ERROR;
	}
	
	sqlite3_close(db);
	return 0;
	
}

int updateONUUARTCfgInDB(unsigned long onu_dsn ,unsigned char portindex, ONU_UART_CONF *uart_set)
{
	sqlite3 *db;
	char *zErrMsg =0 ;
	int rc , nrow, ncol;
	char **result;
	char sql_str[256];
	int record_exist;
	
	ONU_UART_CONF setValue;

	/* check the previous setting in the database */
	rc = sqlite3_open(CONF_DB, &db);
	if (rc)
	{
		logger  (LOG_ERR, "Can not open fsaconfig database:%s\n", sqlite3_errmsg(db));
		return ERROR;
	}

	sprintf(sql_str, "select baudrate, function, time from ONUUART where OnuDsn=%u and PortNo=%d", onu_dsn, portindex);
	rc = sqlite3_get_table(db,
					sql_str,
					&result,
					&nrow,
					&ncol,
					&zErrMsg);

	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR,  "DB error:sql=%s\n error=%s\n", sql_str, zErrMsg);
		sqlite3_close(db);
		return ERROR;
	}
	if (nrow < 1)
	{
		record_exist = 0;

		// set the default value
		setValue.baudrate = 3;
		setValue.function = 0;
		setValue.time = 5;
	}
	else
	{
		record_exist = 1;
		setValue.baudrate = SAFE_ATOI(result[ncol]);
		setValue.function = SAFE_ATOI(result[ncol + 1]);
		setValue.time = SAFE_ATOI(result[ncol + 2]);
	}
	
	sqlite3_free_table (result);

	// update the value with the new one 
	if(uart_set->setFlag & MD_DATACH_BRAUDRATE) 
		setValue.baudrate = uart_set->baudrate;

	if(uart_set->setFlag & MD_DATACH_PORTSEL) 
		setValue.function= uart_set->function;

	if(uart_set->setFlag & MD_DATACH_RECVBUFF) 
		setValue.time = uart_set->time;


	if(record_exist)			
		sprintf (sql_str, "update ONUUART set baudrate =%d, function=%d, time=%d where OnuDsn=%u and PortNo=%d ", 
		setValue.baudrate, setValue.function, setValue.time, onu_dsn, portindex);
	else
		sprintf(sql_str, "insert into ONUUART (OnuDsn, PortNo, baudrate, function, time ) values (%d,%d,%d,%d, %d)", 
		onu_dsn, portindex, setValue.baudrate, setValue.function, setValue.time);

	logger  (LOG_DEBUG, "SQL:%s\n", sql_str);

	rc = sqlite3_exec(db,
					sql_str,
					0,
					0,
					&zErrMsg);
	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR,  "update ONUUART table err:%s\n%s\n", sql_str, zErrMsg);
		sqlite3_close(db);
		return ERROR;
	}
	
	sqlite3_close(db);
	return 0;
	
}

int updateFsaCard(unsigned char card_no, CARD_SET_FSA *data)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc, nrow, ncol;
	char sql_str[256];
	char **result;
	int Index;
	char mac_str[256];

	rc = sqlite3_open(CONF_DB, &db);
	if (rc)
	{
		logger (LOG_ERR,  "Can't open fsa config database : %s\n", sqlite3_errmsg(db));
		return ERROR;
	}

	sprintf(sql_str ,"select * from FSACardPort where CardNo=0 and PortNo=0");
	rc = sqlite3_get_table (db, sql_str, &result,	&nrow, &ncol, &zErrMsg);
	if (rc !=SQLITE_OK )
	{
		logger (LOG_ERR , "PCD: DB select Portno ERR,%s-%s\n" ,sql_str, zErrMsg);
		sqlite3_close(db);
		return ERROR;
	}
	if ( nrow<1 )
		sprintf(sql_str, "insert into FSACardPort (CardNo, PortNo,portMode) values (0, 0, %d)", data->igmp_snooping);
	else
		sprintf(sql_str, "update FSACardPort set portMode=%d where CardNo=0 and PortNo=0", data->igmp_snooping);

	sqlite3_free_table(result);

	rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR , "PCD: sql exec Error: %s-%s", sql_str, zErrMsg);
		sqlite3_close(db);
		return ERROR;
	}


		/*to find in the config database whether the Card is already exist,if 
		  already exists,first ,update it .otherwise insert a new record.*/
				   
	sprintf(sql_str, "select CardNo  from FSACard   where CardNo=%d", card_no);
	rc = sqlite3_get_table(db,
					sql_str,                /* SQL to be executed */
					&result,			/* Result written to a char *[]  that this points to */
					&nrow,			/* Number of result rows written here */
					&ncol,			/* Number of result columns written here */
					&zErrMsg		/* Error msg written here */);
	if (rc !=SQLITE_OK)
	{
		logger  (LOG_ERR , "DB select cardno Error:%s\n", zErrMsg);
		sqlite3_close(db);
		return ERROR ;
					
	}

	if (nrow < 1)
	{ /* no FSA config exist,insert a record*/
		sqlite3_free_table (result);
		

			sprintf(sql_str, "insert into FSACard(CardNo, CardName, adminState  ) values (%d, '%s' ,%d)",card_no,data->card_name,data->admin_state);
			
			rc = sqlite3_exec(db,
						sql_str,              	
						0,		
						0,			
						&zErrMsg);

			if (rc !=SQLITE_OK)
			{
				logger  (LOG_ERR,  "DB insert FSACard config info Error:%s%s\n", sql_str,zErrMsg);
				sqlite3_close(db);
				return ERROR ;
							
			}

	}
	else{

	/*the FSA card config exist update it */
	sqlite3_free_table (result);
	sprintf(sql_str, "update FSACard set CardName='%s' , adminState=%d where CardNo=%d", data->card_name,data->admin_state, card_no);
	rc = sqlite3_exec(db,
					sql_str,             
					0,			
					0,			
					&zErrMsg);

	if (rc !=SQLITE_OK)
	{
		logger  (LOG_ERR, "DB update Name Error:%s%s\n", sql_str, zErrMsg);
		sqlite3_close(db);
		return ERROR;
					
	}

	}

	sprintf(sql_str ,"select * from FSACardPort where CardNo=%d" , card_no );
	rc = sqlite3_get_table (db,
					sql_str,
					&result,
					&nrow,
					&ncol,
					&zErrMsg);
	if (rc !=SQLITE_OK )
	{
		logger (LOG_ERR , "DB select Portno ERR,%s-%s\n" ,sql_str, zErrMsg);
		sqlite3_close(db);
		return ERROR;
	}

	if ( nrow<1 )
	{
		/* no config data exist, insert new records.*/
		sqlite3_free_table(result);
		for (Index=1 ; Index<=2; Index++)
		{	
			sprintf(mac_str, "%02X%02X%02X%02X%02X%02X", data->port[Index-1].bounded_mac[0], data->port[Index-1].bounded_mac[1], data->port[Index-1].bounded_mac[2], data->port[Index-1].bounded_mac[3], data->port[Index-1].bounded_mac[4], data->port[Index-1].bounded_mac[5] );
			sprintf(sql_str, "insert into FSACardPort (CardNo, PortNo,adminState,portMode, bwSend,bwRecv,boundMac ) values (%d,%d,%d,%d,%d,%d,'%s')",card_no,Index,data->port[Index-1].admin_state,data->port[Index-1].portMode,data->port[Index-1].bw_alloc_send,data->port[Index-1].bw_alloc_recv, mac_str );
			rc = sqlite3_exec(
						db,
						sql_str,
						0,
						0,
						&zErrMsg);
			if (rc != SQLITE_OK)
			{
				logger (LOG_ERR , "insert FSAcardport err %s-%s", sql_str, zErrMsg);
				sqlite3_close(db);
				return ERROR;
			}


		}
		sqlite3_close(db);
		return 0;
	}
	else {

		sqlite3_free_table(result);
		sprintf( sql_str, "delete from FSACardPort where CardNo=%d", card_no);
		rc = sqlite3_exec(
					db,
					sql_str,
					0,
					0,
					&zErrMsg);
		if (rc != SQLITE_OK)
		{
			logger ( LOG_ERR, "delete FSAport err %s-%s", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;

		}

		for (Index=1 ; Index<=2; Index++)
		{
			sprintf(mac_str, "%02X%02X%02X%02X%02X%02X", data->port[Index-1].bounded_mac[0], data->port[Index-1].bounded_mac[1], data->port[Index-1].bounded_mac[2], data->port[Index-1].bounded_mac[3], data->port[Index-1].bounded_mac[4], data->port[Index-1].bounded_mac[5] );
			sprintf(sql_str, "insert into FSACardPort (CardNo, PortNo,adminState, portMode, bwSend,bwRecv,boundMac ) values (%d,%d,%d,%d,%d,%d,'%s')",card_no,Index,data->port[Index-1].admin_state,data->port[Index-1].portMode, data->port[Index-1].bw_alloc_send,data->port[Index-1].bw_alloc_recv, mac_str );
			rc = sqlite3_exec(
						db,
						sql_str,
						0,
						0,
						&zErrMsg);
			if (rc != SQLITE_OK)
			{
				logger ( LOG_ERR, "insert FSAcardport err %s-%s", sql_str, zErrMsg);
				sqlite3_close(db);
				return ERROR;
			}


		}
		sqlite3_close(db);
		return 0;

	}
}

int queryPortClockDelay()
{
	sqlite3 *db;
	char *zErrMsg;
	int nrow,ncol, rc,fldIndex;
	char ** result;
	char sql_str[256];
	int ret = 0;
		
	rc = sqlite3_open(CONF_DB, &db);
	if (rc )
	{
		logger (LOG_ERR,  "can not open fsa config database.%s\n",sqlite3_errmsg(db) );
		return 0;
	}

	sprintf(sql_str, "select portMode from FSACardPort where CardNo=0 and PortNo=0");
	rc = sqlite3_get_table(db, sql_str,	 &result, &nrow, &ncol, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		logger (LOG_ERR ,  "db select fsa config cardname err:%s\n", zErrMsg);
		sqlite3_close(db);
		return 0;
	}
	
	if ( nrow > 0)
	{
		fldIndex = ncol;
		if ( NULL != result[fldIndex])
			ret = atoi(result[fldIndex]);
	}	
	
	sqlite3_free_table(result);
	sqlite3_close(db);
	return (ret);
	
}

int ProcessMsg(char * buffer, int size, int fds, int web_flag)
{
	char response_buff[MAX_CMDBUFF_SIZE];
	char temp_buff[16];
	char* data_ptr;
	MSG_HEAD_485 *msg_header;
	MSG_HEAD_485 *msgrsp_header;
	CARD_INFO *	card_info;
	ONU_LIST_ENTRY* onu_list;
	FSA_CARD_STATE * card_fsa;
	SFP_INFO *sfp_index;
	SFP_INFO *sfpinfo;
	FSA_CARD_STATE fsainfo_etherport;
	PORT_PERF	 *portperf;
	CARD_SET_FSA *fsacard_set;
	CARD_SET *cardset;
	FE_PORTINDEX *portindex;
	FE_PORT_STATE *etherport_get;
	FE_PORT_STATE *etherport_index;
	ETHER_PORT_SET *etherport_set;
	GET_ALARMS_RSP * alarm_rsp;
	PORT_COUNT *count;
	FSA_NETWORK *fsa_net;
	FSA_LOWSPD_CONFIG *fsa_ls_config;
	FSA_LOWSPD_STATE *fsa_ls_state;
		
	ONU_INFO* onu_info;
	ONU_UART_CONF *onuUartConfInfo;
	ONU_IO* onu_io;
	ONU_PORTMIRROR_CONF *onuPortMirrorConfInfo;
	ONU_QoS_CONF *onuQoSConfInfo;
	ONU_VLAN_CONF *onuVLANConfInfo;
	unsigned char *num;
	
	ONU_SET	*onuSet;
	ONU_SETID *onu_setid;
	ONU_UART_CONF *onuUartConf;
	ONU_PORTMIRROR_CONF *onuPortMirrorConf;
	ONU_QoS_CONF *onuQoSConf;
	ONU_VLAN_CONF *onuVLANSave;
	ONU_VLAN_CONF *onuVLANDel;
	int snd_size, ret , updateFlag = 0;/*1:update fsacard port ,2:update fsacard name*/

	if(web_flag)
	{
		MSG_HEAD_CMD *cmd_header;
		
		//convert message format
		cmd_header = (MSG_HEAD_CMD *) buffer;
		if(size != cmd_header->length + sizeof(MSG_HEAD_CMD)) {
			logger (LOG_ERR , "Message data length error !\n");
			return -1;
		}
		msg_header = (MSG_HEAD_485 *) temp_buff;
		msg_header->code = cmd_header->msg_code;
		msg_header->dst = 0;
		msg_header->onu_nodeid= cmd_header->node_id;
		msg_header->length= cmd_header->length;
		data_ptr = buffer + sizeof(MSG_HEAD_CMD);
	} else {
		msg_header = (MSG_HEAD_485 *) buffer;
		if(size != msg_header->length + sizeof(MSG_HEAD_485)) {
			logger (LOG_ERR , "Message data length error !\n");
			return -1;
		}
		data_ptr = buffer + sizeof(MSG_HEAD_485);
	}
	
	msgrsp_header = (MSG_HEAD_485 *) response_buff;

	memset(msgrsp_header, 0, sizeof(MSG_HEAD_485));
	msgrsp_header->code = msg_header->code;
	msgrsp_header->dst= CTRL_CARDNO;		/* send response to ctrl card*/
	msgrsp_header->src= msg_header->dst;		/* set self card no*/
	msgrsp_header->onu_nodeid = msg_header->onu_nodeid;
	
	if(msg_header->code == GET_ALARMS)
	{
		if(alarmlog)
			logger(LOG_DEBUG, "Process message:%d\n", msgrsp_header->code);
	} else 
		logger(LOG_DEBUG, "Process message:%d\n", msgrsp_header->code);
	
	switch(msgrsp_header->code){
		case C_CARD_GET:
			card_info = (CARD_INFO*) (response_buff  + sizeof(MSG_HEAD_485));
			card_info->card_no = msg_header->dst;
			ret = GetCardInfo(card_info);
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length = sizeof(CARD_INFO);
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_FSA_GET:
			card_fsa = (FSA_CARD_STATE*)(response_buff  + sizeof(MSG_HEAD_485));
			ret = getFSAState(card_fsa);
			if(ret == 0) 
			{
				int clock_delay = queryPortClockDelay();
				card_fsa->igmp_snooping |= (clock_delay&0xF0);
				msgrsp_header->rsp = 0;
				msgrsp_header->length = sizeof(FSA_CARD_STATE);
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GET:
			onu_info = (ONU_INFO*)(response_buff   + sizeof(MSG_HEAD_485));
			ret = getONUInfo(msg_header->onu_nodeid, onu_info);
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length = sizeof(ONU_INFO);
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;

		case C_ONU_GETDATACH:
			num = response_buff  + sizeof(MSG_HEAD_485);
			onuUartConfInfo = (ONU_UART_CONF*)(response_buff   + sizeof(MSG_HEAD_485) + 1);
			 
			ret = getONUUartInfo(msg_header->onu_nodeid, onuUartConfInfo, num);
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length = 1 + (*num) * sizeof(ONU_UART_CONF);
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GETIO:
			onu_io = (ONU_IO*)(response_buff   + sizeof(MSG_HEAD_485));
			ret = getONUIO(msg_header->onu_nodeid, onu_io);
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length = sizeof(ONU_IO);
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;

		case C_ONU_GETPORTMIRROR:
			onuPortMirrorConfInfo = (ONU_PORTMIRROR_CONF*)(response_buff   + sizeof(MSG_HEAD_485));
			ret = getONUPortMirror(msg_header->onu_nodeid, onuPortMirrorConfInfo);
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length = sizeof(ONU_PORTMIRROR_CONF);
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;

		case C_ONU_GETQoS:
			onuQoSConfInfo = (ONU_QoS_CONF*)(response_buff   + sizeof(MSG_HEAD_485));
			ret = getONUQoS(msg_header->onu_nodeid, onuQoSConfInfo);
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length = sizeof(ONU_QoS_CONF);
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GETVLAN:
			num = (unsigned char *)(response_buff + sizeof(MSG_HEAD_485));
			onuVLANConfInfo = (ONU_VLAN_CONF*)(response_buff + sizeof(MSG_HEAD_485) + sizeof(unsigned char));
			ret = getONUVLAN(msg_header->onu_nodeid, onuVLANConfInfo, num);
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length = (*num) * sizeof(ONU_VLAN_CONF) + sizeof(unsigned char);
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_SFP_GET:
			sfp_index = (SFP_INFO *)data_ptr;
			sfpinfo = (SFP_INFO *) (response_buff + sizeof (MSG_HEAD_485));
			if ( 0 == msg_header->onu_nodeid)
			{
				ret = getFSASFPInfo(sfpinfo, sfp_index->sfpIndex);
				if (ret == 0)
				{
						msgrsp_header->rsp = 0;
						msgrsp_header->length = sizeof(SFP_INFO);
				} else {
						msgrsp_header->rsp = ret;
						msgrsp_header->length = 0;
					}
					
			}
			else {
				ret = getSFPInfo(msg_header->onu_nodeid, sfp_index->sfpIndex, sfpinfo);
				if (ret == 0)
				{
					msgrsp_header->rsp = 0;
					msgrsp_header->length = sizeof(SFP_INFO);
				} else {
					msgrsp_header->rsp = ret;
					msgrsp_header->length = 0;
				}
			}
			break;

		case C_CARD_SET:
			if (msg_header->length != sizeof (CARD_SET))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			cardset = (CARD_SET *)data_ptr;
			if (cardset->setFlag & MD_CARD_ADMIN )
			{
				
				ret = setFSAAdmin( 0, cardset->admin_state);
				if ( ret == 0)
				{	
					adminState = cardset->admin_state;
					msgrsp_header->rsp = 0;
					msgrsp_header->length = 0;
				}
				else {

					msgrsp_header->rsp = ret;
					msgrsp_header->length = 0;
				}
			}

			if (cardset->setFlag & MD_CARD_NAME)
			{
//				ret = updateFSAName(msg_header->dst, cardset);
				logger (LOG_DEBUG, "snmp set name !\n");
				memcpy(CardName, cardset->card_name ,  MAX_DISPLAYNAME_LEN);
				if ( ret == 0)
				{
					
					msgrsp_header->rsp = 0;
					msgrsp_header->length = 0;
				}
				else {

					msgrsp_header->rsp = ret;
					msgrsp_header->length = 0;
				}
			}
			break;

		case C_FSA_SET:
			if(msg_header->length != sizeof(CARD_SET_FSA))
			{
				msgrsp_header->rsp= ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			fsacard_set = (CARD_SET_FSA *) data_ptr;
			memcpy(CardName, fsacard_set->card_name, MAX_DISPLAYNAME_LEN );
			adminState = fsacard_set->admin_state;
			memset(&statisticcount, 0 , sizeof(statisticcount));
			statisticcount.countType = fsacard_set->statistic_type;
			
			if((setFSAAdmin(0, fsacard_set->admin_state)==0) &&(setFSAAdmin(1, fsacard_set->port[0].admin_state)==0)  && (setFSAAdmin(2, fsacard_set->port[1].admin_state)==0) && (setFSAstate(fsacard_set) == 0) ) 
			{
				msgrsp_header->rsp = 0;
				msgrsp_header->length = 0;
				logger(LOG_DEBUG, "set fsa admin successful!\n");
				updateFlag = 1;
				
				//ret = updateFsaCard(msg_header->dst, fsacard_set); 
				
				//if (ret != 0)
				//{
				//	logger(LOG_DEBUG, "Debug update fsaconfig database err .retcode:%d\n ", ret);
				//}
				
				
			} else {
				msgrsp_header->rsp = -1;
			}
			msgrsp_header->length = 0;
			logger(LOG_DEBUG, "fsa return length is %d\n", msgrsp_header->length);
			break;	

		case GET_PORT_PERF:
		 	if(msg_header->length != (sizeof (FE_PORTINDEX)))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			portindex = (FE_PORTINDEX *) data_ptr;

			portperf = (PORT_PERF*)(response_buff   + sizeof(MSG_HEAD_485));
			ret = getPortPerf(msg_header->onu_nodeid, portindex->portIndex -1, portperf );
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length = sizeof(PORT_PERF);
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;

		case C_ONU_SET:
			if(msg_header->length != sizeof(ONU_SET))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			onuSet = (ONU_SET *) data_ptr;
#ifndef DAT_CARD
			if (onuSet->setFlag & MD_ONU_ADMIN)//for snmp.
			{
				ret = setONUAdmin(msg_header->onu_nodeid ,0,  onuSet->admin_state);
				if(ret != 0)
				{
					msgrsp_header->rsp = ret;
					msgrsp_header->length = 0;
					break;
				}
			}
#endif
			if(onuSet->setFlag & MD_ONU_NAME)//for snmp.
			{
				ret = setONUName(msg_header->onu_nodeid, onuSet->onu_name);
				if(ret != 0)
				{
					msgrsp_header->rsp = ret;
					msgrsp_header->length = 0;
					break;
				}
			}

#ifndef DAT_CARD
			//if(onuSet->setFlag & MD_ONU_STORM)//for snmp.
			//{
				//ret = setONUStorm(msg_header->onu_nodeid, onuSet->broadcaststorm_ctrl, onuSet->storm_threshold);
				ret = setONUStorm(msg_header->onu_nodeid, onuSet);
				if(ret != 0)
				{
					msgrsp_header->rsp = ret;
					msgrsp_header->length = 0;
					break;
				}
			//}
			logger(LOG_DEBUG, " set onu storm ret %d, rsp code %d\n", ret, msgrsp_header->rsp);
#endif
			msgrsp_header->rsp = 0;
			msgrsp_header->length = 0;
			break;

		 case C_ONU_SETDATACH:
		 	if(msg_header->length != sizeof(ONU_UART_CONF))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			onuUartConf= (ONU_UART_CONF *) data_ptr;
			/*if (onuUartConf->setFlag & 0xFF)//for web.
			{
				ret = setONUUart(msg_header->onu_nodeid , onuUartConf->busAndBaudrate, onuUartConf->function, onuUartConf->time);
				if(ret != 0)
				{
					msgrsp_header->rsp = ret;
					msgrsp_header->length = 0;
					break;
				}
			}*/
			
			ret = setONUUart(msg_header->onu_nodeid, onuUartConf);
			if(ret != 0)
			{
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
				break;
			}

			//if (onuUartConf->setFlag & md)
			updateFlag = 3; //update in local dabase later
				
			msgrsp_header->rsp = 0;
			msgrsp_header->length = 0;
			break;
			
		case C_ONU_SETIO:
			if(msg_header->length != sizeof(ONU_IO))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}

			onu_io= (ONU_IO *) data_ptr;
			
			ret = setONUIO(msg_header->onu_nodeid, onu_io);
			if(ret != 0)
			{
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
				break;
			}
				
			msgrsp_header->rsp = 0;
			msgrsp_header->length = 0;
			break;

		case C_ONU_SETPORTMIRROR:
			if(msg_header->length != sizeof(ONU_PORTMIRROR_CONF))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}

			onuPortMirrorConf= (ONU_PORTMIRROR_CONF *) data_ptr;
			
			ret = setONUPortMirror(msg_header->onu_nodeid, onuPortMirrorConf);
			if(ret != 0)
			{
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
				break;
			}
				
			msgrsp_header->rsp = 0;
			msgrsp_header->length = 0;
			break;

		case C_ONU_SETQoS:
			if(msg_header->length != sizeof(ONU_QoS_CONF))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}

			onuQoSConf= (ONU_QoS_CONF *) data_ptr;
			
			ret = setONUQoS(msg_header->onu_nodeid, onuQoSConf);
			if(ret != 0)
			{
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
				break;
			}
				
			msgrsp_header->rsp = 0;
			msgrsp_header->length = 0;
			break;
		case C_ONU_SAVEVLAN:
			if(msg_header->length != sizeof(ONU_VLAN_CONF))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}

			onuVLANSave= (ONU_VLAN_CONF *) data_ptr;
			
			ret = saveONUVLAN(msg_header->onu_nodeid, onuVLANSave);
			if(ret != 0)
			{
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
				break;
			}
				
			msgrsp_header->rsp = 0;
			msgrsp_header->length = 0;
			break;
		case C_ONU_DELVLAN:
			if(msg_header->length != sizeof(ONU_VLAN_CONF))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}

			onuVLANDel= (ONU_VLAN_CONF *) data_ptr;
			
			ret = delONUVLAN(msg_header->onu_nodeid, onuVLANDel);
			if(ret != 0)
			{
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
				break;
			}
				
			msgrsp_header->rsp = 0;
			msgrsp_header->length = 0;
			break;
			
		 case C_ETHER_SET:
		 	if(msg_header->length != (sizeof(ETHER_PORT_SET) +sizeof (FE_PORTINDEX)))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
		 	etherport_set = (ETHER_PORT_SET *)data_ptr;
			portindex = (FE_PORTINDEX *) (data_ptr + sizeof(ETHER_PORT_SET));
			if ( 0 == msg_header->onu_nodeid)
			{
				if (etherport_set->setFlag & MD_ETHERPORT_ADMIN)
				{
					ret = setFSAAdmin(portindex->portIndex, etherport_set->admin_state);
					if ( ret == 0)
					{
						msgrsp_header->rsp = 0;
						msgrsp_header->length = 0;
						ret = updatePortForSnmp(msg_header->dst, portindex->portIndex, etherport_set);
					}
					else{

						msgrsp_header->rsp = ret;
						msgrsp_header->length = 0;
					}
					break;
				}
				//if (etherport_set->setFlag & MD_ETHERPORT_WKMODE)
				//if (etherport_set->setFlag &	MD_ETHERPORT_BWMALLOCSEND)
				//if (etherport_set->setFlag & MD_ETHERPORT_BWMALLOCRECV)
				//if (etherport_set->setFlag & MD_ETHERPORT_BOUNDEDMAC)

			}else{ 
				ret = setONUFEPort(msg_header->onu_nodeid, portindex->portIndex -1,  etherport_set);
				if ( ret == 0)
				{
					msgrsp_header->rsp = 0;
					msgrsp_header->length = 0;
				} else{
					msgrsp_header->rsp = ret;
					msgrsp_header->length = 0;
				}
			}
			break;
	     case C_ONU_RESET:
			ret = resetONU(msg_header->onu_nodeid);
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length =0;
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;
	     case SET_PERF_RESET:
		 	if(msg_header->length != (sizeof (FE_PORTINDEX)))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			portindex = (FE_PORTINDEX *) data_ptr;

			portperf = (PORT_PERF*)(response_buff   + sizeof(MSG_HEAD_485));
			
			ret = resetPortPerf(msg_header->onu_nodeid, portindex->portIndex -1);
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length =0;
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;
			

		case C_ONU_GETLIST:
			onu_list = (ONU_LIST_ENTRY*)(response_buff   + sizeof(MSG_HEAD_485) + 1);
			ret = getONUList(onu_list);
			*(response_buff   + sizeof(MSG_HEAD_485)) = (char) ret;
			msgrsp_header->rsp = 0;
			msgrsp_header->length =1 + ret * sizeof(ONU_LIST_ENTRY);
			logger(LOG_DEBUG, "Get Onu list, count:%d\n", ret);
			break;
		case C_ONU_GETDSN_LIST:
			ret = getONUDSNList(response_buff   + sizeof(MSG_HEAD_485));
			msgrsp_header->rsp = 0;
			msgrsp_header->length = ret;
			logger(LOG_DEBUG, "Get Onu Dsn list\n");
			break;
		case C_ONU_SET_ONUID:
			if(msg_header->length != sizeof(ONU_SETID))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}

			onu_setid= (ONU_SETID *) data_ptr;
			
			ret = setONUID(msg_header->onu_nodeid, onu_setid);
			if(ret != 0)
			{
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
				break;
			}
				
			msgrsp_header->rsp = 0;
			msgrsp_header->length = 0;
			break;
			
		case C_ETHER_GET:
		 	if(msg_header->length != sizeof (FE_PORT_STATE))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				logger (LOG_ERR, "get etheport info message error!.length=%d,expect=%d\n", msg_header->length, sizeof(FE_PORT_STATE));
				break;
			}
			etherport_get = (FE_PORT_STATE *)(response_buff + sizeof(MSG_HEAD_485));
			etherport_index = (FE_PORT_STATE *)data_ptr;
			if (0 == msg_header->onu_nodeid)
			{
				ret = getFSAState (&fsainfo_etherport);
				if (ret == 0)
				{
					if ( etherport_index->portIndex == 1)
					{
						etherport_get->admin_state = fsainfo_etherport.geport_state[0].admin_state;
						etherport_get->port_state = fsainfo_etherport.geport_state[0].port_state;
						etherport_get->portWorkingMode = fsainfo_etherport.geport_state[0].portWorkingMode;
						etherport_get->portIndex = 1;
						etherport_get->bw_alloc_send = fsainfo_etherport.geport_state[0].bw_alloc_send;
						etherport_get->bw_alloc_recv = fsainfo_etherport.geport_state[0].bw_alloc_recv;
						memcpy( etherport_get->bounded_mac, &fsainfo_etherport.geport_state[0].bounded_mac, 6);
						msgrsp_header->rsp = 0;
						msgrsp_header->length = sizeof(FE_PORT_STATE);
						logger (LOG_DEBUG, "get etheport info successful.etherportindex=%d,length=%d\n", etherport_index->portIndex, msgrsp_header->length);
					}
					if ( etherport_index->portIndex == 2)
					{
						etherport_get->admin_state = fsainfo_etherport.geport_state[1].admin_state;
						etherport_get->port_state = fsainfo_etherport.geport_state[1].port_state;
						etherport_get->portWorkingMode = fsainfo_etherport.geport_state[1].portWorkingMode;
						etherport_get->portIndex = 2;
						etherport_get->bw_alloc_send = fsainfo_etherport.geport_state[1].bw_alloc_send;
						etherport_get->bw_alloc_recv = fsainfo_etherport.geport_state[1].bw_alloc_recv;
						memcpy( etherport_get->bounded_mac, &fsainfo_etherport.geport_state[1].bounded_mac, 6);
						msgrsp_header->rsp = 0;
						msgrsp_header->length = sizeof(FE_PORT_STATE);
						logger (LOG_DEBUG,  "get etheport info successful.etherportindex=%d,length=%d\n", etherport_index->portIndex, msgrsp_header->length);
					}
				}
				else{

					msgrsp_header->rsp = ret;
					msgrsp_header->length = 0;
					logger (LOG_DEBUG , "get etheport info successful.etherportindex=%d,length=%d\n", etherport_index->portIndex, msgrsp_header->length);
				}
				
			}
			else{
				etherport_index->portIndex -= 1; 
			 	if(etherport_index->portIndex > 3)
				{
					msgrsp_header->rsp = ERROR_WRONG_ARGS;
					msgrsp_header->length = 0;
					logger (LOG_ERR, "get etheport info error!portIndex=%d\n", etherport_index->portIndex);
				break;
				}
				
				ret = getONUFEPort(msg_header->onu_nodeid, etherport_index->portIndex,  etherport_get);
				if ( 0 == ret)
				{
					msgrsp_header->rsp = 0;
					msgrsp_header->length = sizeof(FE_PORT_STATE);
					logger (LOG_DEBUG, "get etheport info successful.etherportindex=%d,length=%d\n", etherport_index->portIndex, msgrsp_header->length);
				} else {
					msgrsp_header->rsp = ret;
					msgrsp_header->length = 0;
					logger (LOG_DEBUG,  "get etheport info failed .etherportindex=%d,length=%d\n", etherport_index->portIndex, msgrsp_header->length);
				}
			}		
			break;
		case GET_ALARMS:
			alarm_rsp = (GET_ALARMS_RSP*)(response_buff   + sizeof(MSG_HEAD_485));
			alarm_rsp->rsp_type = 1;
			ret = getAlarm(alarm_rsp->data.fsa_alarms, 48);
			alarm_rsp->alarmNum = (unsigned char)ret;
			msgrsp_header->rsp = 0;
			msgrsp_header->length = 2 + ret * sizeof(ONET_ALARM_INFO);
			if(alarmlog)
				logger(LOG_DEBUG, "Get Alarm list, count:%d\n", ret);
			break;
			
		case C_FSA_RESETCOUNT:
			ret = resetPortCount();
			if(ret == 0) {
				msgrsp_header->rsp = 0;
				msgrsp_header->length =0;
			} else {
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;
			
		case C_FSA_GETCOUNT:
			count = (PORT_COUNT *)(response_buff + sizeof(MSG_HEAD_485));
			ret = getPortCount(count);
			if ( 0 == ret )
			{
				msgrsp_header->rsp = 0;
				msgrsp_header->length = sizeof(PORT_COUNT);
			}
			else
			{
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;
			
		case REFRESH_CHECK:
			msgrsp_header->length = 0;
			if(msg_header->length !=  sizeof(RERESH_CHECK_REQ))
			{
				logger(LOG_ERR, "Refresh check message length error! return as no refresh need.\n");
				msgrsp_header->rsp = 0; /*no change*/
			} else {
				RERESH_CHECK_REQ *refresh_check;
				refresh_check = (RERESH_CHECK_REQ *)data_ptr;
				msgrsp_header->rsp = getRefreshState(refresh_check->sessionID, refresh_check->type, 0);
			}
			break;		
	     case C_CARD_RESET:
			msg485_send(fds, response_buff, sizeof(MSG_HEAD_485));
			system("/sbin/reboot");
			return;			
		case C_FSA_NET_GET:
			fsa_net = (FSA_NETWORK *)(response_buff + sizeof(MSG_HEAD_485));
			ret = GetNetwork(fsa_net);
			if ( 0 == ret )
			{
				msgrsp_header->rsp = 0;
				msgrsp_header->length = sizeof(FSA_NETWORK);
			}
			else
			{
				msgrsp_header->rsp = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_FSA_NET_SET:
			if(msg_header->length != sizeof(FSA_NETWORK))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}

			fsa_net= (FSA_NETWORK *) data_ptr;			
			ret = SetNetwork(fsa_net);
			msgrsp_header->rsp = ret;
			msgrsp_header->length = 0;
			break;
		case C_FSA_LOWSPD_GET:
			fsa_ls_config = (FSA_LOWSPD_CONFIG *)(response_buff + sizeof(MSG_HEAD_485));
			GetLowspdConfig(fsa_ls_config);
			msgrsp_header->rsp = 0;
			msgrsp_header->length = sizeof(FSA_LOWSPD_CONFIG);
			break;
		case C_FSA_LOWSPD_STAT:
			fsa_ls_state= (FSA_LOWSPD_STATE*)(response_buff + sizeof(MSG_HEAD_485));
			GetLowspdState(fsa_ls_state, msg_header->onu_nodeid);
			msgrsp_header->rsp = 0;
			msgrsp_header->length = sizeof(FSA_LOWSPD_STATE);
			break;
		case C_FSA_LOWSPD_SET:
			if(msg_header->length != sizeof(FSA_LOWSPD_CONFIG))
			{
				msgrsp_header->rsp = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}

			fsa_ls_config= (FSA_LOWSPD_CONFIG *) data_ptr;			
			ret = SetLowspdConfig(fsa_ls_config);
			msgrsp_header->rsp = ret;
			msgrsp_header->length = 0;
			break;
		case C_FSA_LOWSPD_STATRESET:
			reset_rs_statistics();
			reset_io_statistics();
			msgrsp_header->rsp = 0;
			msgrsp_header->length =0;
			break;
	default:
			msgrsp_header->rsp = 1;
			msgrsp_header->length = 0;			
	}

	if(web_flag)
	{
		MSG_HEAD_CMD_RSP *cmdrsp_header;
		char cmdrsp_buff[MAX_CMDBUFF_SIZE];
		
		cmdrsp_header = (MSG_HEAD_CMD_RSP *) cmdrsp_buff;
		memset(cmdrsp_header, 0, sizeof(MSG_HEAD_CMD_RSP));
		cmdrsp_header->msg_code = msgrsp_header->code;
		cmdrsp_header->card_no = 0;
		cmdrsp_header->node_id = msgrsp_header->onu_nodeid;
		cmdrsp_header->length = msgrsp_header->length;
		cmdrsp_header->rsp_code = msgrsp_header->rsp;
		memcpy(cmdrsp_buff+sizeof(MSG_HEAD_CMD_RSP), response_buff + sizeof(MSG_HEAD_485), msgrsp_header->length);
		
		snd_size = cmdrsp_header->length + sizeof(MSG_HEAD_CMD_RSP);
		if(write(fds, cmdrsp_buff, snd_size) != snd_size) {
				logger ( LOG_ERR ,"Message response write error!\n");
				return -1;
		}

		if(dump485)
		{
			struct timeval checkpoint;

			gettimeofday(&checkpoint, NULL);
			logger(LOG_DEBUG, "%d-Send Msg:%d bytes\n", checkpoint.tv_usec/1000, snd_size);
			DumpMsg( (unsigned char *)cmdrsp_buff, snd_size); 
		}

	}else {
		snd_size = msgrsp_header->length + sizeof(MSG_HEAD_485);
		if(msg485_send(fds, response_buff, snd_size) != snd_size) {
				logger(LOG_ERR, "485 Message response send error!\n");
				return -1;
		}
	}
	
	if (1 == updateFlag)	
	{
		ret = updateFsaCard(msg_header->dst, fsacard_set); 
				
		if (ret != 0)
		{
			logger(LOG_WARNING, "Debug update fsaconfig database err .retcode:%d\n ", ret);
		}
		updateFlag = 0;

	}
	else if ( 2 == updateFlag )
	{
		ret = updateFSAName(msg_header->dst, cardset);
		if ( 0 != ret )
		{
			logger (LOG_ERR, "update fsa card name err. retcode :%d\n", ret);
		}
		updateFlag = 0;
	}
	else if ( 3 == updateFlag )
	{
		//update UART setting
		ret = updateONUUARTCfgInDB(ONUDsn[msg_header->onu_nodeid], onuUartConf->port, onuUartConf);
		if ( 0 != ret )
		{
			logger (LOG_ERR, "update ONU uart config failed. retcode :%d\n", ret);
		}
		updateFlag = 0;
	}

	return 0;
}



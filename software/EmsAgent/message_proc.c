#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <search.h>
#include<time.h>
#include <netinet/in.h>
#include <sqlite3.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/serial_comm.h"
#include "../common/logger.h"
#include "refresh_check.h"
#include "sw_ctrl.h"
#include "back_mgmt.h"
#include "card_mgmt.h"

#define	VERSION_HW0 	0
#define	VERSION_HW1 	1
#define	VERSION_FW0 	0
#define	VERSION_FW1 	1
#define	VERSION_SW0 	2
#define	VERSION_SW1 	1

extern int				 	fd485;
extern CARD_INFO				cardData[1+MAX_CARD_NUM_UTN];
extern ETHER_PORT_SET	FE8Card[1+MAX_CARD_NUM_UTN][9];


extern PORT_PERF *perf_data;

/*fsa600 and onu100*/
ONU_SET onu100_Info[1+MAX_CARD_NUM_UTN][ 1 + MAX_ONU100_NUM];//onu100 name.
FSA600_GETRMT  RMT_Data[1+MAX_CARD_NUM_UTN][ 1 + MAX_ONU100_NUM];// first get and then maybe set.for snmp .....
FSA600_PORT_485  FSA600_Data[1+MAX_CARD_NUM_UTN][ 1 + MAX_ONU100_NUM];

/* control card states variables*/
char 	OSversion[MAX_DISPLAYNAME_LEN];
//char 	CardName[MAX_DISPLAYNAME_LEN];
unsigned char 	adminState = 1;
 

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
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	CARD_INFO		*card_rsp;

	if(card_info->card_no == CTRL_CARDNO)
	{
		card_info->admin_state = adminState;
		memcpy(card_info->card_name, cardData[CTRL_CARDNO].card_name , MAX_DISPLAYNAME_LEN);
		card_info->card_type = CARD_TYPE_CTRL;
		card_info->version_HW[0] = '0';
		card_info->version_HW[1] = '1';
		card_info->version_FW[0] = '0';
		card_info->version_FW[1] = '1';
		card_info->version_SW[0] = '2';
		card_info->version_SW[1] = '1';
		logger (LOG_DEBUG, "hw[0] is %c, hw[1] is %c\n", card_info->version_HW[0], card_info->version_HW[1]);
		memcpy(card_info->version_OS, OSversion, MAX_DISPLAYNAME_LEN);
		return 0;
	}
	else
	{
		memset(sendbuff, 0, sizeof(MSG_HEAD_485));
		headReq = (MSG_HEAD_485*)sendbuff;
		headRsp = (MSG_HEAD_485*)recvbuff;
		headReq->code = C_CARD_GET;
		headReq->src = CTRL_CARDNO;
		headReq->onu_nodeid = 0;
		headReq->length = 0;
		
		headReq->dst = card_info->card_no;
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
		{	
			/* Got the response*/
			if(headRsp->rsp != 0)
			{
				logger ( LOG_ERR,  "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				
			
			if(headRsp->length != sizeof(CARD_INFO))
			{
				logger (LOG_ERR  , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(CARD_INFO));
				return(ERROR_COMM_PARSE);
			}
			
			card_rsp = (CARD_INFO *) (recvbuff + sizeof(MSG_HEAD_485));

			if(card_rsp->card_no != card_info->card_no)
			{
				logger (LOG_ERR,  "Error: Get Card response is not desired card number !,  card_no:%d, expect:%d.\n",  card_rsp->card_no , card_info->card_no );
				return(ERROR_COMM_PARSE);
			}

			memcpy(card_info, card_rsp, sizeof(CARD_INFO));

			if ( CARD_TYPE_FE8 == card_rsp->card_type)
			{
				card_info->admin_state = cardData[card_rsp->card_no].admin_state;
				memcpy(card_info->card_name, cardData[card_rsp->card_no].card_name, MAX_DISPLAYNAME_LEN);
			}
			if ( CARD_TYPE_FSA600 == card_rsp->card_type)
			{
				memcpy(card_info->card_name, cardData[card_rsp->card_no].card_name, MAX_DISPLAYNAME_LEN);
			}
			/* update in-memory cache*/
			memcpy(&(cardData[card_rsp->card_no]), card_info, sizeof(CARD_INFO));
			
			return 0;
		}
		
		return(ERROR_COMM_485);
	}
}




int GetCardFsa(unsigned char card_no, FSA_CARD_STATE * card_state)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FSA_CARD_STATE * fsa_rsp;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FSA_GET;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR,  "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(FSA_CARD_STATE))
		{
			logger (LOG_ERR , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(FSA_CARD_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		fsa_rsp = (FSA_CARD_STATE *) (recvbuff + sizeof(MSG_HEAD_485));		
		memcpy(card_state, fsa_rsp, sizeof(FSA_CARD_STATE));
		return 0;
	}
	return(ERROR_COMM_485);

}


int GetCardFE8(unsigned char card_no,  CARD_INFO_FE8 *card_fe8state)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	CARD_INFO_FE8 *fe8_rsp;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FE8_GET;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(CARD_INFO_FE8))
		{
			logger (LOG_ERR,  "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(CARD_INFO_FE8));
			return(ERROR_COMM_PARSE);
		}
			
		fe8_rsp = (CARD_INFO_FE8 *) (recvbuff + sizeof(MSG_HEAD_485));	
		memcpy(card_fe8state, fe8_rsp, sizeof(CARD_INFO_FE8));
		return 0;
	}
	return(ERROR_COMM_485);
	

}

int SetCardNTT(unsigned char card_no, CARD_SET_485FE *nttset)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	CARD_SET_485FE  *card_set;
	int i;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FE8_SET;
	headReq->src = CTRL_CARDNO;
	headReq->dst = card_no;
	headReq->onu_nodeid = 0;
	headReq->length = sizeof(CARD_SET_485FE);

	card_set = (CARD_SET_485FE*)(sendbuff + sizeof(MSG_HEAD_485));
//	card_set->admin_state = cardData[card_no].admin_state;
//	memcpy(card_set, nttset, sizeof(CARD_SET_485FE));
//	logger  (LOG_DEBUG , "set mac is %d-%d-%d-%d-%d-%d\n", card_set->port.bounded_mac[0], card_set->port.bounded_mac[1], card_set->port.bounded_mac[2], card_set->port.bounded_mac[3], card_set->port.bounded_mac[4], card_set->port.bounded_mac[5]);	

		card_set->Index = nttset->Index;
		card_set->port.admin_state = nttset->port.admin_state;
		card_set->port.bw_alloc_recv = nttset->port.bw_alloc_recv;
		card_set->port.bw_alloc_send = nttset->port.bw_alloc_send;
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
		{	
			/* Got the response*/
			if(headRsp->length != 0)
			{
				logger (LOG_ERR , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
				return(ERROR_COMM_PARSE);
			}
			
			if(headRsp->rsp != 0)
			{
				logger (LOG_ERR , "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				

			return 0;	

		}

		return(ERROR_COMM_485);

}

int GetFE8BroadcastStormInfo(unsigned char card_no,BROADCASTSTORM_CTRL_FE *broadcastStormInfo)
{
		char			sendbuff[MAX_485MSG_FULLLEN];
		char			recvbuff[MAX_485MSG_FULLLEN];
		MSG_HEAD_485	*headReq, *headRsp;
		BROADCASTSTORM_CTRL_FE *broadcastStorm_rsp;
	
		memset(sendbuff, 0, sizeof(MSG_HEAD_485));
		headReq = (MSG_HEAD_485*)sendbuff;
		headRsp = (MSG_HEAD_485*)recvbuff;
		headReq->code = C_FE8_BROADCASTSTORM_GET;
		headReq->src = CTRL_CARDNO;
		headReq->length = 0;
			
		headReq->dst = card_no;
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
		{	
				/* Got the response*/
			if(headRsp->rsp != 0)
			{
				logger (LOG_ERR , "Error: Get FE8 BroadcastStorm error response !,  error:%d.\n",	headRsp->rsp);
				return(headRsp->rsp);
			}

			if(headRsp->length != sizeof(BROADCASTSTORM_CTRL_FE))
			{
				logger (LOG_ERR , "Error: Get FE8 BroadcastStorm response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(BROADCASTSTORM_CTRL_FE));
				return(ERROR_COMM_PARSE);
			}
	
			broadcastStorm_rsp = (BROADCASTSTORM_CTRL_FE *)(recvbuff  + sizeof(MSG_HEAD_485));
			memcpy(broadcastStormInfo, broadcastStorm_rsp, sizeof(BROADCASTSTORM_CTRL_FE));
			
			return 0;
		}
		return(ERROR_COMM_485);

}

int SetFE8BroadcastStorm(unsigned char card_no, BROADCASTSTORM_CTRL_FE *broadcastStormInfo)
{
		char			sendbuff[MAX_485MSG_FULLLEN];
		char			recvbuff[MAX_485MSG_FULLLEN];
		MSG_HEAD_485	*headReq, *headRsp;
		BROADCASTSTORM_CTRL_FE *BroadcastStorm_set;
	
		
	
		memset(sendbuff, 0, sizeof(MSG_HEAD_485));
		headReq = (MSG_HEAD_485*)sendbuff;
		headRsp = (MSG_HEAD_485*)recvbuff;
		headReq->code = C_FE8_BROADCASTSTORM_SET;
		headReq->src = CTRL_CARDNO;
		headReq->length = sizeof(BROADCASTSTORM_CTRL_FE);
			
		headReq->dst = card_no;
	
		BroadcastStorm_set = (BROADCASTSTORM_CTRL_FE *)(sendbuff+sizeof(MSG_HEAD_485));
		memcpy(BroadcastStorm_set, broadcastStormInfo, sizeof(BROADCASTSTORM_CTRL_FE));
		
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
		{	
				/* Got the response*/
			if(headRsp->rsp != 0)
			{
				logger ( LOG_ERR ,"Error: set FE8 BroadcastStorm error response !,  error:%d.\n",	headRsp->rsp);
				return(headRsp->rsp);
			}				
					
			if(headRsp->length != 0)
			{
				logger ( LOG_ERR ,"Error: set FE8 BroadcastStorm response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
				return(ERROR_COMM_PARSE);
			}
				
			return 0;
		}
		return(ERROR_COMM_485);

}


int GetCardFE8Port(unsigned char card_no,FE_PORT_STATE *fe8portstate)
{


	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FE_PORT_STATE *port_send;
	FE_PORT_STATE *fe8_rsp;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FE8_PORT_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	headReq->length = sizeof(FE_PORT_STATE);

	port_send = (FE_PORT_STATE*)(sendbuff + sizeof(MSG_HEAD_485));
	port_send->portIndex = fe8portstate->portIndex;
	
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(FE_PORT_STATE))
		{
			logger (LOG_ERR , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(FE_PORT_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		fe8_rsp = (FE_PORT_STATE *) (recvbuff + sizeof(MSG_HEAD_485));
		logger  (LOG_DEBUG , "get mac is %d-%d-%d-%d-%d-%d\n", fe8_rsp->bounded_mac[0], fe8_rsp->bounded_mac[1], fe8_rsp->bounded_mac[2], fe8_rsp->bounded_mac[3], fe8_rsp->bounded_mac[4], fe8_rsp->bounded_mac[5]);	
		memcpy(fe8portstate, fe8_rsp, sizeof(FE_PORT_STATE));
		//memcpy (&FE8PortInfo[card_no][port_send->portIndex], fe8_rsp, sizeof(FE_PORT_STATE));
		//update cache.
		FE8Card[card_no][port_send->portIndex].admin_state = fe8_rsp->admin_state;
		FE8Card[card_no][port_send->portIndex].portMode = fe8_rsp->portMode;
		FE8Card[card_no][port_send->portIndex].portWorkingMode = fe8_rsp->portWorkingMode;
		FE8Card[card_no][port_send->portIndex].bw_alloc_recv = fe8_rsp->bw_alloc_recv;
		FE8Card[card_no][port_send->portIndex].bw_alloc_send = fe8_rsp->bw_alloc_send;
		memcpy( &FE8Card[card_no][port_send->portIndex].bounded_mac , fe8_rsp->bounded_mac, 6);
		return 0;
	}
	return(ERROR_COMM_485);


}


int GetSfpInfo( unsigned char card_no, unsigned char onu_id ,unsigned char Index ,SFP_INFO *sfpState)//Remain to be improved
{
	
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	SFP_INFO *sfp_index;
	SFP_INFO *sfp_rsp;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_SFP_GET;
	headReq->src = CTRL_CARDNO;
	headReq->length = sizeof(SFP_INFO);
	headReq->onu_nodeid = onu_id;
		
	headReq->dst = card_no;
	sfp_index = (SFP_INFO *)(sendbuff + sizeof (MSG_HEAD_485));
	sfp_index->sfpIndex = Index;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}

		if(headRsp->length != sizeof(SFP_INFO))
		{
			logger (LOG_ERR , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(SFP_INFO));
			return(ERROR_COMM_PARSE);
		}
			
		sfp_rsp = (SFP_INFO *)(recvbuff  + sizeof(MSG_HEAD_485));
		memcpy(sfpState, sfp_rsp, sizeof(SFP_INFO));
		
		return 0;
	}
	return(ERROR_COMM_485);
	
}


int GetOnuList( unsigned char card_no, ONU_LIST_ENTRY *ponulist)//present not  use.........
{

	char sendbuff[MAX_485MSG_FULLLEN];
	char recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 *headReq, *headRsp;
	ONU_LIST_ENTRY *rsp_onulist;
	int onucount;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485* )recvbuff;
	headReq->code = C_ONU_GETLIST;
	headReq->dst = card_no;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;


	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR,  "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		rsp_onulist = (ONU_LIST_ENTRY *)(recvbuff + sizeof(MSG_HEAD_485) + 1 );
		onucount = *( recvbuff + sizeof(MSG_HEAD_485));
		if(headRsp->length != onucount*sizeof(ONU_LIST_ENTRY))
		{
			logger (LOG_ERR , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, onucount*sizeof(ONU_LIST_ENTRY));
			return(ERROR_COMM_PARSE);
		}
			
		memcpy(ponulist, rsp_onulist,  onucount*sizeof(ONU_LIST_ENTRY));
		return onucount;
	}
	return(ERROR_COMM_485);

	
	
}

int SetCardFE8(unsigned char card_no, CARD_SET_485FE *fe8set)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	CARD_SET_485FE  *card_set;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FE8_SET;
	headReq->src = CTRL_CARDNO;
	headReq->dst = card_no;
	headReq->onu_nodeid = 0;
	headReq->length = sizeof(CARD_SET_485FE);

	card_set = (CARD_SET_485FE*)(sendbuff + sizeof(MSG_HEAD_485));
	memcpy(card_set, fe8set, sizeof(CARD_SET_485FE));
	logger  (LOG_DEBUG , "set mac is %d-%d-%d-%d-%d-%d\n", card_set->port.bounded_mac[0], card_set->port.bounded_mac[1], card_set->port.bounded_mac[2], card_set->port.bounded_mac[3], card_set->port.bounded_mac[4], card_set->port.bounded_mac[5]);	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		return 0;
	}
	return(ERROR_COMM_485);

}

int updateDBFE8Info(unsigned char card_no , unsigned char portIndex )//Remain to be improved
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc, nrow, ncol;
	char sql_str[256];
	char mac_str[256];
	char **result;


	rc = sqlite3_open(CONF_DB, &db);
	if (rc)
	{
		logger (LOG_ERR ,  "Can't open config database: %s\n", sqlite3_errmsg(db));
		return ERROR;
	}

		/*to find in the config database whether the PortNo is already exist,if 
		  already exists,then update it, otherwise insert a new records.
		 */
				   
	sprintf(sql_str, "select *  from FE8CardPort  where CardNo=%d and PortNo=%d", card_no, portIndex);
	rc = sqlite3_get_table(db,
					sql_str,                /* SQL to be executed */
					&result,			/* Result written to a char *[]  that this points to */
					&nrow,			/* Number of result rows written here */
					&ncol,			/* Number of result columns written here */
					&zErrMsg		/* Error msg written here */);
	if (rc !=SQLITE_OK)
	{
		logger  (LOG_ERR,  "DB select portno Error:%s%s\n",sql_str,zErrMsg);
		sqlite3_close(db);
		return ERROR ;
					
	}

	if (nrow <1)
	{ /* no FE8 port config exist,insert a new records*/
		sqlite3_free_table (result);
		sprintf(mac_str, "%02X%02X%02X%02X%02X%02X",FE8Card[card_no][portIndex].bounded_mac[0], FE8Card[card_no][portIndex].bounded_mac[1], FE8Card[card_no][portIndex].bounded_mac[2], FE8Card[card_no][portIndex].bounded_mac[3], FE8Card[card_no][portIndex].bounded_mac[4], FE8Card[card_no][portIndex].bounded_mac[5]);
		sprintf(sql_str, "insert into FE8CardPort(CardNo, PortNo,adminState ,portMode, portWM , bwSend , bwRecv, boundMac) values (%d,%d,%d,%d,%d,%d,%d, '%s')",
				card_no,portIndex, FE8Card[card_no][portIndex].admin_state,FE8Card[card_no][portIndex].portMode,FE8Card[card_no][portIndex].portWorkingMode,FE8Card[card_no][portIndex].bw_alloc_send,FE8Card[card_no][portIndex].bw_alloc_recv, mac_str);
			
		rc = sqlite3_exec(db,
						sql_str,        
						0,			
						0,			
						&zErrMsg);

		if (rc !=SQLITE_OK)
		{
			logger  (LOG_ERR , "DB insert FE8CardPort config info Error:%s%s\n",sql_str,zErrMsg);
			sqlite3_close(db);
			return ERROR ;
							
			}


			
	sqlite3_close(db);
	return 0;
	}
	else{

	/*the FE8 card config exist ,updata it. */
	sqlite3_free_table (result);

	sprintf(mac_str, "%02X%02X%02X%02X%02X%02X",FE8Card[card_no][portIndex].bounded_mac[0], FE8Card[card_no][portIndex].bounded_mac[1], FE8Card[card_no][portIndex].bounded_mac[2], FE8Card[card_no][portIndex].bounded_mac[3], FE8Card[card_no][portIndex].bounded_mac[4], FE8Card[card_no][portIndex].bounded_mac[5]);
	sprintf(sql_str, "update FE8CardPort set adminState=%d , portMode=%d ,portWM=%d , bwSend=%d , bwRecv=%d, boundMac='%s' where CardNo=%d and PortNo=%d",
	 FE8Card[card_no][portIndex].admin_state,FE8Card[card_no][portIndex].portMode,FE8Card[card_no][portIndex].portWorkingMode,FE8Card[card_no][portIndex].bw_alloc_send,FE8Card[card_no][portIndex].bw_alloc_recv,mac_str ,card_no,portIndex);
			
		rc = sqlite3_exec(db,
						sql_str,            
						0,			
						0,			
						&zErrMsg);

	if (rc !=SQLITE_OK)
	{
		logger  (LOG_ERR , "DB updata FE8CardPort config info Error:%s%s\n", sql_str ,zErrMsg);
		sqlite3_close(db);
		return ERROR ;
							
	}

		sqlite3_close(db);
		return 0;
		

	}	
	
	return ERROR;

}



int GetOnuInfo(unsigned char card_no, unsigned char node_id, ONU_INFO * onu_info)
{

	if ( CARD_TYPE_FSA600 == cardData[card_no].card_type )
	{
		onu_info->node_id = node_id;
		logger(LOG_DEBUG,"get onu100 name !\n");
		memcpy(onu_info->onu_name,  &onu100_Info[card_no][node_id].onu_name  ,MAX_DISPLAYNAME_LEN );
		return 0;
		
	}
	else{
		
		char 			sendbuff[MAX_485MSG_FULLLEN];
		char 			recvbuff[MAX_485MSG_FULLLEN];
		MSG_HEAD_485 	*headReq, *headRsp;
		ONU_INFO * onu_rsp;

		memset(sendbuff, 0, sizeof(MSG_HEAD_485));
		headReq = (MSG_HEAD_485*)sendbuff;
		headRsp = (MSG_HEAD_485*)recvbuff;
		headReq->code = C_ONU_GET;
		headReq->src = CTRL_CARDNO;
		headReq->onu_nodeid = node_id;
		headReq->length = 0;
			
		headReq->dst = card_no;
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
		{	
				/* Got the response*/
			if(headRsp->rsp != 0)
			{
				logger (LOG_ERR , "Error: Get ONU INFO error response !,  error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				

			if(headRsp->length != sizeof(ONU_INFO))
			{
				logger (LOG_ERR , "Error: Get ONU INFO response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ONU_INFO));
				return(ERROR_COMM_PARSE);
			}
				
			onu_rsp = (ONU_INFO *)(recvbuff + sizeof(MSG_HEAD_485));
			memcpy(onu_info, onu_rsp , sizeof(ONU_INFO));
			return 0;
		}
		return(ERROR_COMM_485);

	}
	
}



int GetEtherportInfo(unsigned char card_no, unsigned char node_id, FE_PORT_STATE *etherPortState)
{


	if(cardData[card_no].card_type== CARD_TYPE_FE8)
	{

		 
		  if(GetCardFE8Port(card_no, etherPortState) !=0)
		{
			logger (LOG_DEBUG , "snmp get etherport info err.\n" );
			return -1;
		 }
		  		
		  else
		  {
		  	logger (LOG_DEBUG, "snmp get etherport info success.\n");
			return 0;
		  }
		  	
		
	}
		
	

	if(cardData[card_no].card_type== CARD_TYPE_FSA)
	{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FE_PORT_STATE *etherPort_rsp;
	FE_PORT_STATE *etherportindex;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ETHER_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(FE_PORT_STATE);
	etherportindex = (FE_PORT_STATE *)(sendbuff + sizeof(MSG_HEAD_485));
	etherportindex->portIndex = etherPortState->portIndex;
	logger (LOG_DEBUG , "get fsacard etherport cardno is %d , onuid is %d, portIndex is %d\n", card_no , node_id, etherportindex->portIndex);	
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get fsaCard etherport  error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(FE_PORT_STATE))
		{
			logger (LOG_ERR,  "Error: Get fsaCard etherport  response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(FE_PORT_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		etherPort_rsp	=(FE_PORT_STATE *) (recvbuff + sizeof(MSG_HEAD_485));	
		memcpy(etherPortState,etherPort_rsp , sizeof(FE_PORT_STATE));
		return 0;
	}
	return(ERROR_COMM_485);
		
	}
	
	return (ERROR_UNKNOWN);
		
}


int GetPerfState(unsigned char card_no, unsigned char node_id, unsigned char portIndex,PORT_PERF *perfState)
{
	
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FE_PORTINDEX *feportindex;
	PORT_PERF *perf_rsp;
	int Index; //perf_data memory index.
	unsigned long prev_perfInfo ;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = GET_PORT_PERF;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(FE_PORTINDEX);
	feportindex = (FE_PORTINDEX *)(sendbuff+sizeof(MSG_HEAD_485));
	feportindex->portIndex = portIndex;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(PORT_PERF))
		{
			logger (LOG_ERR , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(PORT_PERF));
			return(ERROR_COMM_PARSE);
		}
			
		perf_rsp =(PORT_PERF *)(recvbuff + sizeof(MSG_HEAD_485));

		if ( cardData[card_no].card_type == CARD_TYPE_FSA )
		{
			memcpy(perfState, perf_rsp, sizeof(PORT_PERF));
		}
		else{
				if ( node_id > MAX_ONU100_NUM)
				{
					logger (LOG_ERR, "node_id exceed max onu100 number. node_id is %d\n", node_id);
					return ERROR;
				}
				
				if ( 0 == node_id )
				{
						Index = (card_no - 1)*MAX_CARD_NUM_UTN + (MAX_ONU100_NUM - 1)*MAX_ONU100_NUM + portIndex;
				}
				else {
					Index = (card_no - 1)*MAX_CARD_NUM_UTN + (node_id - 1)*MAX_ONU100_NUM + portIndex;
					}
				
				logger (LOG_DEBUG, "cardno=%d-onuno=%d-portIndex=%d-Index=%d\n", card_no, node_id, portIndex, Index);
				/*overflow or not ?*/
				prev_perfInfo = perf_data[Index].perfRxByteLo;
				perf_data[Index].perfRxByteLo += ntohl(perf_rsp->perfRxByteLo);
				if ( prev_perfInfo > perf_data[Index].perfRxByteLo )
				{
						perf_data[Index].perfRxByteHi++;			
				}
				
				prev_perfInfo = perf_data[Index].perfTxByteLo;
				perf_data[Index].perfTxByteLo += ntohl(perf_rsp->perfTxByteLo);
				if ( prev_perfInfo > perf_data[Index].perfTxByteLo )
				{
						perf_data[Index].perfTxByteHi++;			
				}
				
				perfState->perfRxByteHi = perf_data[Index].perfRxByteHi;
				perfState->perfRxByteLo = perf_data[Index].perfRxByteLo;
				perfState->perfTxByteHi = perf_data[Index].perfTxByteHi;
				perfState->perfTxByteLo = perf_data[Index].perfTxByteLo;


		}
		
		return 0;
	}
	return(ERROR_COMM_485);
		
}


int getStatisticCount(unsigned char  card_no, PORT_COUNT *portcount )
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	PORT_COUNT *count_rsp;
	int i;
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FSA_GETCOUNT;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger(LOG_ERR , "Error: Get  FSA statistic count info error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(PORT_COUNT))
		{
			logger (LOG_ERR , "Error: Get FSA statistic count info response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(PORT_COUNT));
			return(ERROR_COMM_PARSE);
		}
			
		count_rsp =(PORT_COUNT *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(portcount, count_rsp, sizeof(PORT_COUNT));
		portcount->countType = count_rsp->countType;
		/*for ( i = 0; i < 26; i++ )
		{
			portcount->portCount1[i] = count_rsp->portCount1[i];
			portcount->portCount2[i] = count_rsp->portCount2[i];
			logger (LOG_DEBUG, "response count1 is %d, count2 is %d\n", count_rsp->portCount1[i], count_rsp->portCount2[i] );
			logger (LOG_DEBUG, "copy count1 is %d, count2 is %d\n", portcount->portCount1[i], portcount->portCount2[i] );
		}*/
		return 0;
	}
	return(ERROR_COMM_485);

}

int ReseStatisticCount(unsigned char card_no)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FSA_RESETCOUNT;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger(LOG_ERR , "Error: Get  FSA reset statistic count info error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: Get FSA reset statistic count info response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		
		return 0;
	}
	return(ERROR_COMM_485);


}

int GetFsa600Info ( unsigned char  card_no, unsigned char node_id, FSA600_PORT_485  *fsa600_state)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FSA600_PORT_485 *state_rsp;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FSA600_INFO;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger(LOG_ERR , "Error: Get  FSA600 info error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(FSA600_PORT_485))
		{
			logger (LOG_ERR , "Error: Get FSA600 info response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(FSA600_PORT_485));
			return(ERROR_COMM_PARSE);
		}
			
		state_rsp =(FSA600_PORT_485 *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(fsa600_state, state_rsp, sizeof(FSA600_PORT_485));
		memcpy(&FSA600_Data[card_no][node_id], state_rsp, sizeof(FSA600_PORT_485));
		return 0;
	}
	return(ERROR_COMM_485);


}

int GetFSA600RMT ( unsigned char card_no, unsigned char  node_id, FSA600_GETRMT *getremote)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FSA600_GETRMT *remote_rsp;
	FSA600_GETRMT *rmt_Id;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc, fldIndex, nrow, ncol ,i;
	char sql_str[256];
	char **result;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FSA600_GETRMT;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof( FSA600_GETRMT );

	rmt_Id = (FSA600_GETRMT *)(sendbuff + sizeof (MSG_HEAD_485 ));
	rmt_Id->OnuId = node_id;
	
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get  remote onu100 info error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(FSA600_GETRMT))
		{
			logger (LOG_ERR , "Error: Get remote onu100 info response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(FSA600_GETRMT));
			return(ERROR_COMM_PARSE);
		}
			
		remote_rsp =(FSA600_GETRMT *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy( getremote, remote_rsp, sizeof(FSA600_GETRMT));
		memcpy(&RMT_Data[card_no][node_id], remote_rsp, sizeof (FSA600_GETRMT));

		/*get onu100 name*/
		rc = sqlite3_open(CONF_DB, &db);
		if (rc)
		{
			logger ( LOG_ERR , "Can't open database: %s\n", sqlite3_errmsg(db));
			return ERROR;
		}


		sprintf(sql_str, "select OnuName from ONU100 where CardNo=%d and onuId=%d ", card_no, node_id );
		rc = sqlite3_get_table(db,
						sql_str,
						&result,
						&nrow,
						&ncol,
						&zErrMsg);

		if (rc != SQLITE_OK)
		{
			logger (LOG_ERR , "select onu100 name err  :%s\n%s\n", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;
		}

		/*if the related records does not exist, this code will be skipped.*/
		for ( i = 1; i <= nrow; i++)
		{
			
			fldIndex = i * ncol;
			if ( NULL !=  result[fldIndex] )
			{
				strncpy(getremote->onu_name , result[fldIndex], MAX_DISPLAYNAME_LEN);
				strncpy(RMT_Data[card_no][node_id].onu_name, result[fldIndex], MAX_DISPLAYNAME_LEN);
			}
		}
	
		sqlite3_free_table(result);
		sqlite3_close(db);
		
		return 0;
	}
	return(ERROR_COMM_485);


}



int GetUartInfo(unsigned char  card_no, unsigned char  node_id, char * result, int * rslt_len)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	unsigned char uart_num;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_GETDATACH;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;
	headReq->onu_nodeid = node_id;
	
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get onu data channel error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}

		uart_num = *(recvbuff  + sizeof(MSG_HEAD_485));
		if(headRsp->length != (1 + uart_num * sizeof(ONU_UART_CONF)))
		{
			logger (LOG_ERR , "Error: Get onu data channel response length error!,  length:%d, expect:%d.\n",  headRsp->length, (1 + uart_num * sizeof(ONU_UART_CONF)));
			return(ERROR_COMM_PARSE);
		}

		*rslt_len = headRsp->length;
		memcpy(result, recvbuff  + sizeof(MSG_HEAD_485), headRsp->length);
		
		return 0;
	}
	return(ERROR_COMM_485);

}

int GetONUDSNList(unsigned char  card_no, unsigned char  node_id, char * result, int * rslt_len)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	unsigned char uart_num;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_GETDSN_LIST;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;
	headReq->onu_nodeid = node_id;
	
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get onu dsn list error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}

		if(headRsp->length != MAX_ONUDSN_LEN*MAX_ONU_NUM)
		{
			logger (LOG_ERR , "Error: Get onu dsn list response length error!,  length:%d, expect:%d.\n",  headRsp->length, MAX_ONUDSN_LEN*MAX_ONU_NUM);
			return(ERROR_COMM_PARSE);
		}

		*rslt_len = headRsp->length;
		memcpy(result, recvbuff  + sizeof(MSG_HEAD_485), headRsp->length);
		
		return 0;
	}
	return(ERROR_COMM_485);

}

int GetPortMirror(unsigned char  card_no, unsigned char  node_id,ONU_PORTMIRROR_CONF *onuPortMirrorConfInfo )
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ONU_PORTMIRROR_CONF *portmirror_rsp;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_GETPORTMIRROR;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;
	headReq->onu_nodeid = node_id;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get onu port mirror error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}

		if(headRsp->length != sizeof(ONU_PORTMIRROR_CONF))
		{
			logger (LOG_ERR , "Error: Get onu port mirror response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ONU_PORTMIRROR_CONF));
			return(ERROR_COMM_PARSE);
		}
			
		portmirror_rsp = (ONU_PORTMIRROR_CONF *)(recvbuff  + sizeof(MSG_HEAD_485));
		memcpy(onuPortMirrorConfInfo, portmirror_rsp, sizeof(ONU_PORTMIRROR_CONF));
		
		return 0;
	}
	return(ERROR_COMM_485);
}

int GetIO(unsigned char  card_no, unsigned char  node_id,ONU_IO *io_stat )
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ONU_IO *io_rsp;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_GETIO;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;
	headReq->onu_nodeid = node_id;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get onu io error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}

		if(headRsp->length != sizeof(ONU_IO))
		{
			logger (LOG_ERR , "Error: Get onu io response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ONU_QoS_CONF));
			return(ERROR_COMM_PARSE);
		}
			
		io_rsp = (ONU_IO *)(recvbuff  + sizeof(MSG_HEAD_485));
		memcpy(io_stat, io_rsp, sizeof(ONU_IO));
		
		return 0;
	}
	return(ERROR_COMM_485);
}

int GetQoS(unsigned char  card_no, unsigned char  node_id,ONU_QoS_CONF *onuQoSConfInfo )
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ONU_QoS_CONF *qos_rsp;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_GETQoS;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;
	headReq->onu_nodeid = node_id;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get onu QoS error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}

		if(headRsp->length != sizeof(ONU_QoS_CONF))
		{
			logger (LOG_ERR , "Error: Get onu QoS response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ONU_QoS_CONF));
			return(ERROR_COMM_PARSE);
		}
			
		qos_rsp = (ONU_QoS_CONF *)(recvbuff  + sizeof(MSG_HEAD_485));
		memcpy(onuQoSConfInfo, qos_rsp, sizeof(ONU_QoS_CONF));
		
		return 0;
	}
	return(ERROR_COMM_485);
}


//add by yp
int GetVLANge8(unsigned char  card_no, unsigned char  node_id, ONU_VLAN_CONF *onuVLANConfInfo, unsigned char *totalnum )

{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	//ONU_VLAN_CONF *vlan_rsp;
	unsigned char *num;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_GE8_GETVLAN;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;
	headReq->onu_nodeid = node_id;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get onu VLAN error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}

		num = (unsigned char *)(recvbuff + sizeof(MSG_HEAD_485));
		if(headRsp->length != ((*num) * sizeof(ONU_VLAN_CONF) + sizeof(unsigned char)))
		{
			logger (LOG_ERR , "Error: Get onu VLAN response length error!,  length:%d, expect:%d.\n",  headRsp->length, (*num) * sizeof(ONU_VLAN_CONF) + sizeof(unsigned char));
			return(ERROR_COMM_PARSE);
		}
			
		*totalnum = *num;
		/*vlan_rsp = (ONU_VLAN_CONF *)(recvbuff  + sizeof(MSG_HEAD_485));
		memcpy(onuVLANConfInfo, vlan_rsp, sizeof(ONU_VLAN_CONF));*/
		memcpy(onuVLANConfInfo, recvbuff + sizeof(MSG_HEAD_485) + sizeof(unsigned char), (*num) * sizeof(ONU_VLAN_CONF));
		logger (LOG_DEBUG , "here?\n");
		
		return 0;
	}
	return(ERROR_COMM_485);
}



int GetVLAN(unsigned char  card_no, unsigned char  node_id, ONU_VLAN_CONF *onuVLANConfInfo, unsigned char *totalnum )
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	//ONU_VLAN_CONF *vlan_rsp;
	unsigned char *num;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_GETVLAN;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;
	headReq->onu_nodeid = node_id;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get onu VLAN error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}

		num = (unsigned char *)(recvbuff + sizeof(MSG_HEAD_485));
		if(headRsp->length != ((*num) * sizeof(ONU_VLAN_CONF) + sizeof(unsigned char)))
		{
			logger (LOG_ERR , "Error: Get onu VLAN response length error!,  length:%d, expect:%d.\n",  headRsp->length, (*num) * sizeof(ONU_VLAN_CONF) + sizeof(unsigned char));
			return(ERROR_COMM_PARSE);
		}
			
		*totalnum = *num;
		/*vlan_rsp = (ONU_VLAN_CONF *)(recvbuff  + sizeof(MSG_HEAD_485));
		memcpy(onuVLANConfInfo, vlan_rsp, sizeof(ONU_VLAN_CONF));*/
		memcpy(onuVLANConfInfo, recvbuff + sizeof(MSG_HEAD_485) + sizeof(unsigned char), (*num) * sizeof(ONU_VLAN_CONF));
		logger (LOG_DEBUG , "here?\n");
		
		return 0;
	}
	return(ERROR_COMM_485);
}

int GetVideoState( unsigned char card_no, unsigned char channelno, ENCODE_VIDEO_STATE *videostate)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_VIDEO_STATE *videostate_rsp;
	int timeout;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_VIDEO_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = channelno;
	if(channelno < 2)
		timeout = TIMEOUT_485MSG;
	else
		timeout = 1000000;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(ENCODE_VIDEO_STATE))
		{
			logger (LOG_ERR , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ENCODE_VIDEO_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		videostate_rsp =(ENCODE_VIDEO_STATE *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(videostate, videostate_rsp, sizeof(ENCODE_VIDEO_STATE));
		return 0;
	}
	return(ERROR_COMM_485);

}

int GetNetworkState( unsigned char card_no, unsigned char channelno, ENCODE_NET_STATE *netstate)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_NET_STATE *netstate_rsp;
	int timeout;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_NET_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = channelno;
	if(channelno < 2)
		timeout = TIMEOUT_485MSG;
	else
		timeout = 1000000;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get encodeCard network error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(ENCODE_NET_STATE))
		{
			logger (LOG_ERR , "Error: Get encodeCard network response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ENCODE_NET_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		netstate_rsp =(ENCODE_NET_STATE *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(netstate, netstate_rsp, sizeof(ENCODE_NET_STATE));
		return 0;
	}
	return(ERROR_COMM_485);

}

//add by yp
int GetNetworkStateGe8( unsigned char card_no, unsigned char channelno, GE8_NET_STATE *netstate)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	GE8_NET_STATE *netstate_rsp;
	int timeout;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_GE8_NET_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = channelno;
	timeout = TIMEOUT_485MSG;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get GE8 Card network error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(GE8_NET_STATE))
		{
			logger (LOG_ERR , "Error: Get GE8 Card network response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(GE8_NET_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		netstate_rsp =(GE8_NET_STATE *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(netstate, netstate_rsp, sizeof(GE8_NET_STATE));
		return 0;
	}
	return(ERROR_COMM_485);

}

int GetGe8PortLink( unsigned char card_no, unsigned char channelno,GE_PORT_STATE *ge8_portlink)
{
	char			sendbuff[MAX_485MSG_FULLLEN];
	char			recvbuff[MAX_485MSG_FULLLEN];
	GE_PORT_STATE  *ge8port;
	memset(recvbuff,' ',MAX_485MSG_FULLLEN);
	MSG_HEAD_485	*headReq, *headRsp;
	int timeout;
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_GE8_PORTMODE;
	headReq->src = CTRL_CARDNO;
	timeout = 1000000;
	
	headReq->dst = card_no;
	//if(channelno < 2)
		//timeout = TIMEOUT_485MSG;
	//else
		timeout = 1000000;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
		//logger(LOG_ERR,"length %d\n",headRsp->length);
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get encodeCard network error response !,	error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
	
		if(headRsp->length != 8*sizeof(GE_PORT_STATE))
		{
			logger (LOG_ERR , "Error: Get encodeCard network response length error!,  length:%d, expect:%d.\n",  headRsp->length,  8*sizeof(GE_PORT_STATE));
			return(ERROR_COMM_PARSE);
		}
			memcpy(ge8_portlink,recvbuff + sizeof(MSG_HEAD_485),headRsp->length);	
			return 0;
	}
	//logger(LOG_ERR,"portmode error 485");	
	return(ERROR_COMM_485);

}

//add by yp
int GetGe8Port( unsigned char card_no, unsigned char channelno,GE_PORT_STATE *ge8_portlink)
	{
		char			sendbuff[MAX_485MSG_FULLLEN];
		char			recvbuff[MAX_485MSG_FULLLEN];
		GE_PORT_STATE  *ge8port;
		memset(recvbuff,' ',MAX_485MSG_FULLLEN);
		MSG_HEAD_485	*headReq, *headRsp;
		int timeout;
		memset(sendbuff, 0, sizeof(MSG_HEAD_485));
		headReq = (MSG_HEAD_485*)sendbuff;
		headRsp = (MSG_HEAD_485*)recvbuff;
		headReq->code = C_GE8_GETPORT;
		headReq->src = CTRL_CARDNO;
		timeout = 1000000;
		headReq->length=sizeof(GE_PORT_STATE);
		headReq->dst = card_no;
		//if(channelno < 2)
			//timeout = TIMEOUT_485MSG;
//		else
			timeout = 1000000;
		memcpy(sendbuff + sizeof(MSG_HEAD_485),ge8_portlink,headReq->length);	
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
		{	
			//logger(LOG_ERR,"length %d\n",headRsp->length);
			if(headRsp->rsp != 0)
			{
				logger (LOG_ERR , "Error: Get encodeCard network error response !,	error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				
		
			if(headRsp->length != sizeof(GE_PORT_STATE))
			{
				logger (LOG_ERR , "Error: Get encodeCard network response length error!,  length:%d, expect:%d.\n",  headRsp->length,  8*sizeof(GE_PORT_STATE));
				return(ERROR_COMM_PARSE);
			}
				memcpy(ge8_portlink,recvbuff + sizeof(MSG_HEAD_485),headRsp->length);	
				return 0;
		}
		//logger(LOG_ERR,"portmode error 485"); 
		return(ERROR_COMM_485);
	
	}

int SetGe8Port( unsigned char card_no, unsigned char channelno,GE_PORT_STATE *ge8_portlink)
	{
		char			sendbuff[MAX_485MSG_FULLLEN];
		char			recvbuff[MAX_485MSG_FULLLEN];
		GE_PORT_STATE  *ge8port;
		memset(recvbuff,' ',MAX_485MSG_FULLLEN);
		MSG_HEAD_485	*headReq, *headRsp;
		int timeout;
		memset(sendbuff, 0, sizeof(MSG_HEAD_485));
		headReq = (MSG_HEAD_485*)sendbuff;
		headRsp = (MSG_HEAD_485*)recvbuff;
		headReq->code = C_GE8_SETPORT;
		headReq->src = CTRL_CARDNO;
		timeout = 1000000;
		headReq->length=sizeof(GE_PORT_STATE);
		headReq->dst = card_no;
	//	if(channelno < 2)
	//		timeout = TIMEOUT_485MSG;
	//	else
			timeout = 1000000;
		memcpy(sendbuff + sizeof(MSG_HEAD_485),ge8_portlink,headReq->length);	
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
		{	
			//logger(LOG_ERR,"length %d\n",headRsp->length);
			if(headRsp->rsp != 0)
			{
				logger (LOG_ERR , "Error: Get encodeCard network error response !,	error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				
		
			if(headRsp->length != 0)
			{
				logger (LOG_ERR , "Error: Get encodeCard network response length error!,  length:%d, expect:%d.\n",  headRsp->length,  8*sizeof(GE_PORT_STATE));
				return(ERROR_COMM_PARSE);
			}
				memcpy(ge8_portlink,recvbuff + sizeof(MSG_HEAD_485),headRsp->length);	
				return 0;
		}
		//logger(LOG_ERR,"portmode error 485"); 
		return(ERROR_COMM_485);
	
}

//add by yp

int GetGe8Statistics( unsigned char card_no, unsigned char channelno,GE8_STATISTICS * portstatistics)
{
		char			sendbuff[MAX_485MSG_FULLLEN];
		char			recvbuff[MAX_485MSG_FULLLEN];
		GE8_STATISTICS * port_statistics;
		memset(recvbuff,' ',MAX_485MSG_FULLLEN);
		MSG_HEAD_485	*headReq, *headRsp;
		int timeout,i=0;
		memset(sendbuff, 0, sizeof(MSG_HEAD_485));
		headReq = (MSG_HEAD_485*)sendbuff;
		headRsp = (MSG_HEAD_485*)recvbuff;
		headReq->code = C_GE8_STATISTICS;
		headReq->src = CTRL_CARDNO;
		timeout = 1000000;
		headReq->length=8*sizeof(GE8_STATISTICS);
		headReq->dst = card_no;
		//if(channelno < 2)
	//		timeout = TIMEOUT_485MSG;
	//	else
			timeout = 1000000;
		memcpy(sendbuff + sizeof(MSG_HEAD_485),portstatistics,headReq->length);	
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
		{	
			//logger(LOG_ERR,"length %d\n",headRsp->length);
			if(headRsp->rsp != 0)
			{
				logger (LOG_ERR , "Error: Get encodeCard network error response !,	error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				
		
			if(headRsp->length != 8*sizeof(GE8_STATISTICS))
			{
				logger (LOG_ERR , "Error: Get encodeCard network response length error!,  length:%d, expect:%d.\n",  headRsp->length,  8*sizeof(GE_PORT_STATE));
				return(ERROR_COMM_PARSE);
			}
			
			/*for(i=0;i<8;i++)
			{
				port_statistics=(GE8_STATISTICS *)(recvbuff + sizeof(MSG_HEAD_485)+i*sizeof(GE8_STATISTICS));
				logger(LOG_ERR,"rpack %d,tpack %d,rbyte %d,tbyte %d\n",port_statistics->Rx_Packets,port_statistics->Tx_Packets,port_statistics->Rx_Packets_B,port_statistics->Tx_Packets_B);
				memcpy(dest + sizeof(MSG_HEAD_CMD_RSP)+i*sizeof(GE8_STATISTICS),port_statistics,sizeof(GE8_STATISTICS));
			}*/
			memcpy(portstatistics ,recvbuff + sizeof(MSG_HEAD_485),headRsp->length);		
			
			return 0;
		}
		//logger(LOG_ERR,"portmode error 485"); 
		return(ERROR_COMM_485);
}

//add by yp
int ClrGe8Statistics(unsigned char card_no, unsigned char channelno)
{
	char			sendbuff[MAX_485MSG_FULLLEN];
	char			recvbuff[MAX_485MSG_FULLLEN];
	
	memset(recvbuff,' ',MAX_485MSG_FULLLEN);
	MSG_HEAD_485	*headReq, *headRsp;
	int timeout,i=0;
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_GE8_STATISTICS_CLR;
	headReq->src = CTRL_CARDNO;
	timeout = 1000000;
	headReq->length=0;
	headReq->dst = card_no;
//	if(channelno < 2)
//		timeout = TIMEOUT_485MSG;
//	else
		timeout = 1000000;
	//memcpy(sendbuff + sizeof(MSG_HEAD_485),portstatistics,headReq->length); 
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			//logger(LOG_ERR,"length %d\n",headRsp->length);
			if(headRsp->rsp != 0)
			{
				logger (LOG_ERR , "Error: Get encodeCard network error response !,	error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				
			
			if(headRsp->length != 0)
			{
				logger (LOG_ERR , "Error: Get encodeCard network response length error!,  length:%d, expect:%d.\n",  headRsp->length,  8*sizeof(GE_PORT_STATE));
				return(ERROR_COMM_PARSE);
			}
				
				/*for(i=0;i<8;i++)
				{
					port_statistics=(GE8_STATISTICS *)(recvbuff + sizeof(MSG_HEAD_485)+i*sizeof(GE8_STATISTICS));
					logger(LOG_ERR,"rpack %d,tpack %d,rbyte %d,tbyte %d\n",port_statistics->Rx_Packets,port_statistics->Tx_Packets,port_statistics->Rx_Packets_B,port_statistics->Tx_Packets_B);
					memcpy(dest + sizeof(MSG_HEAD_CMD_RSP)+i*sizeof(GE8_STATISTICS),port_statistics,sizeof(GE8_STATISTICS));
				}*/
			//memcpy(portstatistics ,recvbuff + sizeof(MSG_HEAD_485),headRsp->length);		
				
			return 0;
			}
			//logger(LOG_ERR,"portmode error 485"); 
			return(ERROR_COMM_485);
	

}

int GetGe8IGMP(unsigned char card_no, unsigned char channelno,GE8_IGMP  *igmp_yp)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_GE8_GETIGMP;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = channelno;
	int timeout;
//	if(channelno < 2)
//		timeout = TIMEOUT_485MSG;
//	else
		timeout = 1000000;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get protocol error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(GE8_IGMP))
		{
			logger (LOG_ERR , "Error: Get PROTOCOL response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ENCODE_PROTOCOL_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		//igmp_yp =(GE8_IGMP *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(igmp_yp, recvbuff + sizeof(MSG_HEAD_485), sizeof(GE8_IGMP));
		return 0;
	}
	return(ERROR_COMM_485);
}
int SetGe8IGMP(unsigned char card_no, unsigned char channelno,GE8_IGMP  *igmp_yp)
{
	char			sendbuff[MAX_485MSG_FULLLEN];
	char			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485	*headReq, *headRsp;
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_GE8_SETIGMP;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = channelno;
	int timeout;
///	if(channelno < 2)
//		timeout = TIMEOUT_485MSG;
//	else
		timeout = 1000000;
	headReq->length=sizeof(GE8_IGMP);		
	headReq->dst = card_no;
	memcpy(sendbuff + sizeof(MSG_HEAD_485),igmp_yp,headReq->length);	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
				/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get protocol error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
	
		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: Get PROTOCOL response length error!,	length:%d, expect:%d.\n",  headRsp->length, sizeof(ENCODE_PROTOCOL_STATE));
			return(ERROR_COMM_PARSE);
		}
				
			//igmp_yp =(GE8_IGMP *)(recvbuff + sizeof(MSG_HEAD_485));
		//memcpy(igmp_yp, recvbuff + sizeof(MSG_HEAD_485), sizeof(GE8_IGMP));
		return 0;
		}
		return(ERROR_COMM_485);

}

int GetPROTOCOLState( unsigned char card_no, unsigned char channelno, ENCODE_PROTOCOL_STATE *protocol_state)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_PROTOCOL_STATE *protocolstate_rsp;
	int timeout;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_PROTOCOL_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = channelno;
	if(channelno < 2)
		timeout = TIMEOUT_485MSG;
	else
		timeout = 1000000;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get protocol error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(ENCODE_PROTOCOL_STATE))
		{
			logger (LOG_ERR , "Error: Get PROTOCOL response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ENCODE_PROTOCOL_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		protocolstate_rsp =(ENCODE_PROTOCOL_STATE *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(protocol_state, protocolstate_rsp, sizeof(ENCODE_PROTOCOL_STATE));
		return 0;
	}
	return(ERROR_COMM_485);
}




int GetOSDState( unsigned char card_no, unsigned char channelno, ENCODE_OSD_STATE *osd_state)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_OSD_STATE *osdstate_rsp;
	int timeout;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_OSD_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = channelno;
	if(channelno < 2)
		timeout = TIMEOUT_485MSG;
	else
		timeout = 1000000;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get osd error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(ENCODE_OSD_STATE))
		{
			logger (LOG_ERR , "Error: Get OSD response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ENCODE_OSD_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		osdstate_rsp =(ENCODE_OSD_STATE *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(osd_state, osdstate_rsp, sizeof(ENCODE_OSD_STATE));
		return 0;
	}
	return(ERROR_COMM_485);

}

int GetPICTUREState( unsigned char card_no, unsigned char channelno, ENCODE_PICTURE_STATE *picture_state)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_PICTURE_STATE *picturestate_rsp;
	int timeout;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_PICTURE_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = channelno;
	if(channelno < 2)
		timeout = TIMEOUT_485MSG;
	else
		timeout = 1000000;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get PICTURE error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(ENCODE_PICTURE_STATE))
		{
			logger (LOG_ERR , "Error: Get PICTURE response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ENCODE_PICTURE_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		picturestate_rsp =(ENCODE_PICTURE_STATE *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(picture_state, picturestate_rsp, sizeof(ENCODE_PICTURE_STATE));
		return 0;
	}
	return(ERROR_COMM_485);
}


int GetSysTime( unsigned char card_no, ENCODE_SYSTIME *sys_time)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_SYSTIME *systime_rsp;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_SYSTIME_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get systime error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(ENCODE_SYSTIME))
		{
			logger (LOG_ERR , "Error: Get system time response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ENCODE_SYSTIME));
			return(ERROR_COMM_PARSE);
		}
			
		systime_rsp =(ENCODE_SYSTIME *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(sys_time, systime_rsp, sizeof(ENCODE_SYSTIME));
		return 0;
	}
	return(ERROR_COMM_485);
	
}


int GetNTP( unsigned char card_no, ENCODE_NTP *sys_ntp)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_NTP *sysntp_rsp;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_NTP_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get NTP error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(ENCODE_NTP))
		{
			logger (LOG_ERR , "Error: Get NTP time response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ENCODE_NTP));
			return(ERROR_COMM_PARSE);
		}
			
		sysntp_rsp =(ENCODE_NTP *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(sys_ntp, sysntp_rsp, sizeof(ENCODE_NTP));
		return 0;
	}
	return(ERROR_COMM_485);
	
}

int getPatten(unsigned char card_no , unsigned char channelno , ENCODE_MODE_STATE *patten)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_MODE_STATE *patten_rsp;
	int timeout;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_MODE_GET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = channelno;
	if(channelno < 2)
		timeout = TIMEOUT_485MSG;
	else
		timeout = 1000000;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get patten response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(ENCODE_MODE_STATE))
		{
			logger (LOG_ERR , "Error: Get patten response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(ENCODE_MODE_STATE));
			return(ERROR_COMM_PARSE);
		}
			
		patten_rsp =(ENCODE_MODE_STATE *)(recvbuff + sizeof(MSG_HEAD_485));
		memcpy(patten, patten_rsp, sizeof(ENCODE_MODE_STATE));
		return 0;
	}
	return(ERROR_COMM_485);

}


int SetSysTime (unsigned char card_no, ENCODE_SYSTIME *sys_time)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_SYSTIME *time_set;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_SYSTIME_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	headReq->length = sizeof(ENCODE_SYSTIME);
		
	headReq->dst = card_no;
	time_set = (ENCODE_SYSTIME *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(time_set, sys_time, sizeof(ENCODE_SYSTIME));

	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: set encode card systime error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
		
		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: set encode card systime  error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
				
		return 0;
	}
	return(ERROR_COMM_485);
}


int SetNTP (unsigned char card_no, ENCODE_NTP *ntp)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_NTP *ntp_set;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_NTP_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	headReq->length = sizeof(ENCODE_NTP);
		
	headReq->dst = card_no;
	ntp_set = (ENCODE_NTP *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(ntp_set, ntp, sizeof(ENCODE_NTP));

	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: set encode card ntp error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}	
		
		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: set encode card ntp  error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
				
		return 0;
	}
	return(ERROR_COMM_485);


}
//add by yp
int SetNetworkGE8 (unsigned char  card_no, GE8_NET_STATE *net_state)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	GE8_NET_STATE *net_set;
	int timeout;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_GE8_NET_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	headReq->length = sizeof(GE8_NET_STATE);
	timeout = TIMEOUT_485MSG;
		
	headReq->dst = card_no;
	net_set = (GE8_NET_STATE *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(net_set, net_state, sizeof(GE8_NET_STATE));
	//logger( LOG_ERR , "ip %s   mask    %s   gateway  %s\n",net_state->ipaddr,net_state->mask,net_state->gateway);
	//logger( LOG_ERR , "brfore rsp.\n");
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		
			logger( LOG_ERR , "rep.\n");
		if(headRsp->rsp != 0)
		{
			logger( LOG_ERR , "Error: set encode card network error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger(LOG_ERR , "Error: set encode card network  error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
		//logger( LOG_ERR , "return 0.\n");	
		return 0;
	}
	//logger ( LOG_ERR , "Error: sSetNetworkGE8.\n");
	return(ERROR_COMM_485);




}

int SetNetwork (unsigned char  card_no, ENCODE_NET_STATE *net_state)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_NET_STATE *net_set;
	int timeout;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_NET_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	headReq->length = sizeof(ENCODE_NET_STATE);
	if(net_state->channelno < 2)
		timeout = TIMEOUT_485MSG;
	else
		timeout = 1000000;
		
	headReq->dst = card_no;
	net_set = (ENCODE_NET_STATE *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(net_set, net_state, sizeof(ENCODE_NET_STATE));

	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR , "Error: set encode card network error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: set encode card network  error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	return(ERROR_COMM_485);




}

int SetPROTOCOL (unsigned char card_no, ENCODE_PROTOCOL_STATE *protocol_state)
{
	char			sendbuff[MAX_485MSG_FULLLEN];
	char			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485	*headReq, *headRsp;
	ENCODE_PROTOCOL_STATE *protocol_set;
	int timeout;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_PROTOCOL_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = protocol_state->channelno;
	headReq->length = sizeof(ENCODE_PROTOCOL_STATE);
	if(protocol_state->channelno < 2)
		//timeout = TIMEOUT_485MSG;
		timeout = 600000;
	else
		//timeout = 1000000;
		//timeout = 1200000;
		timeout = 1500000;
			
	headReq->dst = card_no;
	protocol_set = (ENCODE_PROTOCOL_STATE *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(protocol_set, protocol_state, sizeof(ENCODE_PROTOCOL_STATE));
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: set encode card protocol error response !,  error:%d.\n",	headRsp->rsp);
			return(headRsp->rsp);
		}				
					
		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: set encode card protocol  error!,	length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
				
		return 0;
	}
	return(ERROR_COMM_485);
}

int SetVIDEO (unsigned char card_no, ENCODE_VIDEO_STATE *videostate)
	{
		char			sendbuff[MAX_485MSG_FULLLEN];
		char			recvbuff[MAX_485MSG_FULLLEN];
		MSG_HEAD_485	*headReq, *headRsp;
		ENCODE_VIDEO_STATE *video_set;
		int timeout;
	
		memset(sendbuff, 0, sizeof(MSG_HEAD_485));
		headReq = (MSG_HEAD_485*)sendbuff;
		headRsp = (MSG_HEAD_485*)recvbuff;
		headReq->code = C_ENCODE_VIDEO_SET;
		headReq->src = CTRL_CARDNO;
		headReq->onu_nodeid = videostate->channelno;
		headReq->length = sizeof(ENCODE_VIDEO_STATE);
		if(videostate->channelno < 2)
			timeout = TIMEOUT_485MSG;
		else
			timeout = 1000000;
			
		headReq->dst = card_no;
		video_set = (ENCODE_VIDEO_STATE *)(sendbuff+sizeof(MSG_HEAD_485));
		memcpy(video_set, videostate, sizeof(ENCODE_VIDEO_STATE));
	
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
		{	
				/* Got the response*/
			if(headRsp->rsp != 0)
			{
				logger (LOG_ERR , "Error: set encode card video error response !,  error:%d.\n",	headRsp->rsp);
				return(headRsp->rsp);
			}				
					
			if(headRsp->length != 0)
			{
				logger (LOG_ERR , "Error: set encode card video  error!,	length:%d, expect:%d.\n",  headRsp->length, 0);
				return(ERROR_COMM_PARSE);
			}
				
			return 0;
		}
		return(ERROR_COMM_485);
	
	
	}


int SetOSD ( unsigned char card_no, ENCODE_OSD_STATE *osd_state)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_OSD_STATE *osd_set;
	int timeout;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_OSD_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = osd_state->channelno;
	headReq->length = sizeof(ENCODE_OSD_STATE);
	if(osd_state->channelno < 2)
		timeout = TIMEOUT_485MSG;
	else
		timeout = 1000000;
		
	headReq->dst = card_no;
	osd_set = (ENCODE_OSD_STATE *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(osd_set, osd_state, sizeof(ENCODE_OSD_STATE));

	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: set encode card osd error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: set encode card osd  error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	return(ERROR_COMM_485);


}

int SetPicture ( unsigned char card_no, ENCODE_PICTURE_STATE *picture_state)
{
	char			sendbuff[MAX_485MSG_FULLLEN];
	char			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485	*headReq, *headRsp;
	ENCODE_PICTURE_STATE *picture_set;
	int timeout;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_PICTURE_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = picture_state->channelno;
	headReq->length = sizeof(ENCODE_PICTURE_STATE);
	if(picture_state->channelno < 2)
		timeout = TIMEOUT_485MSG;
	else
		timeout = 1000000;
	
	headReq->dst = card_no;
	picture_set = (ENCODE_PICTURE_STATE *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(picture_set, picture_state, sizeof(ENCODE_PICTURE_STATE));
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: set encode card picture error response !,	error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}		
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: set encode card picture error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
		return 0;
	}
	return(ERROR_COMM_485);
}



int SetPatten ( unsigned char card_no, ENCODE_MODE_STATE *patten)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_MODE_STATE *patten_set;
	int timeout;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_MODE_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = patten->channelno;
	headReq->length = sizeof(ENCODE_MODE_STATE);
	if(patten->channelno < 2)
		timeout = TIMEOUT_485MSG;
	else
		timeout = 1000000;
		
	headReq->dst = card_no;
	patten_set = (ENCODE_MODE_STATE *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(patten_set, patten, sizeof(ENCODE_MODE_STATE));

	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, timeout))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: set encode card patten error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: set encode card patten error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	return(ERROR_COMM_485);


}

/*int  encodeCTRL ( unsigned char  card_no,ENCODE_CTRL *control)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ENCODE_CTRL *ctrl_set;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ENCODE_CTRL;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = control->channelno;
	headReq->length = sizeof(ENCODE_CTRL);
		
	headReq->dst = card_no;
	ctrl_set = (ENCODE_CTRL *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(ctrl_set, control, sizeof(ENCODE_CTRL));

	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			// Got the response
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: set encode card ctrl error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: set encode card ctrl error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	return(ERROR_COMM_485);


}*/

int ResetPerfState(unsigned char card_no, unsigned char node_id, unsigned char portIndex)
{
	
	
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FE_PORTINDEX *feportindex;
	int Index;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = SET_PERF_RESET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(FE_PORTINDEX);
	feportindex = (FE_PORTINDEX *)(sendbuff+sizeof(MSG_HEAD_485));
	feportindex->portIndex = portIndex;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
		
		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: Get Card response length error!,  length:%d\n",  headRsp->length);
			return(ERROR_COMM_PARSE);
		}
			
		if ( 0 == node_id )
		{
				Index = (card_no - 1)*MAX_CARD_NUM_UTN + (MAX_ONU100_NUM - 1)*MAX_ONU100_NUM + portIndex;
		}
		else {
			Index = (card_no - 1)*MAX_CARD_NUM_UTN + (node_id - 1)*MAX_ONU100_NUM + portIndex;
			}
		logger (LOG_DEBUG, "cardno=%d-onuno=%d-portIndex=%d-Index=%d\n", card_no, node_id, portIndex, Index);		

		perf_data[Index].perfRxByteHi = 0;
		perf_data[Index].perfRxByteLo = 0;
		perf_data[Index].perfTxByteHi = 0;
		perf_data[Index].perfTxByteLo = 0;
		return 0;
	}
	return(ERROR_COMM_485);
		

}


int SetEtherPort(unsigned char card_no, unsigned char node_id,  unsigned char portindex,ETHER_PORT_SET *data)//for snmp
{
	if(cardData[card_no].card_type ==CARD_TYPE_FSA )
	{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FE_PORTINDEX *etherportindex;
	ETHER_PORT_SET *setetherport;
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ETHER_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof (FE_PORTINDEX) + sizeof(ETHER_PORT_SET);

	setetherport = (ETHER_PORT_SET *)(sendbuff + sizeof (MSG_HEAD_485));
	memcpy(setetherport, data, sizeof(ETHER_PORT_SET));
	etherportindex = (FE_PORTINDEX *)(sendbuff + sizeof (MSG_HEAD_485) + sizeof (ETHER_PORT_SET));
	etherportindex->portIndex = portindex;
	
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			

		return 0;
	}
	return(ERROR_COMM_485);

	}

	if(cardData[card_no].card_type ==CARD_TYPE_FE8)
	{
		CARD_SET_485FE fe8set;
		fe8set.Index = portindex;
		fe8set.port.admin_state = FE8Card[card_no][portindex].admin_state;
		fe8set.admin_state = cardData[card_no].admin_state;
		fe8set.port.portMode = FE8Card[card_no][portindex].portMode;
		fe8set.port.portWorkingMode = FE8Card[card_no][portindex].portWorkingMode;
		fe8set.port.bw_alloc_send = FE8Card[card_no][portindex].bw_alloc_send;
		fe8set.port.bw_alloc_recv = FE8Card[card_no][portindex].bw_alloc_recv;
		memcpy (fe8set.port.bounded_mac, FE8Card[card_no][portindex].bounded_mac, 6);
		 
		 if (data->setFlag & MD_ETHERPORT_ADMIN)
		 {
	
			fe8set.port.admin_state = data->admin_state;
			logger (LOG_DEBUG, "set fe8 etherport port  admin_state  is %d - %d  \n", data->admin_state, fe8set.port.admin_state);
	       	}
				
		if (data->setFlag & MD_ETHERPORT_MODE)
		 {

			fe8set.port.portMode = data->portMode;
			logger (LOG_DEBUG, "set fe8 portMode  is %d - %d  \n", data->portMode, fe8set.port.portMode);
	       	}

		if (data->setFlag & MD_ETHERPORT_WORKINGMODE )
		 {

			fe8set.port.portWorkingMode= data->portWorkingMode;
			logger (LOG_DEBUG, "set fe8 portWorkingMode  is %d - %d  \n", data->portWorkingMode, fe8set.port.portWorkingMode );
	       	}
		
		if (data->setFlag & MD_ETHERPORT_BWMALLOCSEND)
		 {
			
			fe8set.port.bw_alloc_send = data->bw_alloc_send;
			logger (LOG_DEBUG, "set fe8 bw_alloc_send  is %d - %d  \n", data->bw_alloc_send,fe8set.port.bw_alloc_send);
	      	 }
		
		if (data->setFlag & MD_ETHERPORT_BWMALLOCRECV)
		 {
			
			fe8set.port.bw_alloc_recv = data->bw_alloc_recv;
			logger (LOG_DEBUG, "set fe8 bw_alloc_recv  is %d - %d  \n", data->bw_alloc_recv , fe8set.port.bw_alloc_recv);
	        }
		
		if (data->setFlag & MD_ETHERPORT_BOUNDEDMAC)
		 {

			memcpy (fe8set.port.bounded_mac, data->bounded_mac, 6);
			logger  (LOG_DEBUG , "set  fe8 mac from snmp  is %d-%d-%d-%d-%d-%d\n", data->bounded_mac[0], data->bounded_mac[1], data->bounded_mac[2], data->bounded_mac[3], data->bounded_mac[4], data->bounded_mac[5]);	
			logger  (LOG_DEBUG , "set  fe8 mac   is %d-%d-%d-%d-%d-%d\n", fe8set.port.bounded_mac[0], fe8set.port.bounded_mac[1], fe8set.port.bounded_mac[2], fe8set.port.bounded_mac[3], fe8set.port.bounded_mac[4], fe8set.port.bounded_mac[5]);
	       }
		
		if( SetCardFE8(card_no, &fe8set) != 0)
		{
			logger ( LOG_ERR ,"snmp  setcardfe8  err cardno %d", card_no);
			return ERROR;
						
		}
		else
		{
			logger (LOG_DEBUG , "snmp set cardfe8 successful! cardno is %d\n" , card_no);
			FE8Card[card_no][portindex].admin_state = data->admin_state;
			FE8Card[card_no][portindex].portMode = data->portMode;
			FE8Card[card_no][portindex].portWorkingMode = data->portWorkingMode;
			FE8Card[card_no][portindex].bw_alloc_recv = data->bw_alloc_recv;
			FE8Card[card_no][portindex].bw_alloc_send = data->bw_alloc_send;
			memcpy(&FE8Card[card_no][portindex].bounded_mac, data->bounded_mac, 6);
			return 0;
		}
		

	}

	return ERROR;
} 




int updateDBCardName(unsigned char card_no ,char  name[])
{
		sqlite3 *db;
		char *zErrMsg = 0;
		int rc,  nrow, ncol;
		char sql_str[256];
		char **result;

		rc = sqlite3_open(CONF_DB, &db);
		if (rc)
		{
			logger ( LOG_ERR , "Can't open database: %s\n", sqlite3_errmsg(db));
			return ERROR;
		}

		/*to find in the config  database whether the card is already exist,if the card
		   already exists,update it,otherwise insert a new column.*/
							   
	    sprintf(sql_str, "select CardName from FE8Card where CardNo=%d", card_no);
	rc = sqlite3_get_table(db,
					sql_str,                /* SQL to be executed */
					&result,			/* Result written to a char *[]  that this points to */
					&nrow,			/* Number of result rows written here */
					&ncol,			/* Number of result columns written here */
					&zErrMsg		/* Error msg written here */);
	if (rc !=SQLITE_OK)
	{
		logger  ( LOG_ERR ,"DB Error:%s\n", zErrMsg);
		sqlite3_close(db);
		return ERROR ;
								
	}

	if (nrow < 1)
	{ /* no FE8 card config exist*/
		sqlite3_free_table (result);
		sprintf(sql_str, "insert into FE8Card(CardNo, CardName) values (%d,'%s')", card_no,name );
		rc = sqlite3_exec(db,
					sql_str,              
					0,			
					0,		
					&zErrMsg);

	if (rc !=SQLITE_OK)
	{
		logger  (LOG_ERR , "DB insert FE8Card config info Error:%s\n", zErrMsg);
		sqlite3_close(db);
		return ERROR ;
								
	}
	sqlite3_close(db);
	strncpy(cardData[card_no].card_name,  name,MAX_DISPLAYNAME_LEN );
	return 0;
								
	}
	else{

		/*the FE8 card config exist update it */
		sqlite3_free_table (result);
		sprintf(sql_str, "update FE8Card set  CardName='%s' where CardNo=%d",  name, card_no);
		rc = sqlite3_exec(db,
					sql_str,             
					0,			
					0,			
					&zErrMsg);

		if (rc !=SQLITE_OK)
		{
			logger  (LOG_ERR , "DB update Name Error:%s\n", zErrMsg);
			sqlite3_close(db);
			return ERROR;
								
		}
		sqlite3_close(db);
		strncpy(cardData[card_no].card_name,  name,MAX_DISPLAYNAME_LEN );
		return 0;
								

	}

	return ERROR;


}

int updateDBCardadmin(unsigned char card_no ,unsigned char admin_state )//Do not combined with the CardName in order to facilitate the updating of the port
{
/*write to configDB,this code specifically for the snmp setcard,for the web ,this code canbe implement later */
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc ,  nrow, ncol;
	char sql_str[256];
	char **result;

	rc = sqlite3_open(CONF_DB, &db);
	if (rc)
	{
		logger ( LOG_ERR , "Can't open database: %s\n", sqlite3_errmsg(db));
		return ERROR;
	}

	/*to find in the config database whether the card is already exist,if the card
	  already exists,update it,otherwise insert a new column.*/
						   
	sprintf(sql_str, "select CardName from FE8Card where CardNo=%d", card_no);
	rc = sqlite3_get_table(db,
					sql_str,                /* SQL to be executed */
					&result,			/* Result written to a char *[]  that this points to */
					&nrow,			/* Number of result rows written here */
					&ncol,			/* Number of result columns written here */
					&zErrMsg		/* Error msg written here */);
	if (rc !=SQLITE_OK)
	{
		logger ( LOG_ERR ,"DB Error:%s\n", zErrMsg);
		sqlite3_close(db);
		return ERROR ;
							
	}

	if (nrow < 1)
	{ /* no FE8 card config exist*/
		sqlite3_free_table (result);
		sprintf(sql_str, "insert into FE8Card(CardNo, adminState ) values (%d, %d)", card_no,admin_state);
		rc = sqlite3_exec(db,
		sql_str,             
		0,		
		0,			
		&zErrMsg);

	if (rc !=SQLITE_OK)
	{
		logger ( LOG_ERR ,"DB insert FE8Card config info Error:%s\n", zErrMsg);
		sqlite3_close(db);
		return ERROR ;
							
	}
	sqlite3_close(db);
	cardData[card_no].admin_state = admin_state;
		return 0;
							
				
	}
	else {
		/*fe8card config exists,update it.*/
		sqlite3_free_table(result);
		sprintf ( sql_str, "update FE8Card set adminState=%d where CardNo=%d", admin_state, card_no);
		rc = sqlite3_exec( db,
					sql_str,
						0,
						0,
					&zErrMsg);
		if (rc != SQLITE_OK)
		{
			printf ("update fe8card adminstate err: %s\n%s\n", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;
									
		}

		sqlite3_close(db);
		cardData[card_no].admin_state = admin_state;

		return 0;


	}
	return ERROR;

}


int SetCard(unsigned char card_no, unsigned char node_id, CARD_SET *data)//Remain to be improved
{     //card_type =atoi(cardData[card_no].card_type);
	int card_type = (int) cardData[(int)card_no].card_type;
	CARD_SET_485FE fe8set;
	int portIndex;
	logger (LOG_DEBUG, "setcard flag is %d, card type is %d , card number is %d\n", data->setFlag, card_type , card_no);
	switch(card_type)
	{
		case  CARD_TYPE_NTT:
		case  CARD_TYPE_NTT_S: // 20150115
			if((data->setFlag & MD_CARD_ADMIN) || (data->setFlag == 0xFF))//need to modify
			{					
				/*just set adminstate,set Index = 0;ignore ports.
				*/
				
				fe8set.admin_state = data->admin_state;
				fe8set.Index = 0;
				
				if( SetCardFE8(card_no, &fe8set) != 0)
				{
					logger ( LOG_ERR ,"setcardNTT err cardno %d", card_no);
					return ERROR;
			       }	
						
			}	
	
			return 0;	
			break;
		case CARD_TYPE_FE8:
			if((data->setFlag & MD_CARD_ADMIN) || (data->setFlag == 0xFF))//need to modify
			{					
				/*just set adminstate,set Index = 0;ignore ports.
				*/
				
				fe8set.admin_state = data->admin_state;
				fe8set.Index = 0;					
				if( SetCardFE8(card_no, &fe8set) != 0)
				{
					logger ( LOG_ERR ,"setcardfe8 err cardno %d", card_no);
					return ERROR;
			       }
				if( updateDBCardadmin(card_no, data->admin_state ) != 0)
				{
					logger ( LOG_ERR ,"update fe8 admin err.\n");
					return ERROR;
				 }
				/*update setfe8 card cache.*/
				for (portIndex = 1; portIndex <= 8; portIndex++ )
				{
					FE8Card[card_no][portIndex].admin_state = data->admin_state;
					logger (LOG_DEBUG , "FE8Card's admin_stae is %d , card admin_state is %d\n", FE8Card[card_no][portIndex].admin_state, data->admin_state);
				}
				
				for (portIndex = 1;portIndex <= 8; portIndex++ )
				{
					if (  updateDBFE8Info( card_no , portIndex ) != 0 )
				        {
						logger (LOG_ERR ,"update fe8  port Index is %d .\n", portIndex);
						return ERROR;
				         }else{
					 	logger (LOG_DEBUG, "update fe8 port successful.Index is %d\n", portIndex );
				       }
					
				}
			
				
						
			}	
		
			
			if((data->setFlag & MD_CARD_NAME) ||(data->setFlag == 0xFF))
			{
				strncpy(cardData[card_no].card_name,  data->card_name,MAX_DISPLAYNAME_LEN );			
				if (0 != updateDBCardName(card_no, data->card_name) )
				{
						logger (LOG_ERR, "update fe8 card name for snmp err.\n");
						return ERROR;
				}
				
			}

			return 0;	
				break;
		case CARD_TYPE_FSA600://Now only consider changing the name of fsa600card.store in fe8card table.
			strncpy(cardData[card_no].card_name,  data->card_name,MAX_DISPLAYNAME_LEN );			
			if (0 != updateDBCardName(card_no, data->card_name) )
			{
					logger (LOG_ERR, "update fsa600 card name  err.\n");
					return ERROR;
			}
			return 0;
			break;
			
		case CARD_TYPE_CTRL:
			logger(LOG_DEBUG, "snmp set ctrl card name , set name is %s\n", data->card_name);
				if((data->setFlag & MD_CARD_NAME) ||data->setFlag == 0xFF)
				{

					if (0 != updateDBCardName(card_no, data->card_name) )
					{
						logger (LOG_ERR, "update control card name err.\n");
						return ERROR;
					}
					return 0;
							
				}
				break;
			
		case CARD_TYPE_FSA:
			char 	sendbuff[MAX_485MSG_FULLLEN];
			char 	recvbuff[MAX_485MSG_FULLLEN];
			MSG_HEAD_485 	*headReq, *headRsp;
			CARD_SET *cardset;
			logger (LOG_DEBUG, "set fsa12500 execute here ?");
			memset(sendbuff, 0 ,sizeof(MSG_HEAD_485));
			headReq = (MSG_HEAD_485 *)sendbuff;
			headRsp = (MSG_HEAD_485 *)recvbuff;
			headReq->code = C_CARD_SET;
			headReq->src = CTRL_CARDNO;
			headReq->dst = card_no;
			headReq->onu_nodeid = 0;
			headReq->length = sizeof (CARD_SET);

			cardset = (CARD_SET *)(sendbuff + sizeof (MSG_HEAD_485));
			memcpy(cardset, data, sizeof (CARD_SET));

				
			if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
			{	
				/* Got the response*/
				if(headRsp->rsp != 0)
				{
					logger( LOG_ERR ,"Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
					return(headRsp->rsp);
				}				
				if(headRsp->length != 0)
				{
					logger( LOG_ERR ,"Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
					return(ERROR_COMM_PARSE);
				}
									
				return 0;
			}
			
			return(ERROR_COMM_485);
			break;

		default:
				break;

		 }
		


	return 0;
}


int SetCardFSA(unsigned char card_no, CARD_SET_FSA * fsa_set)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	CARD_SET_FSA  *fsacard_set;

	

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FSA_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	headReq->length = sizeof(CARD_SET_FSA);
		
	headReq->dst = card_no;

	fsacard_set = (CARD_SET_FSA *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(fsacard_set, fsa_set, sizeof(CARD_SET_FSA));
	


		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
		{	
			/* Got the response*/
			if(headRsp->rsp != 0)
			{
				logger ( LOG_ERR ,"Error: set fsa Card error response !,  error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				
					
			if(headRsp->length != 0)
			{
				logger ( LOG_ERR ,"Error: set  fsaCard response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
				return(ERROR_COMM_PARSE);
			}
				
			return 0;
	}
	return(ERROR_COMM_485);


}


int SetOnuUart (unsigned char  card_no, unsigned char  node_id,ONU_UART_CONF *onu_uart)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ONU_UART_CONF*uart_set;

	

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_SETDATACH;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(ONU_UART_CONF);
		
	headReq->dst = card_no;

	uart_set = (ONU_UART_CONF *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(uart_set, onu_uart, sizeof(ONU_UART_CONF));
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: set onu data channel error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: set onu data channel response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	return(ERROR_COMM_485);



}

int SetOnuIO (unsigned char  card_no, unsigned char  node_id,ONU_IO *io_set )
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_SETIO;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(ONU_IO);
		
	headReq->dst = card_no;

	memcpy(sendbuff+sizeof(MSG_HEAD_485), io_set, sizeof(ONU_IO));
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: set onu io error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: set onu io response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	return(ERROR_COMM_485);
}

int SetOnuID (unsigned char  card_no, unsigned char  node_id, ONU_SETID *onu_setid )
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_SET_ONUID;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(ONU_SETID);
		
	headReq->dst = card_no;

	memcpy(sendbuff+sizeof(MSG_HEAD_485), onu_setid, sizeof(ONU_SETID));
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: set onu io error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: set onu io response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	return(ERROR_COMM_485);
}

int SetOnuPortMirror (unsigned char  card_no, unsigned char  node_id,ONU_PORTMIRROR_CONF *onuPortMirrorConfInfo)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ONU_PORTMIRROR_CONF *portMirror_set;

	

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_SETPORTMIRROR;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(ONU_PORTMIRROR_CONF);
		
	headReq->dst = card_no;

	portMirror_set = (ONU_PORTMIRROR_CONF *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(portMirror_set, onuPortMirrorConfInfo, sizeof(ONU_PORTMIRROR_CONF));
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: set onu port mirror error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: set onu port mirror response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	return(ERROR_COMM_485);
}


int SetOnuQoS (unsigned char  card_no, unsigned char  node_id,ONU_QoS_CONF *onuQoSConfInfo )
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ONU_QoS_CONF *QoS_set;

	

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_SETQoS;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(ONU_QoS_CONF);
		
	headReq->dst = card_no;

	QoS_set = (ONU_QoS_CONF *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(QoS_set, onuQoSConfInfo, sizeof(ONU_QoS_CONF));
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: set onu QoS error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: set onu QoS response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	return(ERROR_COMM_485);
}


int SaveONUVLAN (unsigned char  card_no, unsigned char  node_id, ONU_VLAN_CONF *onuVLANConfInfo )
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ONU_VLAN_CONF *VLAN_save;

	

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_SAVEVLAN;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(ONU_VLAN_CONF);
		
	headReq->dst = card_no;

	VLAN_save = (ONU_VLAN_CONF *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(VLAN_save, onuVLANConfInfo, sizeof(ONU_VLAN_CONF));
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: save onu VLAN error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: save onu VLAN response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	return(ERROR_COMM_485);

}

//add by yp
int SaveGE8VLAN (unsigned char  card_no, unsigned char  node_id, ONU_VLAN_CONF *onuVLANConfInfo )
	{
		char			sendbuff[MAX_485MSG_FULLLEN];
		char			recvbuff[MAX_485MSG_FULLLEN];
		MSG_HEAD_485	*headReq, *headRsp;
		ONU_VLAN_CONF *VLAN_save;
	
		
		
		memset(sendbuff, 0, sizeof(MSG_HEAD_485));
		headReq = (MSG_HEAD_485*)sendbuff;
		headRsp = (MSG_HEAD_485*)recvbuff;
		headReq->code = C_GE8_SAVEVLAN;
		headReq->src = CTRL_CARDNO;
		headReq->onu_nodeid = node_id;
		headReq->length = sizeof(ONU_VLAN_CONF);
			
		headReq->dst = card_no;
		
		VLAN_save = (ONU_VLAN_CONF *)(sendbuff+sizeof(MSG_HEAD_485));
		memcpy(VLAN_save, onuVLANConfInfo, sizeof(ONU_VLAN_CONF));
		
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
		{	
				/* Got the response*/
			if(headRsp->rsp != 0)
			{
				logger ( LOG_ERR ,"Error: save onu VLAN error response !,  error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				
					
			if(headRsp->length != 0)
			{
				logger ( LOG_ERR ,"Error: save onu VLAN response length error!,  length:%d, expect:%d.\n",	headRsp->length, 0);
				return(ERROR_COMM_PARSE);
			}
				
			return 0;
		}
		
		return(ERROR_COMM_485);
	
	}

int DelONUVLAN (unsigned char  card_no, unsigned char  node_id, ONU_VLAN_CONF *onuVLANConfInfo )
{
	char			sendbuff[MAX_485MSG_FULLLEN];
	char			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485	*headReq, *headRsp;
	ONU_VLAN_CONF *VLAN_del;
	
		
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_DELVLAN;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(ONU_VLAN_CONF);
			
	headReq->dst = card_no;
	
	VLAN_del = (ONU_VLAN_CONF *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(VLAN_del, onuVLANConfInfo, sizeof(ONU_VLAN_CONF));
		
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
				/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: delete onu VLAN table error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
					
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: delete onu VLAN table response length error!,  length:%d, expect:%d.\n",	headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
				
		return 0;
	}
	return(ERROR_COMM_485);
	
}

//add by yp
int DelGE8VLAN (unsigned char  card_no, unsigned char  node_id, ONU_VLAN_CONF *onuVLANConfInfo )
{
	char			sendbuff[MAX_485MSG_FULLLEN];
	char			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485	*headReq, *headRsp;
	ONU_VLAN_CONF *VLAN_del;
	
		
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_GE8_DELVLAN;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(ONU_VLAN_CONF);
			
	headReq->dst = card_no;
	
	VLAN_del = (ONU_VLAN_CONF *)(sendbuff+sizeof(MSG_HEAD_485));
	memcpy(VLAN_del, onuVLANConfInfo, sizeof(ONU_VLAN_CONF));
		
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSGLONG))
	{	
				/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: delete onu VLAN table error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
					
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: delete onu VLAN table response length error!,  length:%d, expect:%d.\n",	headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
		return 0;
	}
	
	return(ERROR_COMM_485);
	
}



int  SetFSA600 (unsigned char  card_no, unsigned char node_id, FSA600_SET_485 *data)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FSA600_SET_485 *fsa600set;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc,nrow, ncol;
	char sql_str[256];
	char **result;	

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FSA600_SET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(FSA600_SET_485);
		
	headReq->dst = card_no;
	fsa600set = (FSA600_SET_485 *)(sendbuff+sizeof(MSG_HEAD_485));
	/*consider snmpset........*/
	//memcpy(fsa600set, &FSA600_Data[card_no][node_id], sizeof(FSA600_SET_485));
	fsa600set->PortState = FSA600_Data[card_no][node_id].PortState;
	fsa600set->TransmitMode = FSA600_Data[card_no][node_id].TransmitMode;
	fsa600set->UpStream = FSA600_Data[card_no][node_id].UpStream;
	fsa600set->UpStreamFlag = FSA600_Data[card_no][node_id].UpStreamFlag;
	fsa600set->DownStream = FSA600_Data[card_no][node_id].DownStream;
	fsa600set->DownStreamFlag = FSA600_Data[card_no][node_id].DownStreamFlag;
	
	if ( data->setFlag & MD_FSA600_PORTSTATE )
	{
		fsa600set->PortState = data->PortState;
		logger (LOG_DEBUG, "set fsa600 portstate excute here? portstate is %d - %d  \n", data->PortState, fsa600set->PortState);
	}

	if ( data->setFlag & MD_FSA600_TRANSMITMODE)
	{
		fsa600set->TransmitMode = data->TransmitMode;
		
		logger (LOG_DEBUG, "set fsa600 transmitmode excute here? transmitmode is %d - %d  \n", data->TransmitMode, fsa600set->TransmitMode);
	}

	if ( data->setFlag & MD_FSA600_UPSTREAM )
	{
		fsa600set->UpStream = data->UpStream;
		fsa600set->UpStreamFlag = data->UpStreamFlag;
		logger (LOG_DEBUG, "set fsa600 upstream excute here? upstream is %d - %d  \n",  data->UpStream , fsa600set->UpStream);
		logger (LOG_DEBUG, "set fsa600 upstreamflag excute here? UpStreamFlag is %d - %d  \n",  data->UpStreamFlag , fsa600set->UpStreamFlag);
	}
	if ( data->setFlag & MD_FSA600_DOWNSTREAM )
	{
		fsa600set->DownStream = data->DownStream;
		fsa600set->DownStreamFlag = data->DownStreamFlag;
		logger (LOG_DEBUG, "set fsa600 DownStream excute here? DownStream is %d - %d  \n",  data->DownStream , fsa600set->DownStream);
		logger (LOG_DEBUG, "set fsa600 DownStreamFlag excute here? DownStreamFlag is %d - %d  \n",  data->DownStreamFlag , fsa600set->DownStreamFlag);
	}

	if (  0xFF ==  data->setFlag  )
	{
		memcpy(fsa600set, data, sizeof(FSA600_SET_485));
		logger (LOG_DEBUG, "set fsa600 all excute here?\n");
	}
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff ,TIMEOUT_485MSG ))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: setfsa600  error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: setfsa600  response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			

		
		/*updata config database*/
		rc = sqlite3_open(CONF_DB, &db);
		if (rc)
		{
			logger ( LOG_ERR , "Can't open database: %s\n", sqlite3_errmsg(db));
			return ERROR;
		}


		sprintf(sql_str, "select CardNo from FSA600 where CardNo=%d and localIndex=%d ", card_no, node_id );
		rc = sqlite3_get_table(db,
						sql_str,
						&result,
						&nrow,
						&ncol,
						&zErrMsg);

		if (rc != SQLITE_OK)
		{
			logger (LOG_ERR , "select cardno err for updata fsa600 :%s\n%s\n", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;
		}
		if (nrow < 1)
		{
			sqlite3_free_table(result);
			
			sprintf(sql_str, "insert into FSA600 (CardNo ,localIndex, portState , transmitmMode, upStreamFlag, upStream, downStreamFlag,downStream, resetFlag) values (%d,%d,%d,%d,%d,%d,%d,%d,%d)", card_no, node_id, fsa600set->PortState, fsa600set->TransmitMode, fsa600set->UpStreamFlag, fsa600set->UpStream, fsa600set->DownStreamFlag, fsa600set->UpStream, 1);
			
			rc = sqlite3_exec(db,
						sql_str,
						0,
						0,
						&zErrMsg);
			if (rc != SQLITE_OK)
			{
				logger (LOG_ERR , "insert into FSA600 err:%s\n%s\n", sql_str, zErrMsg);
				sqlite3_close(db);
				return ERROR;
			}
			sqlite3_close(db);
			return 0;
			

			
		}
		else
		{
			sqlite3_free_table(result);
			
			sprintf (sql_str, "update FSA600 set portState=%d , transmitmMode=%d, upStreamFlag=%d, upStream=%d, downStreamFlag=%d,downStream=%d, resetFlag=%d where CardNo=%d and localIndex=%d ",fsa600set->PortState, fsa600set->TransmitMode, fsa600set->UpStreamFlag, fsa600set->UpStream, fsa600set->DownStreamFlag, fsa600set->UpStream, 1, card_no, node_id);

			rc = sqlite3_exec(db,
						sql_str,
						0,
						0,
						&zErrMsg);
			if (rc != SQLITE_OK)
			{
				logger (LOG_ERR , "update  FSA600 err:%s\n%s\n", sql_str, zErrMsg);
				sqlite3_close(db);
				return ERROR;
			}
			sqlite3_close(db);
			return 0;
			

		}
		
				
	}
	return(ERROR_COMM_485);



}


int SetOnu100 (unsigned char card_no, unsigned char node_id ,  FSA600_SETRMT *data)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FSA600_SETRMT *setonu100;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc, nrow, ncol;
	char sql_str[256];
	char **result;	

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FSA600_SETRMT;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = sizeof(FSA600_SETRMT);
		
	headReq->dst = card_no;
	setonu100 = (FSA600_SETRMT *)(sendbuff+sizeof(MSG_HEAD_485));

	/*consider snmpset........*/
	setonu100->TransmitMode = RMT_Data[card_no][node_id].TransmitMode;
	setonu100->CfgWorkMode1 = RMT_Data[card_no][node_id].CfgWorkMode1;
	setonu100->CfgWorkMode2 = RMT_Data[card_no][node_id].CfgWorkMode2;
	memcpy(setonu100->onu_name, RMT_Data[card_no][node_id].onu_name, MAX_DISPLAYNAME_LEN);
	setonu100->OnuId = node_id;
	
	if ( data->setFlag & MD_ONU100_TRANSMITMODE )
	{
		setonu100->TransmitMode = data->TransmitMode;
		logger (LOG_DEBUG, "set 0nu100 TransmitMode excute here? TransmitMode is %d - %d  \n", data->TransmitMode, setonu100->TransmitMode);
	}

	if ( data->setFlag & MD_ONU100_CFGWORKMODE1 )
	{
		setonu100->CfgWorkMode1 = data->CfgWorkMode1;
		logger (LOG_DEBUG, "set 0nu100 CfgWorkMode1 excute here? CfgWorkMode1 is %d - %d  \n", data->CfgWorkMode1, setonu100->CfgWorkMode1);
	}

	if ( data->setFlag & MD_ONU100_CFGWORKMODE2 )
	{
		setonu100->CfgWorkMode2 = data->CfgWorkMode2;
		logger (LOG_DEBUG, "set 0nu100 CfgWorkMode2 excute here? CfgWorkMode2 is %d - %d  \n", data->CfgWorkMode2, setonu100->CfgWorkMode2);
	}
	
	if ( data->setFlag & MD_ONU100_NAME)
	{
		memcpy(setonu100->onu_name, data->onu_name, MAX_DISPLAYNAME_LEN );
		logger (LOG_DEBUG, "set 0nu100 name  is %s - %s  \n", data->onu_name, setonu100->onu_name );
	}
	
	if (  0xFF ==  data->setFlag  )
	{
		memcpy(setonu100 , data, sizeof(FSA600_SETRMT));
		logger (LOG_DEBUG, "set onu100 all excute here?\n");
	}
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error:  setonu100  error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: setonu100  response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
			return(ERROR_COMM_PARSE);
		}
			
			/*updata config database*/
		rc = sqlite3_open(CONF_DB, &db);
		if (rc)
		{
			logger ( LOG_ERR , "Can't open database: %s\n", sqlite3_errmsg(db));
			return ERROR;
		}


		sprintf(sql_str, "select CardNo from ONU100 where CardNo=%d and onuId=%d ", card_no, node_id );
		rc = sqlite3_get_table(db,
						sql_str,
						&result,
						&nrow,
						&ncol,
						&zErrMsg);

		if (rc != SQLITE_OK)
		{
			logger (LOG_ERR , "select cardno err for updata onu100 :%s\n%s\n", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;
		}
		if (nrow < 1)
		{
			sqlite3_free_table(result);
			
			sprintf(sql_str, "insert into ONU100 (CardNo ,onuId, OnuName , transmitmMode, cfgWorkMode1, cfgWorkMode2, resetFlag) values (%d,%d, '%s',%d,%d,%d,%d)", card_no, node_id,setonu100->onu_name, setonu100->TransmitMode, setonu100->CfgWorkMode1, setonu100->CfgWorkMode2, 1);
			
			rc = sqlite3_exec(db,
						sql_str,
						0,
						0,
						&zErrMsg);
			if (rc != SQLITE_OK)
			{
				logger (LOG_ERR , "insert into ONU100 err:%s\n%s\n", sql_str, zErrMsg);
				sqlite3_close(db);
				return ERROR;
			}
			sqlite3_close(db);
			return 0;
			

			
		}
		else
		{
			sqlite3_free_table(result);
			
			sprintf (sql_str, "update ONU100 set OnuName='%s',  transmitmMode=%d, cfgWorkMode1=%d, cfgWorkMode2=%d,  resetFlag=%d where CardNo=%d and onuId=%d ", setonu100->onu_name ,setonu100->TransmitMode, setonu100->CfgWorkMode1, setonu100->CfgWorkMode2, 1, card_no, node_id);

			rc = sqlite3_exec(db,
						sql_str,
						0,
						0,
						&zErrMsg);
			if (rc != SQLITE_OK)
			{
				logger (LOG_ERR , "update  ONU100 err:%s\n%s\n", sql_str, zErrMsg);
				sqlite3_close(db);
				return ERROR;
			}
			sqlite3_close(db);
			return 0;
			

		}
	}
	return(ERROR_COMM_485);

}


int FSA600resetAndDFT (unsigned char card_no, unsigned char node_id , unsigned char code)
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;

	sqlite3 *db;
	char *zErrMsg = 0;
	int rc,  nrow, ncol;
	char sql_str[256];
	char **result;	

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = code;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = 0;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error: FSA600resetAndDFT error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: FSA600resetAndDFT response length error!,  length:%d\n",  headRsp->length);
			return(ERROR_COMM_PARSE);
		}
			
		if (( C_FSA600_RESET == code) || ( C_FSA600_RESETRMT == code )) 
			return 0;
		
		/*updata resetflag in DB.*/
		rc = sqlite3_open(CONF_DB, &db);
		if (rc)
		{
			logger ( LOG_ERR , "Can't open database: %s\n", sqlite3_errmsg(db));
			return ERROR;
		}

		if (  C_FSA600_SETDFT == code )
		{
			sprintf(sql_str, "select CardNo from FSA600 where CardNo=%d and localIndex=%d ", card_no, node_id );
		}
		 if (  C_FSA600_SETDFTRMT == code )
		{
			sprintf(sql_str, "select CardNo from ONU100 where CardNo=%d and onuId=%d ", card_no, node_id );

		}
		
		rc = sqlite3_get_table(db, sql_str, &result, &nrow, &ncol, &zErrMsg);

		if (rc != SQLITE_OK)
		{
			logger (LOG_ERR , "select cardno err for FSA600resetAndDFT :%s\n%s\n", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;
		}
		if (nrow < 1)
		{
			sqlite3_free_table(result);
			 if ( C_FSA600_SETDFT == code  )
			{
				sprintf(sql_str, "insert into FSA600 (CardNo ,localIndex,   resetFlag) values (%d,%d,%d)", card_no, node_id,  2);
			}
			
			 if ( C_FSA600_SETDFTRMT == code )
			{
				sprintf(sql_str, "insert into ONU100 (CardNo ,onuId,   resetFlag) values (%d,%d,%d)", card_no, node_id,  2);
			}
			
				
			rc = sqlite3_exec(db,
							sql_str,
							0,
							0,
							&zErrMsg);
			if (rc != SQLITE_OK)
			{
				logger (LOG_ERR , "insert  err:%s\n%s\n", sql_str, zErrMsg);
				sqlite3_close(db);
				return ERROR;
			}
			sqlite3_close(db);
			return 0;
	
		}
		else
		{
			sqlite3_free_table(result);
			 if ( C_FSA600_SETDFT == code  )
			{
				sprintf(sql_str, "update FSA600 set resetFlag=%d where CardNo=%d and localIndex=%d", 2, card_no, node_id  );
			}
			if ( C_FSA600_SETDFTRMT == code )
			{
				sprintf(sql_str, "update ONU100 set resetFlag=%d where CardNo=%d and onuId=%d", 2, card_no, node_id  );
			}
				
			
			rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
			if (rc != SQLITE_OK)
			{
				logger (LOG_ERR , "update  ONU100 err:%s\n%s\n", sql_str, zErrMsg);
				sqlite3_close(db);
				return ERROR;
			}
			sqlite3_close(db);
			return 0;
				

		}
		
	}
	return(ERROR_COMM_485);
}

int SetOnu(unsigned char card_no, unsigned char node_id, ONU_SET *data)//onu100 and onu400 ??
{

	if ( CARD_TYPE_FSA600 == cardData[card_no].card_type )//update onu100 name
	{
		sqlite3 *db;
		char *zErrMsg = 0;
		int rc,  nrow, ncol;
		char sql_str[256];
		char **result;
		logger (LOG_DEBUG, "start to update onu100 name.\n");
		rc = sqlite3_open(CONF_DB, &db);
		if (rc)
		{
			logger ( LOG_ERR , "Can't open database: %s\n", sqlite3_errmsg(db));
			return ERROR;
		}

		/*to find in the config  database whether the card is already exist,if the card
		   already exists,update it,otherwise insert a new column.*/
							   
	    sprintf(sql_str, "select OnuName from ONU100 where CardNo=%d and onuId=%d", card_no, node_id);
		rc = sqlite3_get_table(db,
						sql_str,                /* SQL to be executed */
						&result,			/* Result written to a char *[]  that this points to */
						&nrow,			/* Number of result rows written here */
						&ncol,			/* Number of result columns written here */
						&zErrMsg		/* Error msg written here */);
		if (rc !=SQLITE_OK)
		{
			logger  ( LOG_ERR ,"DB Error:%s\n", zErrMsg);
			sqlite3_close(db);
			return ERROR ;
									
		}

		if (nrow < 1)
		{ /* no onu100 config exist*/
			sqlite3_free_table (result);
			sprintf(sql_str, "insert into ONU100(CardNo, onuId, OnuName) values (%d, %d,'%s')", card_no,node_id ,data->onu_name );
			rc = sqlite3_exec(db,
						sql_str,              
						0,			
						0,		
						&zErrMsg);

		if (rc !=SQLITE_OK)
		{
			logger  (LOG_ERR , "DB insert ONU100 config info Error:%s\n", zErrMsg);
			sqlite3_close(db);
			return ERROR ;
									
		}
		sqlite3_close(db);
		memcpy(&onu100_Info[card_no][node_id].onu_name ,  data->onu_name ,MAX_DISPLAYNAME_LEN );
		logger (LOG_DEBUG, "update onu100 success.\n");
		return 0;
									
		}
		else{

			/*the onu100 config exist update it */
			sqlite3_free_table (result);
			sprintf(sql_str, "update ONU100 set  OnuName='%s' where CardNo=%d and onuId=%d ",  data->onu_name, card_no,node_id);
			rc = sqlite3_exec(db,
						sql_str,             
						0,			
						0,			
						&zErrMsg);

			if (rc !=SQLITE_OK)
			{
				logger  (LOG_ERR , "DB update onu100Name Error:%s\n", zErrMsg);
				sqlite3_close(db);
				return ERROR;
									
			}
			sqlite3_close(db);
			memcpy(&onu100_Info[card_no][node_id].onu_name ,  data->onu_name , MAX_DISPLAYNAME_LEN );
			logger (LOG_DEBUG, "update onu100 success.\n");
			return 0;
									

		}

		return ERROR;

			
	}
	else {
		
		char 			sendbuff[MAX_485MSG_FULLLEN];
		char 			recvbuff[MAX_485MSG_FULLLEN];
		MSG_HEAD_485 	*headReq, *headRsp;
		ONU_SET *onu_set;

		memset(sendbuff, 0, sizeof(MSG_HEAD_485));
		headReq = (MSG_HEAD_485*)sendbuff;
		headRsp = (MSG_HEAD_485*)recvbuff;
		headReq->code = C_ONU_SET;
		headReq->src = CTRL_CARDNO;
		headReq->onu_nodeid = node_id;
		headReq->length = sizeof(ONU_SET);
			
		headReq->dst = card_no;
		onu_set = (ONU_SET *)(sendbuff+sizeof(MSG_HEAD_485));
		memcpy(onu_set, data, sizeof(ONU_SET));

		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSGLONG))
		{	
				/* Got the response*/
			if(headRsp->rsp != 0)
			{
				logger ( LOG_ERR ,"Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				
					
			if(headRsp->length != 0)
			{
				logger ( LOG_ERR ,"Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, 0);
				return(ERROR_COMM_PARSE);
			}
				
			return 0;
		}
		return(ERROR_COMM_485);

		}
	
}


int updateONUList(unsigned char card_no, unsigned char nodeid, ONU_SET *onuSet)
{

	sqlite3 *db;
	char *zErrMsg = 0;
	char sql_str[256];
	int rc;

	rc = sqlite3_open(DYNAMIC_DB, &db);
	if (rc)
	{
		logger ( LOG_ERR ,"can't open dynamic database:%s\n" , sqlite3_errmsg(db));
		return ERROR;
	
	}

/*
	if (0 == onuSet->setFlag)
	{
		if (onuSet->admin_state == 1)
		{
			sprintf(sql_str, "update ONUList set NodeState=%d, ONUName='%s' where CardNo=%d and NodeId=%d",1,onuSet->onu_name, card_no,nodeid);
		}else{

			sprintf(sql_str, "update ONUList set NodeState=%d, ONUName='%s' where CardNo=%d and NodeId=%d",2,onuSet->onu_name, card_no,nodeid);

		}

		rc = sqlite3_exec(
						db,
						sql_str,
						0,
						0,
						&zErrMsg);
		if (rc !=SQLITE_OK)
		{
			logger ( LOG_ERR ,"update onulist name and nodestate err %s%s\n",sql_str,zErrMsg);
			sqlite3_close(db);
			return ERROR;

		}
		sqlite3_close(db);

		return 0;
	}
*/
	if ( onuSet->setFlag & MD_ONU_ADMIN )
	{
		if (onuSet->admin_state == 1)
			sprintf (sql_str, "update ONUList set NodeState=%d where CardNo=%d and NodeId=%d", 1,card_no, nodeid);
		else
			sprintf(sql_str, "update ONUList set NodeState=%d where CardNo=%d and NodeId=%d", 2, card_no, nodeid);

		rc = sqlite3_exec(db,
						sql_str,
						0,
						0,
						&zErrMsg);
		if (rc != SQLITE_OK)
		{
			logger ( LOG_ERR , "update onulist nodestate err:%s\n%s\n",sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;
		}

	}

	if ( onuSet->setFlag & MD_ONU_NAME)
	{

		sprintf (sql_str, "update ONUList set ONUName='%s' where CardNo=%d and NodeId=%d", onuSet->onu_name, card_no,nodeid);
		logger (LOG_DEBUG, "Update ONU name:%s\n", sql_str );
		rc = sqlite3_exec(db,
						sql_str,
						0,
						0,
						&zErrMsg);
		if (rc != SQLITE_OK)
		{
			logger ( LOG_ERR ,"update onulist name err :%s\n%s\n", sql_str, zErrMsg);
			sqlite3_close(db);
			return ERROR;
		}		
	}

	sqlite3_close(db);
	return 0;

}

int resetONU(unsigned char card_no, unsigned char node_id)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;


	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_RESET;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;
	headReq->length = 0;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		if(headRsp->length != 0)
		{
			logger ( LOG_ERR ,"Error: Get Card response length error!,  length:%d\n",  headRsp->length);
			return(ERROR_COMM_PARSE);
		}
			

		return 0;
	}
	return(ERROR_COMM_485);

}

int genericReq (unsigned char card_no, unsigned char msg_code, unsigned char node_id) 
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;	
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = msg_code;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;	
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error:get err response !card=%d, msg_code=%d, error:%d.\n", card_no, msg_code, headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: response length!card=%d, msg_code=%d, length:%d, expect:%d.\n", card_no, msg_code,  headRsp->length, 0 );
			return(ERROR_COMM_PARSE);
		}
			
		
		return 0;
	}
	return(ERROR_COMM_485);

}

int genericSetReq (unsigned char card_no, unsigned char msg_code, unsigned char node_id, unsigned short send_len, char *send_data) 
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;	
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = msg_code;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;	
	headReq->length = send_len;		
	headReq->dst = card_no;

	memcpy(sendbuff+sizeof(MSG_HEAD_485), send_data, send_len);	
		
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error:get err response !card=%d, msg_code=%d, error:%d.\n", card_no, msg_code, headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != 0)
		{
			logger (LOG_ERR , "Error: response length!card=%d, msg_code=%d, length:%d, expect:%d.\n", card_no, msg_code,  headRsp->length, 0 );
			return(ERROR_COMM_PARSE);
		}
			
		
		return 0;
	}
	return(ERROR_COMM_485);

}


// 发送无数据区的请求
int genericGetReq (unsigned char card_no, unsigned char msg_code, unsigned char node_id, unsigned short *recv_len, char *recv_data) 
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;	
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = msg_code;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;	
	headReq->length = 0;		
	headReq->dst = card_no;

	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error:get err response !card=%d, msg_code=%d, error:%d.\n", card_no, msg_code, headRsp->rsp);
			return(headRsp->rsp);
		}				

		*recv_len = headRsp->length;
		memcpy(recv_data, recvbuff + sizeof(MSG_HEAD_485), headRsp->length);
		
		return 0;
	}
	return(ERROR_COMM_485);

}

// 发送带有数据区的请求
int genericGetReq2 (unsigned char card_no, unsigned char msg_code, unsigned char node_id, unsigned short send_len, char *send_data, unsigned short *recv_len, char *recv_data) 
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	int i, len;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = msg_code;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = node_id;	
	headReq->length = send_len;		
	headReq->dst = card_no;

	memcpy(sendbuff+sizeof(MSG_HEAD_485), send_data, send_len);	

	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
		printf("<-------Receive answer info, msgcode=%d, cardno=%d, nodeid=%d:\n", 
    		msg_code, card_no, node_id);
        len = headRsp->length + sizeof(MSG_HEAD_485);
        if (len > MAX_485MSG_FULLLEN) len =  MAX_485MSG_FULLLEN;
		for (i = 0; i < len; i++)
		    printf("%02X ", (unsigned char)recvbuff[i]);
		printf("------------------------------------------------------->\n");
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR , "Error:get err response !card=%d, msg_code=%d, error:%d.\n", card_no, msg_code, headRsp->rsp);
			return(headRsp->rsp);
		}				

		*recv_len = headRsp->length;
		memcpy(recv_data, recvbuff + sizeof(MSG_HEAD_485), headRsp->length);
		return 0;
	}
	return(ERROR_COMM_485);

}


int getAlarm(unsigned char card_no,  GET_ALARMS_RSP *alarm)
{

	char sendbuff[MAX_485MSG_FULLLEN];
	char recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 *headReq, *headRsp;
	GET_ALARMS_RSP *rsp_alarm;


	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485* )recvbuff;
	headReq->code = GET_ALARMS;
	headReq->dst = card_no;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;


	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger ( LOG_ERR ,"Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				
				
		rsp_alarm = (GET_ALARMS_RSP *)(recvbuff + sizeof(MSG_HEAD_485) );
		if(headRsp->length != rsp_alarm->alarmNum*sizeof(GET_ALARMS_RSP))
		{
			logger ( LOG_ERR ,"Error: Get Card Alarm response length error!,  length:%d, expect:%d.\n",  headRsp->length, rsp_alarm->alarmNum*sizeof(GET_ALARMS_RSP));
			return(ERROR_COMM_PARSE);
		}
			
		memcpy(alarm, rsp_alarm,  rsp_alarm->alarmNum*sizeof(GET_ALARMS_RSP));
		return 0;
	}
	return(ERROR_COMM_485);
}

int GetNttInfo(unsigned char card_no,FE_PORT_STATE *nttportstate)
{
	char sendbuff[MAX_485MSG_FULLLEN];
	char recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 *headReq,*headRsp;
	FE_PORT_STATE *port_send,*fe8_rsp;
	int i;

	headReq=(MSG_HEAD_485*)sendbuff;
	headRsp=(MSG_HEAD_485*)recvbuff;
	headReq->code= C_FE8_PORT_GET;
	headReq->dst = card_no;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	headReq->length = sizeof(FE_PORT_STATE);

	port_send = (FE_PORT_STATE*)(sendbuff + sizeof(MSG_HEAD_485));


	port_send->portIndex = nttportstate->portIndex;

	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
	{	
			/* Got the response*/
			if(headRsp->rsp != 0)
			{
				logger (LOG_ERR , "Error: Get Card error response !,  error:%d.\n",  headRsp->rsp);
				return(headRsp->rsp);
			}				

			if(headRsp->length != sizeof(FE_PORT_STATE))
			{
				logger (LOG_ERR , "Error: Get Card response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(FE_PORT_STATE));
				return(ERROR_COMM_PARSE);
			}
				
			fe8_rsp = (FE_PORT_STATE *) (recvbuff + sizeof(MSG_HEAD_485));
			logger  (LOG_DEBUG , "get mac is %d-%d-%d-%d-%d-%d\n", fe8_rsp->bounded_mac[0], fe8_rsp->bounded_mac[1], fe8_rsp->bounded_mac[2], fe8_rsp->bounded_mac[3], fe8_rsp->bounded_mac[4], fe8_rsp->bounded_mac[5]);	

			memcpy(nttportstate, fe8_rsp, sizeof(FE_PORT_STATE));

			return 0;
	}

	return ERROR_COMM_485;
}


int Ms4Switch(int cardno, char input_video)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_MS4_SWITCH;
	headReq->src   = CTRL_CARDNO;
	headReq->dst = cardno;
	headReq->length = 1;	
	
	*(sendbuff + sizeof(MSG_HEAD_485)) = input_video;
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR, "Error: Ms4Switch !,  error:%d.\n",  headRsp->rsp);
			if(headRsp->rsp == 1)
				return(ERROR_WRONG_ARGS);
			if(headRsp->rsp == 2)
				return(ERROR_I2C);
			if(headRsp->rsp == 3)
				return(ERROR_HDMI);
				
			return(headRsp->rsp);
		}	
		return 0;
	}
	return(ERROR_COMM_485);
}

int Ms4Debug(int cardno, char * in_str, unsigned char in_len, char * out_str, unsigned char *out_len)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_MS4_DEBUG;
	headReq->src   = CTRL_CARDNO;
	headReq->dst = cardno;
	headReq->length = in_len;	
	
	memcpy(sendbuff + sizeof(MSG_HEAD_485), in_str, in_len);
	
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR, "Error: Ms4Debug !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}					
		if(headRsp->length > 64)
			*out_len = 64;
		else
			*out_len = headRsp->length;		
		memcpy(out_str, recvbuff + sizeof(MSG_HEAD_485), *out_len);
		
		return 0;
	}
	return(ERROR_COMM_485);
}

#if 1 // 201708 for MTT
int SendRs232Data(MSG_HEAD_CMD* msg_header, char * buffer, MSG_HEAD_CMD_RSP* msgrsp_header)
{
    int ret = 0;
    //char send_buf[1024];
    char* pdata = buffer + sizeof(MSG_HEAD_CMD);
    
    // 对大于500字节的数据，分多次发送， 重新组织数据
    int data_len = msg_header->length;
    
    while(data_len)
    {
        if (data_len > 500)
            msg_header->length = 500;
        else
            msg_header->length = data_len;
            
        //memcpy(send_buf, msg_header, sizeof(MSG_HEAD_CMD));
        //memcpy(send_buf+ sizeof(MSG_HEAD_CMD), pdata, msg_header->length);
        
        ret = genericSetReq( msg_header->card_no, msgrsp_header->msg_code,
                             msg_header->node_id, msg_header->length, pdata);
        if (ret != 0)
        {
            break;
        }
        
        pdata += msg_header->length;
        data_len -= msg_header->length;
    }
    
    return ret;
}

// 获取前端上传的232数据,每次最多500字节的数据，发送后在上传
int GetRs232Data(MSG_HEAD_CMD* msg_header, char * buffer, MSG_HEAD_CMD_RSP* msgrsp_header)
{
    char* buf = msgrsp_header + sizeof(MSG_HEAD_CMD_RSP);
    unsigned char port = (unsigned char)(buffer + sizeof(MSG_HEAD_CMD)); // 第一个字节是232的端口号，暂未用
    int recv_len = 0;

    buf[0] = port;
    unsigned short send_len = 1;
    int ret = genericGetReq2( msg_header->card_no, msgrsp_header->msg_code, msg_header->node_id, 
                    send_len, &port, &recv_len, buf + 1);
    msgrsp_header->length = recv_len + 1;
    return ret;
}
#endif

int ProcessMsg(char * buffer, int size, int fds)
{
	char response_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	SHELF_INFO *shelfInfo;
	CARD_INFO *	card_info;
	FSA_CARD_STATE * card_fsa;
	CARD_INFO_FE8 *card_fe8;
	GE_PORT_STATE *ge8_portlink;
	GE8_STATISTICS *portstatistics;
	ONU_INFO* onu_info;
	SFP_INFO * sfpState;
	SFP_INFO *sfpIndex;
	FE_PORT_STATE *etherPortState;
	FE_PORT_STATE *etherPortStaterecv;
	PORT_PERF *perfState;
	FE_PORTINDEX *feportindexrecv;
	CARD_SET 	*cardSet;
	CARD_SET_485FE  *fe8card_set;
	FE_PORT_STATE *fe8portstaterecv;
	BROADCASTSTORM_CTRL_FE *broadcastStormInfo;
	FE_PORT_STATE *fe8portstate;
	CARD_SET_FSA *fsacard_set;
	ONU_SET		*onuSet;
	ETHER_PORT_SET *etherportSet;
	FE_PORTINDEX *etherport_index;
	ONU_LIST_ENTRY *ponulist;
	GET_ALARMS_RSP *alarm;
	ENCODE_VIDEO_STATE *videostate;
	ENCODE_NET_STATE *net_state;
	ENCODE_PROTOCOL_STATE *protocol_state;
	ENCODE_OSD_STATE *osd_state;
	ENCODE_PICTURE_STATE *picture_state;
	ENCODE_SYSTIME *sys_time;
	ENCODE_MODE_STATE *patten;
	ENCODE_NTP *sys_ntp;
	ENCODE_CTRL *control;
	FSA600_GETRMT *getremote;
	FSA600_PORT_485  *fsa600_state;
	FSA600_SET_485  *fsa600_set;
	FSA600_SETRMT *onu100_set;
	PORT_COUNT *portcount;
	ONU_UART_CONF *onu_uart;
	ONU_IO *onu_io;
	ONU_SETID *onu_setid;
	ONU_PORTMIRROR_CONF *onuPortMirrorConfInfo;
	ONU_QoS_CONF *onuQoSConfInfo;
	ONU_VLAN_CONF *onuVLANConfInfo,*GE8_vlan_conf;
	SW_PORT_COUNT *swcount;
	SW_PORT_STATE *swport;
	SW_VLAN *vlan;
	SW_MC_STATE *mcg;
	SW_CONFIG *swconfig;
	GE8_IGMP  *igmp_yp;
	GE8_NET_STATE *ge8_net_state;
	NTT_STATE  *ntt_state;
#if 1	 // 201611
	MTT_STATE  *mtt_state;
#endif

#if 1 // 201708
    MTT_411_STATE  *mtt_411_state;
    MTT_441_STATE  *mtt_441_state;
#endif

	FE_PORT_STATE *ntt_info,*nttsend;	
 	CARD_SET_485FE *nttcard_set;
	
	int *tnum, i;
	unsigned char *totalnum;
	unsigned char msg_len;
	
	int rslt_len, snd_size, ret,  updateFlag = 0;/*0 is not update, 1 is used for FE8 snmp config,2 is used for update ONUList */

	msg_header = (MSG_HEAD_CMD *) buffer;
	msgrsp_header = (MSG_HEAD_CMD_RSP *) response_buff;

	/* check the message length */

	if(size != msg_header->length + sizeof(MSG_HEAD_CMD)) {
		logger (LOG_ERR , "Message data length error !\n");
		return -1;
	}

	memset(msgrsp_header, 0, sizeof(MSG_HEAD_CMD_RSP));
	msgrsp_header->msg_code = msg_header->msg_code;
	msgrsp_header->card_no = msg_header->card_no;
	msgrsp_header->node_id = msg_header->node_id;

	logger (LOG_DEBUG , "Process message:0x%02x\n", msgrsp_header->msg_code);

	/* dump the message */
	/*for(i=0; i< msg_header->length; i++)
	{
		printf("%x ",  buffer[sizeof(MSG_HEAD_CMD) + i]);
		if( (i % 8) == 7)
			printf("\n");
	}*/
	
	switch(msgrsp_header->msg_code){
		case C_SHELF_GET:
			shelfInfo = (SHELF_INFO*) (response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetShelfInfo(shelfInfo);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(SHELF_INFO);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_SHELF_SET:
			if(msg_header->length != sizeof(SHELF_INFO))
			{
				logger (LOG_ERR , "Error: C_SHELF_SET, length error:\n");
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			shelfInfo = (SHELF_INFO*)(buffer+sizeof(MSG_HEAD_CMD));
			ret = SetShelf(shelfInfo);
			if(ret==0){
				msgrsp_header->rsp_code = 0;
			}
			else{
				msgrsp_header->rsp_code =ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_CARD_GET:
			card_info = (CARD_INFO*) (response_buff + sizeof(MSG_HEAD_CMD_RSP));
			card_info->card_no = msg_header->card_no;
			ret = GetCardInfo(card_info);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(CARD_INFO);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_FSA_GET:
			card_fsa = (FSA_CARD_STATE*)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetCardFsa(msg_header->card_no, card_fsa);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(FSA_CARD_STATE);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GET:
			onu_info = (ONU_INFO*)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetOnuInfo(msg_header->card_no, msg_header->node_id, onu_info);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(ONU_INFO);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GETLIST:
			ponulist = (ONU_LIST_ENTRY*)(response_buff + sizeof(MSG_HEAD_CMD_RSP) +1);
			ret = GetOnuList( msg_header->card_no,  ponulist);
			*(response_buff + sizeof(MSG_HEAD_CMD_RSP)) = (char) ret;
			msgrsp_header->rsp_code = 0;
			msgrsp_header->length = ret * sizeof(ONU_LIST_ENTRY) + 1;
			break;		
		case C_SFP_GET:
			sfpIndex = (SFP_INFO *)(buffer + sizeof (MSG_HEAD_CMD));
			sfpState = (SFP_INFO*)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			logger (LOG_DEBUG ,"get sfp index is %d\n", sfpIndex->sfpIndex);
			ret = GetSfpInfo( msg_header->card_no, msg_header->node_id,sfpIndex->sfpIndex, sfpState);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(SFP_INFO);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_FSA600_INFO:
			fsa600_state = (FSA600_PORT_485 *)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetFsa600Info ( msg_header->card_no, msg_header->node_id, fsa600_state);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(FSA600_PORT_485);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_FSA600_GETRMT:
			
			getremote = (FSA600_GETRMT *)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetFSA600RMT ( msg_header->card_no, msg_header->node_id, getremote);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(FSA600_GETRMT);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GETDATACH:
			ret = GetUartInfo(msg_header->card_no, msg_header->node_id, response_buff + sizeof(MSG_HEAD_CMD_RSP), &rslt_len);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = rslt_len;
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GETDSN_LIST:
			ret = GetONUDSNList(msg_header->card_no, msg_header->node_id, response_buff + sizeof(MSG_HEAD_CMD_RSP), &rslt_len);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = rslt_len;
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GETIO:
			onu_io = (ONU_IO*)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetIO(msg_header->card_no, msg_header->node_id, onu_io );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(ONU_IO);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GETPORTMIRROR:
			onuPortMirrorConfInfo = (ONU_PORTMIRROR_CONF *)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetPortMirror(msg_header->card_no, msg_header->node_id, onuPortMirrorConfInfo );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(ONU_PORTMIRROR_CONF);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GETQoS:
			onuQoSConfInfo = (ONU_QoS_CONF *)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetQoS(msg_header->card_no, msg_header->node_id, onuQoSConfInfo );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(ONU_QoS_CONF);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ONU_GETVLAN:
			totalnum = (unsigned char *)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			onuVLANConfInfo = (ONU_VLAN_CONF *)(response_buff + sizeof(MSG_HEAD_CMD_RSP) + sizeof(unsigned char));
			ret = GetVLAN(msg_header->card_no, msg_header->node_id, onuVLANConfInfo, totalnum);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = (*totalnum) * sizeof(ONU_VLAN_CONF) + sizeof(unsigned char);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ENCODE_VIDEO_GET:
			videostate = (ENCODE_VIDEO_STATE *)(response_buff + sizeof (MSG_HEAD_CMD_RSP));
			ret = GetVideoState( msg_header->card_no, msg_header->node_id, videostate);
			if ( ret == 0 )
			{
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(ENCODE_VIDEO_STATE);
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_GE8_NET_GET:
			//printf("GE8_NET\r\n");
			ge8_net_state = (GE8_NET_STATE *)(response_buff + sizeof (MSG_HEAD_CMD_RSP));
			ret = GetNetworkStateGe8( msg_header->card_no, msg_header->node_id, ge8_net_state);
			//logger(LOG_ERR,"mac%s,\n",net_state->mac);			
			if ( ret == 0 )
			{
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(GE8_NET_STATE);
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_GE8_PORTMODE:
			//logger(LOG_ERR,"portmode");
			ge8_portlink=(GE_PORT_STATE *)(response_buff + sizeof (MSG_HEAD_CMD_RSP));
			ret=GetGe8PortLink(msg_header->card_no, msg_header->node_id,ge8_portlink);
			if ( ret == 0 )
			{
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = 8*sizeof(GE_PORT_STATE);
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_GE8_NET_SET:
			//logger (LOG_ERR , "Error: C_GE8_NET_SET,  error:\n");		
			if(msg_header->length != sizeof(GE8_NET_STATE))
			{
				logger (LOG_ERR , "Error: C_GE8_NET_SET, length error:\n");
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			ge8_net_state = (GE8_NET_STATE *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetNetworkGE8 (msg_header->card_no,  ge8_net_state);
			if(ret == 0) {
				//logger (LOG_ERR , "Error: C_GE8_NET_SET, ret error:\n");				
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_GE8_GETVLAN:
			totalnum = (unsigned char *)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			GE8_vlan_conf = (ONU_VLAN_CONF *)(response_buff + sizeof(MSG_HEAD_CMD_RSP) + sizeof(unsigned char));
			ret = GetVLANge8(msg_header->card_no, msg_header->node_id, GE8_vlan_conf, totalnum);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = (*totalnum) * sizeof(ONU_VLAN_CONF) + sizeof(unsigned char);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			
			break;	
		case C_GE8_DELVLAN:
			if (msg_header->length != sizeof(ONU_VLAN_CONF))
			{
				logger (LOG_ERR , "Error: C_GE8_DELVLAN length error:\n");
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			GE8_vlan_conf = (ONU_VLAN_CONF *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = DelGE8VLAN (msg_header->card_no, msg_header->node_id, GE8_vlan_conf);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;

		case C_GE8_SAVEVLAN:
			
			if (msg_header->length != sizeof(ONU_VLAN_CONF))
			{
			//	logger (LOG_ERR , "Error: C_GE8_DELVLAN length error:\n");
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			//logger (LOG_ERR , "Error: C_GE8_SAVEVLAN1\n");
			//logger(LOG_ERR,"dest no %d\n",msg_header->card_no);
			GE8_vlan_conf = (ONU_VLAN_CONF *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = SaveGE8VLAN (msg_header->card_no, msg_header->node_id, GE8_vlan_conf);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;

		case C_GE8_GETPORT:
			if (msg_header->length != sizeof(GE_PORT_STATE))
			{
			//	logger (LOG_ERR , "Error: C_GE8_DELVLAN length error:\n");
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			ge8_portlink=(GE_PORT_STATE *)(buffer + sizeof (MSG_HEAD_CMD));
			ret=GetGe8Port(msg_header->card_no, msg_header->node_id,ge8_portlink);
			if ( ret == 0 )
			{
				memcpy(response_buff + sizeof(MSG_HEAD_CMD_RSP),ge8_portlink,sizeof(GE_PORT_STATE));
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(GE_PORT_STATE);
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_GE8_SETPORT:
			if (msg_header->length != sizeof(GE_PORT_STATE))
			{
			//	logger (LOG_ERR , "Error: C_GE8_DELVLAN length error:\n");
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			ge8_portlink=(GE_PORT_STATE *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = SetGe8Port(msg_header->card_no, msg_header->node_id,ge8_portlink);
			if ( ret == 0 )
			{
				//memcpy(response_buff + sizeof(MSG_HEAD_CMD_RSP),ge8_portlink,sizeof(GE_PORT_STATE));
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = 0;
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_GE8_STATISTICS:
			if (msg_header->length !=  8*sizeof(GE8_STATISTICS))
			{
			//	logger (LOG_ERR , "Error: C_GE8_DELVLAN length error:\n");
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			portstatistics=(GE8_STATISTICS *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = GetGe8Statistics(msg_header->card_no, msg_header->node_id,portstatistics);
			if ( ret == 0 )
			{
				//memcpy(response_buff + sizeof(MSG_HEAD_CMD_RSP),ge8_portlink,sizeof(GE_PORT_STATE));
				memcpy(response_buff + sizeof(MSG_HEAD_CMD_RSP),portstatistics,8*sizeof(GE8_STATISTICS));
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length =  8*sizeof(GE8_STATISTICS);
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_GE8_STATISTICS_CLR:
			if (msg_header->length !=  0)
			{
			//	logger (LOG_ERR , "Error: C_GE8_DELVLAN length error:\n");
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			ret = ClrGe8Statistics(msg_header->card_no, msg_header->node_id);
			if ( ret == 0 )
			{
					//memcpy(response_buff + sizeof(MSG_HEAD_CMD_RSP),ge8_portlink,sizeof(GE_PORT_STATE));
					//memcpy(response_buff + sizeof(MSG_HEAD_CMD_RSP),portstatistics,8*sizeof(GE8_STATISTICS));
					msgrsp_header->rsp_code = 0;
					msgrsp_header->length =  0;
			}
			else {
					msgrsp_header->rsp_code = ret;
					msgrsp_header->length = 0;
			}
			break;
		case C_GE8_GETIGMP:
			if (msg_header->length !=  0)
			{
			//	logger (LOG_ERR , "Error: C_GE8_DELVLAN length error:\n");
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			igmp_yp = (GE8_IGMP  *)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetGe8IGMP(msg_header->card_no, msg_header->node_id,igmp_yp);
			if ( ret == 0 )
			{
					msgrsp_header->rsp_code = 0;
					msgrsp_header->length =  sizeof(GE8_IGMP);
			}
			else {
					msgrsp_header->rsp_code = ret;
					msgrsp_header->length = 0;
			}
			break;
		case C_GE8_SETIGMP:
			if (msg_header->length !=  sizeof(GE8_IGMP))
			{
			//	logger (LOG_ERR , "Error: C_GE8_DELVLAN length error:\n");
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			igmp_yp = (GE8_IGMP  *)(buffer + sizeof(MSG_HEAD_CMD));
			ret = SetGe8IGMP(msg_header->card_no, msg_header->node_id,igmp_yp);
			if ( ret == 0 )
			{
					msgrsp_header->rsp_code = 0;
					msgrsp_header->length =  0;
			}
			else {
					msgrsp_header->rsp_code = ret;
					msgrsp_header->length = 0;
			}
			break;
		case C_ENCODE_NET_GET:
			net_state = (ENCODE_NET_STATE *)(response_buff + sizeof (MSG_HEAD_CMD_RSP));
			ret = GetNetworkState( msg_header->card_no, msg_header->node_id, net_state);
			if ( ret == 0 )
			{
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(ENCODE_NET_STATE);
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ENCODE_PROTOCOL_GET:
			protocol_state = (ENCODE_PROTOCOL_STATE *)(response_buff + sizeof (MSG_HEAD_CMD_RSP));
			ret = GetPROTOCOLState( msg_header->card_no, msg_header->node_id, protocol_state);
			if ( ret == 0 )
			{
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(ENCODE_PROTOCOL_STATE);
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ENCODE_OSD_GET:
			osd_state = (ENCODE_OSD_STATE *)(response_buff + sizeof (MSG_HEAD_CMD_RSP));
			ret = GetOSDState( msg_header->card_no, msg_header->node_id, osd_state);
			if ( ret == 0 )
			{
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(ENCODE_OSD_STATE);
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ENCODE_PICTURE_GET:
			picture_state = (ENCODE_PICTURE_STATE *)(response_buff + sizeof (MSG_HEAD_CMD_RSP));
			ret = GetPICTUREState( msg_header->card_no, msg_header->node_id, picture_state);
			if ( ret == 0 )
			{
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(ENCODE_PICTURE_STATE);
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ENCODE_SYSTIME_GET:
			sys_time = (ENCODE_SYSTIME *)(response_buff + sizeof (MSG_HEAD_CMD_RSP));
			ret = GetSysTime( msg_header->card_no, sys_time);
			if ( ret == 0 )
			{
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(ENCODE_SYSTIME);
			}
			else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ENCODE_NTP_GET:
			sys_ntp = ( ENCODE_NTP *) ( response_buff + sizeof (MSG_HEAD_CMD_RSP));
			ret = GetNTP ( msg_header->card_no, sys_ntp);
			if ( 0 == ret )
			{

				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof ( ENCODE_NTP);
			}
			else 
			{
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ENCODE_MODE_GET:
			patten = ( ENCODE_MODE_STATE *) ( response_buff + sizeof (MSG_HEAD_CMD_RSP));
			ret = getPatten(msg_header->card_no , msg_header->node_id ,patten);
			if ( 0 == ret )
			{

				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof ( ENCODE_MODE_STATE);
			}
			else 
			{
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_ENCODE_SYSTIME_SET:
			if(msg_header->length != sizeof(ENCODE_SYSTIME))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			sys_time = (ENCODE_SYSTIME *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetSysTime(msg_header->card_no,  sys_time);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_ENCODE_NTP_SET:
			
			if(msg_header->length != sizeof(ENCODE_NTP))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			sys_ntp = (ENCODE_NTP *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetNTP(msg_header->card_no,  sys_ntp);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_ENCODE_NET_SET:
			if(msg_header->length != sizeof(ENCODE_NET_STATE))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			net_state = (ENCODE_NET_STATE *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetNetwork (msg_header->card_no,  net_state);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_ENCODE_PROTOCOL_SET:
			if(msg_header->length != sizeof(ENCODE_PROTOCOL_STATE))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			protocol_state = (ENCODE_PROTOCOL_STATE *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetPROTOCOL (msg_header->card_no,  protocol_state);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_ENCODE_VIDEO_SET:
			if(msg_header->length != sizeof(ENCODE_VIDEO_STATE))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			videostate = (ENCODE_VIDEO_STATE *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetVIDEO (msg_header->card_no,  videostate);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_ENCODE_OSD_SET:
			if(msg_header->length != sizeof(ENCODE_OSD_STATE))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			osd_state = (ENCODE_OSD_STATE *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetOSD (msg_header->card_no,  osd_state);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_ENCODE_PICTURE_SET:
			if(msg_header->length != sizeof(ENCODE_PICTURE_STATE))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			picture_state = (ENCODE_PICTURE_STATE *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetPicture (msg_header->card_no,  picture_state);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_ENCODE_MODE_SET:
			if(msg_header->length != sizeof(ENCODE_MODE_STATE))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			patten = (ENCODE_MODE_STATE *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetPatten (msg_header->card_no,  patten);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		/*case C_ENCODE_CTRL:
			if(msg_header->length != sizeof(ENCODE_CTRL))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			control = (ENCODE_CTRL *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = encodeCTRL (msg_header->card_no,  control);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;*/
		case C_FE8_GET:
			card_fe8 = (CARD_INFO_FE8*)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetCardFE8(msg_header->card_no, card_fe8);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(CARD_INFO_FE8);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_FE8_BROADCASTSTORM_GET:
			broadcastStormInfo = (BROADCASTSTORM_CTRL_FE*)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetFE8BroadcastStormInfo(msg_header->card_no, broadcastStormInfo);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(BROADCASTSTORM_CTRL_FE);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_FE8_BROADCASTSTORM_SET:
			
			if (msg_header->length != sizeof(BROADCASTSTORM_CTRL_FE))
			{
				printf("msg_header length %d  sizeof  BROADCASTSTORM_CTRL_FE %d",msg_header->length,sizeof(BROADCASTSTORM_CTRL_FE));
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			broadcastStormInfo = (BROADCASTSTORM_CTRL_FE *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = SetFE8BroadcastStorm(msg_header->card_no, broadcastStormInfo);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_FE8_PORT_GET:
			fe8portstaterecv= (FE_PORT_STATE*)(buffer+sizeof(MSG_HEAD_CMD));			
			fe8portstate = (FE_PORT_STATE*)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			fe8portstate->portIndex = (unsigned char)fe8portstaterecv->portIndex;
		
			ret = GetCardFE8Port(msg_header->card_no, fe8portstate);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(FE_PORT_STATE);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
			
		case C_ETHER_GET:
			if(msg_header->length != sizeof(FE_PORT_STATE))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			etherPortStaterecv = (FE_PORT_STATE*)(buffer+sizeof(MSG_HEAD_CMD));
			etherPortState = (FE_PORT_STATE*)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			etherPortState->portIndex = etherPortStaterecv->portIndex;
			ret = GetEtherportInfo(msg_header->card_no, msg_header->node_id,  etherPortState);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(FE_PORT_STATE);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_CARD_SET:
			if(msg_header->length != sizeof(CARD_SET))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			cardSet = (CARD_SET *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetCard(msg_header->card_no, 0,  cardSet);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_FSA_SET:
			if(msg_header->length != sizeof(CARD_SET_FSA))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			fsacard_set = (CARD_SET_FSA *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetCardFSA(msg_header->card_no,  fsacard_set);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
			
		case C_FE8_SET://for fe8 web
			if(msg_header->length != sizeof(CARD_SET_485FE))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			fe8card_set = (CARD_SET_485FE *) (buffer + sizeof(MSG_HEAD_CMD));
			logger ( LOG_DEBUG ,"raw mac is %d-%d-%d-%d-%d-%d\n", fe8card_set->port.bounded_mac[0], fe8card_set->port.bounded_mac[1], fe8card_set->port.bounded_mac[2], fe8card_set->port.bounded_mac[3], fe8card_set->port.bounded_mac[4], fe8card_set->port.bounded_mac[5]);
			FE8Card[msg_header->card_no][fe8card_set->Index].admin_state =fe8card_set->port.admin_state;
			FE8Card[msg_header->card_no][fe8card_set->Index].portMode = fe8card_set->port.portMode;
			FE8Card[msg_header->card_no][fe8card_set->Index].portWorkingMode = fe8card_set->port.portWorkingMode;
			FE8Card[msg_header->card_no][fe8card_set->Index].bw_alloc_send = fe8card_set->port.bw_alloc_send;
			FE8Card[msg_header->card_no][fe8card_set->Index].bw_alloc_recv = fe8card_set->port.bw_alloc_recv;
			memcpy( FE8Card[msg_header->card_no][fe8card_set->Index].bounded_mac, fe8card_set->port.bounded_mac, 6);

			
			ret = SetCardFE8(msg_header->card_no, fe8card_set);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = 0;
				
				
						
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			
			ret = updateDBFE8Info(msg_header->card_no, fe8card_set->Index);
			if (ret !=0)
			{
				logger ( LOG_ERR , "Debug update FE8config DB err %d", ret);
			}
			
		
			break;
			
		case C_ONU_SET:
			if(msg_header->length != sizeof(ONU_SET))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			onuSet = (ONU_SET *) (buffer + sizeof(MSG_HEAD_CMD));
			logger (LOG_DEBUG, "set onu id is %d, flag=0x%x\n", msg_header->node_id, onuSet->setFlag);
			ret = SetOnu(msg_header->card_no, msg_header->node_id, onuSet);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				if ( CARD_TYPE_FSA600 != cardData[msg_header->card_no].card_type )
				{ 
					updateFlag = 2;
				}
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_FSA600_SET:
			if(msg_header->length != sizeof(FSA600_SET_485))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			fsa600_set = (FSA600_SET_485 *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetFSA600 (msg_header->card_no, msg_header->node_id, fsa600_set);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;
		case C_FSA600_SETRMT:
			
			if(msg_header->length != sizeof(FSA600_SETRMT))
			{
				logger (LOG_DEBUG, "start config onu100.\n len is %d-%d\n", msg_header->length, sizeof(FSA600_SETRMT));
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			onu100_set  = (FSA600_SETRMT *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetOnu100 (msg_header->card_no, msg_header->node_id, onu100_set);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;
		case C_ONU_SETDATACH:
			if (msg_header->length != sizeof(ONU_UART_CONF))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			onu_uart = (ONU_UART_CONF *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = SetOnuUart (msg_header->card_no, msg_header->node_id, onu_uart);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;
		case C_ONU_SETIO:
			if (msg_header->length != sizeof(ONU_IO))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			onu_io = (ONU_IO *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = SetOnuIO (msg_header->card_no, msg_header->node_id, onu_io);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;
		case C_ONU_SET_ONUID:
			if (msg_header->length != sizeof(ONU_SETID))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			onu_setid = (ONU_SETID *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = SetOnuID (msg_header->card_no, msg_header->node_id, onu_setid);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;
		case C_ONU_SETPORTMIRROR:
			if (msg_header->length != sizeof(ONU_PORTMIRROR_CONF))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			onuPortMirrorConfInfo = (ONU_PORTMIRROR_CONF *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = SetOnuPortMirror (msg_header->card_no, msg_header->node_id, onuPortMirrorConfInfo);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;

		case C_ONU_SETQoS:
			if (msg_header->length != sizeof(ONU_QoS_CONF))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			onuQoSConfInfo = (ONU_QoS_CONF *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = SetOnuQoS (msg_header->card_no, msg_header->node_id, onuQoSConfInfo);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;

		case C_ONU_SAVEVLAN:
			if (msg_header->length != sizeof(ONU_VLAN_CONF))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			onuVLANConfInfo = (ONU_VLAN_CONF *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = SaveONUVLAN (msg_header->card_no, msg_header->node_id, onuVLANConfInfo);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;
		case C_ONU_DELVLAN:
			if (msg_header->length != sizeof(ONU_VLAN_CONF))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			onuVLANConfInfo = (ONU_VLAN_CONF *)(buffer + sizeof (MSG_HEAD_CMD));
			ret = DelONUVLAN (msg_header->card_no, msg_header->node_id, onuVLANConfInfo);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			break;
			
		case C_FSA600_SETDFT:
		case C_FSA600_RESET:	
		
		case C_FSA600_SETDFTRMT:
		case C_FSA600_RESETRMT:   /*remained to be improved*/
			
			ret = FSA600resetAndDFT (msg_header->card_no, msg_header->node_id , msg_header->msg_code );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case GET_PORT_PERF:
			feportindexrecv = (FE_PORTINDEX*)(buffer+sizeof(MSG_HEAD_CMD));
			logger (LOG_DEBUG, "load perf Index is %d\n", feportindexrecv->portIndex );
			perfState = (PORT_PERF*)(response_buff + sizeof(MSG_HEAD_CMD_RSP));
			ret = GetPerfState(msg_header->card_no, msg_header->node_id,feportindexrecv->portIndex,perfState);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(PORT_PERF);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case SET_PERF_RESET:
			feportindexrecv = (FE_PORTINDEX*)(buffer+sizeof(MSG_HEAD_CMD));
			ret = ResetPerfState(msg_header->card_no, msg_header->node_id,feportindexrecv->portIndex);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_ETHER_SET:
			logger(LOG_DEBUG, "snmp set here?\n");
			if(msg_header->length != (sizeof(ETHER_PORT_SET) + sizeof(FE_PORTINDEX)))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			etherport_index = (FE_PORTINDEX *)(buffer + sizeof(MSG_HEAD_CMD) + sizeof (ETHER_PORT_SET));
			etherportSet = (ETHER_PORT_SET *) (buffer + sizeof(MSG_HEAD_CMD));
			ret = SetEtherPort(msg_header->card_no,msg_header->node_id, etherport_index->portIndex, etherportSet);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				if ( 0 ==  msg_header->node_id  )//fe8 snnmp update database.
				{
					updateFlag = 1;
				}
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			
			
			break;
		case GET_ALARMS:
			alarm = (GET_ALARMS_RSP*)(response_buff   + sizeof(MSG_HEAD_485));
			ret = getAlarm(msg_header->card_no,  alarm);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = alarm->alarmNum*sizeof(GET_ALARMS_RSP);
			break;
		case REFRESH_CHECK:
			msgrsp_header->length = 0;
			if(msg_header->length !=  sizeof(RERESH_CHECK_REQ))
			{
				logger(LOG_ERR, "Refresh check message length error! return as no refresh need.\n");
				msgrsp_header->rsp_code = 0; /*no change*/
			} else {
				RERESH_CHECK_REQ *refresh_check;
				refresh_check = (RERESH_CHECK_REQ *)(buffer + sizeof(MSG_HEAD_CMD));
				msgrsp_header->rsp_code = getRefreshState(refresh_check->sessionID, refresh_check->type, msg_header->card_no);
			}
			break;		

		case C_FSA_GETCOUNT:
			portcount = (PORT_COUNT *)(response_buff + sizeof(MSG_HEAD_CMD_RSP) );
			ret = getStatisticCount(msg_header->card_no, portcount );
			logger( LOG_DEBUG, "get fsa statistic count return ret %d.\n", ret );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(PORT_COUNT);
				logger( LOG_DEBUG, "get fsa statistic count return successful rspcode %d, rsp len %d.\n", msgrsp_header->rsp_code, msgrsp_header->length );
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;

		case C_FSA_RESETCOUNT:
			ret = ReseStatisticCount(msg_header->card_no);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;

		case C_SW_GETVLAN:
			ret = GetAllVlan(msg_header->card_no);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;

		case C_SW_SAVEVLAN:
		case C_SW_DELVLAN:
			vlan = (SW_VLAN *)(buffer + sizeof(MSG_HEAD_CMD ));
			if (msg_header->length != sizeof(SW_VLAN))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			ret = SaveAndDelVlan(msg_header->card_no, vlan, msg_header->msg_code );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;

		case C_SW_SAVEL3INTF:
		case C_SW_DELL3INTF:
			if (msg_header->length != sizeof(SW_L3INTF))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			ret = UpdateL3Interface((SW_L3INTF *)(buffer + sizeof(MSG_HEAD_CMD )), msg_header->msg_code );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;

		case C_SW_GETALLMC:
			tnum = (int *)(response_buff + sizeof(MSG_HEAD_CMD_RSP ) );
			mcg = (SW_MC_STATE *)(  response_buff + sizeof(MSG_HEAD_CMD_RSP ) + sizeof(int));
 			ret = GetALLMCgroup( msg_header->card_no , mcg, tnum );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = sizeof(int) ;
			/*logger(LOG_DEBUG, "multicast group total number is %d, msgrsp_header->length %d\n", (*tnum), msgrsp_header->length);
			for ( i = 0; i < (*tnum); i++)
			{
				logger(LOG_DEBUG, "msg proc mcg group ip %x, flag %d, bitmap %x i %d\n", mcg[i].grpip, mcg[i].flag, mcg[i].bitmap, i);
			}*/
			break;

		case C_SW_SAVEMCG:
		case C_SW_DELMCG:
			mcg = (SW_MC_STATE *)(buffer + sizeof(MSG_HEAD_CMD ));
			if (msg_header->length != sizeof(SW_MC_STATE))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			logger(LOG_DEBUG, "proc save and del grp %x, bitmap %x\n", mcg->grpip, mcg->bitmap );
			ret = SaveAndDelMC(msg_header->card_no, mcg, msg_header->msg_code );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
			
		case C_SW_GETPORTSTATE:
			swport = ( SW_PORT_STATE *)(response_buff + sizeof(MSG_HEAD_CMD_RSP ));
			ret = GetSWportstate(msg_header->card_no, swport );
			if ( ret == 0 )
			{
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(SW_PORT_STATE);
			}
			else
			{
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;

		case C_SW_GETPARAM:
			swconfig = ( SW_CONFIG *)(response_buff + sizeof( MSG_HEAD_CMD_RSP ));
			ret = GetConfigParam(msg_header->card_no, swconfig );
			if ( ret == 0 )
			{
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(SW_CONFIG);
			}
			else
			{
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_SW_SETPARAM:
			swconfig = ( SW_CONFIG *)(buffer + sizeof( MSG_HEAD_CMD ));
			if (msg_header->length != sizeof(SW_CONFIG))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			logger(LOG_DEBUG, "switch set igmp query time value %u , setflag %u\n", swconfig->qry_time , swconfig->setFlag );
			ret = SetConfigParam(msg_header->card_no, swconfig );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_SW_GETCOUNT:
			swcount = (SW_PORT_COUNT *)(response_buff + sizeof(MSG_HEAD_CMD_RSP) );
			ret = GetSWCounters(msg_header->card_no, swcount );
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = sizeof(SW_PORT_COUNT);
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;

		case C_SW_RESETCOUNT:
			ret = ReseSWCounters(msg_header->card_no);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
			} else {
				msgrsp_header->rsp_code = ret;
			}
			msgrsp_header->length = 0;
			break;
		case C_ENCODE_RESTART:
		case C_CARD_RESET:
		case C_ONU_RESET:
		case C_FSA_LOWSPD_STATRESET:
			ret = genericReq( msg_header->card_no, msgrsp_header->msg_code, msg_header->node_id);
			msgrsp_header->rsp_code = ret;
			msgrsp_header->length = 0;
			break;
		case C_FSA_NET_GET:
		case C_FSA_LOWSPD_GET:
		case C_FSA_LOWSPD_STAT:
		case C_EP_GET_OLTSTAT:
		case C_EP_GET_OLTVLAN:
			ret = genericGetReq( msg_header->card_no, msgrsp_header->msg_code, msg_header->node_id, &(msgrsp_header->length),  response_buff + sizeof(MSG_HEAD_CMD_RSP));
			msgrsp_header->rsp_code = ret;
			break;
		case C_FSA_NET_SET:
		case C_FSA_LOWSPD_SET:
		case C_EP_SET_ONUNAME:
		case C_EP_SET_ONUBW:
		case C_EP_SET_ONUUART:
		case C_EP_SET_ONUIO:
		case C_EP_SET_ONUNETWORK:
		case C_EP_SET_ONU_UPPATH:
		case C_EP_SET_OLT_UPPATH:			
		case C_EP_DELONU_OFFLINE:			
		case C_EP_SET_OLTVLAN:
		case C_EP_RESET_ONU:			
		case C_HDC_SETCMD:
#if 1   //20150417
        case C_EP_SET_ONUIOCTRL:
#endif
			ret = genericSetReq( msg_header->card_no, msgrsp_header->msg_code, msg_header->node_id, msg_header->length, buffer + sizeof( MSG_HEAD_CMD ));
			msgrsp_header->rsp_code = ret;
			msgrsp_header->length = 0;
			break;
		case C_EP_GET_ONULIST:
		case C_EP_GET_ONUUART:
		case C_EP_GET_ONUSTAT:
		case C_EP_GET_ONUBW:
		case C_EP_GET_ONUIO:
		case C_EP_GET_ONUNETWORK:
		case C_HDC_GETCMD:
			ret = genericGetReq2( msg_header->card_no, msgrsp_header->msg_code, msg_header->node_id,  msg_header->length, buffer + sizeof( MSG_HEAD_CMD ), &(msgrsp_header->length),  response_buff + sizeof(MSG_HEAD_CMD_RSP));
			msgrsp_header->rsp_code = ret;
			break;
		case C_NTT_INFO:
			nttsend = (FE_PORT_STATE*)(buffer+sizeof(MSG_HEAD_CMD));
			ntt_info=(FE_PORT_STATE*)(response_buff+sizeof(MSG_HEAD_CMD_RSP));
			ntt_info->portIndex = nttsend->portIndex;
			ret = GetNttInfo(msg_header->card_no,ntt_info);
			if(ret == 0){
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length   = sizeof(FE_PORT_STATE);
			} else{
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length   = 0;
			}
			break;
			
#ifndef __MTT__		// 201611
		case C_NTT_GETSTATE:		
			ntt_state=(NTT_STATE*)(response_buff+sizeof(MSG_HEAD_CMD_RSP));
			ret = GetNttState(msg_header->card_no,ntt_state);
			if(ret == 0){
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length   = sizeof(NTT_STATE);
			} else{
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length   = 0;
			}
			break;
#else  // 201611			
        case C_MTT_GETSTATE:
            // 201708 根据卡的类型来转换
#if 1 // 201708            
            if (cardData[msg_header->card_no].card_type == CARD_TYPE_MTT) // add
#endif            
            {
                mtt_state=(MTT_STATE*)(response_buff+sizeof(MSG_HEAD_CMD_RSP));
                ret = GetMttState(msg_header->card_no, mtt_state);
                if(ret == 0){
                    msgrsp_header->rsp_code = 0;
                    msgrsp_header->length   = sizeof(MTT_STATE);
                } else{
                    msgrsp_header->rsp_code = ret;
                    msgrsp_header->length   = 0;
                }
            }
#if 1 // 201708
            else if (cardData[msg_header->card_no].card_type == CARD_TYPE_MTT_411) // add
            {
                mtt_411_state=(MTT_411_STATE*)(response_buff+sizeof(MSG_HEAD_CMD_RSP));
                ret = GetMtt411State(msg_header->card_no, mtt_state);
                if(ret == 0){
                    msgrsp_header->rsp_code = 0;
                    msgrsp_header->length   = sizeof(MTT_411_STATE);
                } else{
                    msgrsp_header->rsp_code = ret;
                    msgrsp_header->length   = 0;
                }
            }
            else if (cardData[msg_header->card_no].card_type == CARD_TYPE_MTT_441) // add
            {
                mtt_441_state=(MTT_441_STATE*)(response_buff+sizeof(MSG_HEAD_CMD_RSP));
                ret = GetMtt441State(msg_header->card_no, mtt_state);
                if(ret == 0){
                    msgrsp_header->rsp_code = 0;
                    msgrsp_header->length   = sizeof(MTT_441_STATE);
                } else{
                    msgrsp_header->rsp_code = ret;
                    msgrsp_header->length   = 0;
                }
            }
#endif
        break;
#endif
#if 1 // 201708
        case C_MTT_SEND_DATA:
        {
            ret = SendRs232Data(msg_header, buffer, msgrsp_header);
            msgrsp_header->rsp_code = ret;
            msgrsp_header->length = 0;
        }    
        break;
        
        case C_MTT_GET_DATA:
            ret = GetRs232Data(msg_header, buffer, msgrsp_header);
            msgrsp_header->rsp_code = ret;
            if (ret != 0)   msgrsp_header->length   = 0;
        break;
        
        case C_MTT_SEND_IO_CTRL:
            ret = genericSetReq(msg_header->card_no, msg_header->msg_code, msg_header->card_nonode_id,
                msg_header->length, buffer + sizeof( MSG_HEAD_CMD ));
			msgrsp_header->rsp_code = ret;
			msgrsp_header->length = 0;
        break;
#endif
		case C_NTT_SET:	
			if(msg_header->length != sizeof(CARD_SET_485FE))
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			
			nttcard_set = (CARD_SET_485FE*) (buffer + sizeof(MSG_HEAD_CMD));
			
			ret = SetCardNTT(msg_header->card_no, nttcard_set);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = 0;				
						
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
		case C_MS4_SWITCH:		
			if(msg_header->length != 1)
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			ret = Ms4Switch(msg_header->card_no, *(buffer + sizeof(MSG_HEAD_CMD)));
			msgrsp_header->rsp_code = ret;
			msgrsp_header->length   = 0;
			break;
		case C_MS4_DEBUG:		
			if(msg_header->length < 1)
			{
				msgrsp_header->rsp_code = ERROR_WRONG_ARGS;
				msgrsp_header->length = 0;
				break;
			}
			ret = Ms4Debug(msg_header->card_no, buffer + sizeof(MSG_HEAD_CMD), msg_header->length, response_buff+sizeof(MSG_HEAD_CMD_RSP), &msg_len);
			if(ret == 0) {
				msgrsp_header->rsp_code = 0;
				msgrsp_header->length = msg_len;				
						
			} else {
				msgrsp_header->rsp_code = ret;
				msgrsp_header->length = 0;
			}
			break;
				
 		default:
			msgrsp_header->rsp_code = 1;
			msgrsp_header->length = 0;			
	}

	snd_size = msgrsp_header->length + sizeof(MSG_HEAD_CMD_RSP);
	if(write(fds, response_buff, snd_size) != snd_size) {
			logger ( LOG_ERR ,"Message response write error!\n");
			return -1;
	}

	if (1 == updateFlag)
	{

		ret = updateDBFE8Info(msg_header->card_no, etherport_index->portIndex);
		if (ret !=0)
		{
			logger ( LOG_ERR , "Debug update FE8config DB from snmp err %d", ret);
		}
	
	}
	
	if (2 == updateFlag)
	{
		logger (LOG_DEBUG, "Update ONU list in db. flag=0x%x\n ", onuSet->setFlag);
		ret = updateONUList(msg_header->card_no, msg_header->node_id , onuSet); 
				
		if (ret != 0)
		{
			logger  (LOG_ERR , "Debug update onulist  err .retcode:%d\n ", ret);
		}
		updateFlag = 0;

	}
	return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "../common/const.h"
#include "../common/msg.h"
#include "../common/alarm_type.h"
#include "../common/serial_comm.h"
#include "../common/logger.h"
#include "alarm_proc.h"
#include "message_proc.h"
#include "card_mgmt.h"
#include "refresh_check.h"

extern CARD_INFO		cardData[];
extern int				 	fd485;
extern FSA600_STATE_485	fsa600_state;
#if 1 // 201611	暂时两个不能共存，待板卡修改后可以共存，mtt状态采用mtt_state
	extern NTT_STATE     ntt_state;
    extern MTT_STATE     mtt_state;
#endif

extern char fxlink_state[6];
extern char sfp_state[6];
//extern char link_state[2];  // 201611
extern char ntu_state[8];
extern GET_ALARMS_RSP fsa600_alarm[1+MAX_CARD_NUM_UTN];
extern ONU_SET onu100_Info[1+MAX_CARD_NUM_UTN][ 1 + MAX_ONU100_NUM];//onu100 name.
extern int alarmlog;

int UpdateOnuListDB(sqlite3 *db, unsigned char card_no, ONU_LIST_ENTRY* onulist, unsigned char onunum)
{
	char *zErrMsg = 0;
	int rc,fldIdx, i, nrow, ncol;
	char sql_str[256];
	char **result;
	unsigned char onus_in_db[MAX_ONU_NUM+1];
	unsigned char node_id;
	char name_tmp[MAX_DISPLAYNAME_LEN +1];
	int ring_flag;

	memset(onus_in_db, 0, sizeof(onus_in_db));

	sprintf(sql_str, "select NodeId from ONUList where CardNo=%d", card_no);
	rc = sqlite3_get_table(db,
					sql_str,                /* SQL to be executed */
					&result,			/* Result written to a char *[]  that this points to */
					&nrow,			/* Number of result rows written here */
					&ncol,			/* Number of result columns written here */
					&zErrMsg		/* Error msg written here */);
	if (rc !=SQLITE_OK)
	{
		logger (LOG_ERR,  "DB select nodeid Error:%s\n", zErrMsg);
		return ERROR ;					
	}

	for(i=1; i<=nrow; i++)
	{
		fldIdx = i*ncol;
		node_id = atoi(result[fldIdx]);
		if(node_id > MAX_ONU_NUM)
			continue;

		logger  (LOG_DEBUG,  "Node current:%d\n", node_id);

		onus_in_db[node_id] = 1;
	}
	
	sqlite3_free_table (result);

	ring_flag = 0;
	for(i=0; i<onunum; i++)
	{
		if(onulist[i].node_id > MAX_ONU_NUM) {
			logger (LOG_WARNING,  "Warning: get wrong node_id:%d from fsa card:%d C_ONU_GETLIST response!\n", onulist[i].node_id, card_no);
			continue;
		}

		//make sure the onu name is terminate with '\0' 
		memcpy(name_tmp, onulist[i].onu_name, MAX_DISPLAYNAME_LEN);
		name_tmp[MAX_DISPLAYNAME_LEN] = '\0';

		//update or insert the onu record in database 
		if(onus_in_db[onulist[i].node_id])
			sprintf(sql_str, "update ONUList  set NodeSN=%u ,NodeState=%u, NetworkState=%u, ONUName='%s' where CardNo=%u and NodeId=%u;",
					 onulist[i].node_sn, onulist[i].node_state, onulist[i].network_state, name_tmp, card_no, onulist[i].node_id);
		else
			sprintf(sql_str, "insert into ONUList(NodeSN, NodeState, NetworkState, ONUName, CardNo, NodeId) values(%d, %d, %d, '%s', %d, %d);",
					 onulist[i].node_sn, onulist[i].node_state, onulist[i].network_state, name_tmp, card_no, onulist[i].node_id);

		rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
		if(rc != SQLITE_OK) {
			logger (LOG_ERR,  "DB Error: Update NUList table error! sql=%s, err=%s\n", sql_str, sqlite3_errmsg(db) );
		}
		
		onus_in_db[onulist[i].node_id] = 2;

		if(onulist[i].network_state == ONU_NET_RING)
			ring_flag = 1;

	}

	/* delete or mark offline  the not matched onu node in database */
	for(i = 1; i<=MAX_ONU_NUM; i++)
	{
		if(onus_in_db[i] == 1)
		{
			if(ring_flag)
				sprintf(sql_str, "delete from ONUList where CardNo=%u and NodeId=%u;",  card_no, i);
			else
				sprintf(sql_str, "update ONUList set NodeState=3 where CardNo=%u and NodeId=%u;",  card_no, i);
			
			rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
			if(rc != SQLITE_OK) {
				logger (LOG_ERR,  "DB Error: delete ONUList table error! sql=%s, err=%s\n", sql_str, sqlite3_errmsg(db) );
			}
			logger (LOG_DEBUG,  "sql = %s", sql_str);
		}
	}	
	
	return 0;
}


int UpdateOnuListDB_sn(sqlite3 *db, unsigned char card_no, unsigned char node_id, unsigned char sn)
{
	char *zErrMsg = 0;
	int rc;
	char sql_str[256];

	sprintf(sql_str, "update ONUList  set NodeSN=%u where CardNo=%u and NodeId=%u;", sn, card_no, node_id);

	rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
	if(rc != SQLITE_OK) {
		logger (LOG_ERR,  "DB Error: Update ONUList table error! sql=%s, err=%s\n", sql_str, sqlite3_errmsg(db) );
		return -1;
	}
		
	return 0;
}


int  updateONU100Alarm ( sqlite3 *db  ,unsigned char  cardno , FSA600_STATE_485 *state)
{

	char sql_str [256];
	char *zErrMsg;
	int rc, i,j;

	char mask = 0x01;

	/*store the sfpstate and fxlinkstate in the arrrays.*/
	for (j = 0; j < 6; j++ )
	{

		if ( 0 !=  (state->Fxlink & mask) )
		{
			fxlink_state[j] = 1;
		}
		else{
			fxlink_state[j] = 0;
		}

		if ( 0 !=   (state->SfpState  & mask) )
		{
			sfp_state[j] = 1;
		}
		else{
			sfp_state[j] = 0;
		}
		
		logger  (LOG_DEBUG , "DEBUG:mask is %d fxlink is %d sfp state is %d\n", mask, fxlink_state[j], sfp_state[j] );
		mask = mask << 1;
	}
	

	/*first ,delete the records associated with the card number.then ,insert new records. */
	sprintf ( sql_str, "delete from ONUList where CardNo=%d", cardno );
	rc = sqlite3_exec ( db , sql_str, 0, 0, &zErrMsg);
	if ( rc != SQLITE_OK )
	{
		logger  (LOG_ERR, "database drop err :%s\n" , zErrMsg );
		return ERROR;
	}

	for ( i = 0; i < 6; i++ )
	{
		if ( 0 != fxlink_state[i]) /*fx linked*/
		{
			sprintf (sql_str,"insert into ONUList (CardNo, NodeId,NodeSN, NodeState,NetworkState)  values (%d,%d, %d, %d,%d)" , cardno, i+1, i+1,  fxlink_state[i] ,  fxlink_state[i]);
			rc = sqlite3_exec (db , sql_str, 0, 0, &zErrMsg);
			if ( rc != SQLITE_OK )
			{
				logger  (LOG_ERR, "insert onu100 into onulist err:%s sql is %s!\n", zErrMsg , sql_str );
				return ERROR;
				
			}
		}
	}

	return SUCCESS;
 }




int UpdateOnuList(sqlite3 *db, unsigned char card_no)
{
	
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	ONU_LIST_ENTRY	*onu_list;
	unsigned char 	*onu_num;
	int 				calc_len;

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_ONU_GETLIST;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	headReq->length = 0;
		
	headReq->dst = card_no;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR,  "Error: Get Onu List error response !,  error:%d.\n",  headRsp->rsp);
			return(-1);
		}	

		onu_num = (unsigned char *)(recvbuff + sizeof(MSG_HEAD_485));
		
		if(*onu_num > MAX_ONU_NUM) 
		{
			logger (LOG_ERR,  "Error: Get Onu List error: onu number:%u exceeds limit.\n",  *onu_num);
			return(-1);		
		}

		calc_len = 1 + (*onu_num) * sizeof(ONU_LIST_ENTRY);
		if(headRsp->length != calc_len)
		{
			logger (LOG_ERR,  "Error: Get OnuList response length error!,  length:%d, expect:%d.\n",  headRsp->length, calc_len);
			return(ERROR_COMM_PARSE);
		}

		onu_list = (ONU_LIST_ENTRY *)(recvbuff + sizeof(MSG_HEAD_485) +1);

		return(UpdateOnuListDB(db, card_no, onu_list, *onu_num));
	}
	return(-1);
}


int UpdateEPOnuList(sqlite3 *db, unsigned char card_no)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	EPONU_LIST_REQ	*req;
	EPONU_LIST_RSP	*rsp;
	int 				pon, more, calc_len;
	
	char *zErrMsg = 0;
	int rc, i;
	char sql_str[512];
	char mac_str[20];

	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_EP_GET_ONULIST;
	headReq->src = CTRL_CARDNO;
	headReq->onu_nodeid = 0;
	headReq->length = sizeof(EPONU_LIST_REQ);
	headReq->dst = card_no;

	req = (EPONU_LIST_REQ * ) (sendbuff + sizeof(MSG_HEAD_485));
	rsp = (EPONU_LIST_RSP* ) (recvbuff + sizeof(MSG_HEAD_485));
	
	req->type = 0;

	for(pon=0; pon<2; pon++)
	{
		req->group = 0;
		req->pon = pon;

		// mark the records in the database
		sprintf(sql_str, "update EPONUList  set DelFlag=1 where CardNo=%u and Pon=%u;", card_no, pon);
		rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
		if(rc != SQLITE_OK) {
			logger (LOG_ERR,  "DB Error: Mark EPONUList table error! sql=%s, err=%s\n", sql_str, sqlite3_errmsg(db) );
		}
		
		more = 1;
		while(more)
		{
			if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
			{	
				/* Got the response*/
				if(headRsp->rsp != 0)
				{
					logger (LOG_ERR,  "Error: Get Onu List error response !,  error:%d.\n",  headRsp->rsp);
					break;
				}					
				if(rsp->onu_units > MAX_EPONU_NUM) 
				{
					logger (LOG_ERR,  "Error: Get Onu List error: onu number:%u exceeds limit.\n",  rsp->onu_units);
					break;		
				}

				calc_len = 2 + rsp->onu_units * sizeof(EPONU_LIST_ENTRY);
				if(headRsp->length != calc_len)
				{
					logger (LOG_ERR,  "Error: Get OnuList response length error!,  length:%d, expect:%d.\n",  headRsp->length, calc_len);
					return(ERROR_COMM_PARSE);
				}

				//update in the database;
				for(i=0; i<rsp->onu_units; i++)
				{
					sprintf(mac_str, "%02X:%02X:%02X:%02X:%02X:%02X", rsp->onulist[i].mac[0],  rsp->onulist[i].mac[1], 
						 rsp->onulist[i].mac[2],  rsp->onulist[i].mac[3],  rsp->onulist[i].mac[4],  rsp->onulist[i].mac[5]);
					rsp->onulist[i].onu_name[31] = 0;
					
					sprintf(sql_str, "delete from EPONUList where CardNo=%u and Pon=%u and NodeId=%u; \
						insert into EPONUList(CardNo, Pon, NodeId, Distance, NodeState, MAC, ONUName) values(%d, %d, %d, %d, %d, '%s', '%s');",
								card_no, pon, rsp->onulist[i].node_id,
								card_no, pon, rsp->onulist[i].node_id, rsp->onulist[i].distance,  rsp->onulist[i].state, mac_str, rsp->onulist[i].onu_name);

					rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
					if(rc != SQLITE_OK) {
						logger (LOG_ERR,  "DB Error: Update EPONUList table error! sql=%s, err=%s\n", sql_str, sqlite3_errmsg(db) );
					}
				}
				more = rsp->more ;
				req->group++;
			}
		}

		//delete not exist nodes;
		sprintf(sql_str, "delete from EPONUList where CardNo=%u and Pon=%u and DelFlag=1;",  card_no, pon);
		rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
		if(rc != SQLITE_OK) {
			logger (LOG_ERR,  "DB Error: delete EPONUList table error! sql=%s, err=%s\n", sql_str, sqlite3_errmsg(db) );
		}

	}
	return(0);
}


void genAlarm(sqlite3 *db, unsigned char alarmType, unsigned char serverity,unsigned char card_no, unsigned char port_no, unsigned char node_id, unsigned char  reason )
{
	char sql_str[256];
	char *zErrMsg = 0;
	int rc;

	sprintf(sql_str, "insert into Alarm(AlarmType, Severity, CardNo, Port, NodeId, Reason) values(%u, %d, %d, %d, %d, %d);", alarmType, serverity, card_no, port_no, node_id, reason);
	rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
	if(rc != SQLITE_OK) {
		logger (LOG_ERR , "DB insert alarm error: %s\nsql:%s\n", sqlite3_errmsg(db),sql_str);
	}
}

void genCardAlarm(sqlite3 *db, unsigned char alarmType, unsigned char serverity, unsigned char card_no, unsigned char port_no)
{
	genAlarm(db, alarmType, serverity, card_no, port_no, 0, 0);
}

unsigned char getServerity(unsigned char alarmType)
{
	unsigned char serverity;

	switch(alarmType) {
		case ATYPE_FANOFF:
		case ATYPE_ONUSFPREMOVED:
		case ATYPE_CARDRESTART:
		case ATYPE_SFPREMOVED:
		case ATYPE_ONETRINGSTANDBYFAILURE:
		case ATYPE_ONETLINKBROKEN:
		case ATYPE_ONETRINGFAILURE:
		case ATYPE_ONETRINGBROKEN:
		case ATYPE_RMTPOWERDOWN:	
		case ATYPE_PORTDOWAN:
		case ATYPE_ONUPORTDOWN:
		case ATYPE_FXDOWN:
		case ATYPE_ONUSFPERROR:
			serverity = ALARM_SERVERITY_WARNING;
			break;
		case ATYPE_PSAOFF:
		case ATYPE_PSBOFF:
		case ATYPE_CARDOFFLINE:
		case ATYPE_CARDFAILURE:
		case ATYPE_ONUOFFLINE:
		case ATYPE_FANFAIL:
			serverity = ALARM_SERVERITY_ERROR;
			break;
		default :
			serverity = ALARM_SERVERITY_INFO;
			break;
	}

	return(serverity);
}

void check_cardAlarms()
{
	static unsigned int  		cardTOcount[1 + MAX_CARD_NUM_UTN];
	sqlite3 			*db; 
	int 				i, j;
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	GET_ALARMS_RSP	*alarm_rsp;
	unsigned short	calc_len;
	int 				needUpdateOnuList, needRefreshTree ,needUpdateNtuList, rc;
	char mask = 0x01;
	char x,y;
	
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = GET_ALARMS;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;

	db = NULL;
	
	for(i=1; i <= MAX_CARD_NUM_UTN; i++)
	{
		//skip the offline card and ctrl card itself
		if((cardData[i].card_no == 0) || (cardData[i].card_no == CTRL_CARDNO))
			continue;
		
		headReq->dst = cardData[i].card_no;
		if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff, TIMEOUT_485MSG))
		{	
			cardTOcount[i] = 0;

			/* Got the alarm response*/
			if(headRsp->rsp != 0)
			{
				logger (LOG_ERR , "Error: Get alarm error response !,  error:%d.\n",  headRsp->rsp);
				continue;
			}				
			alarm_rsp = (GET_ALARMS_RSP *) (recvbuff + sizeof(MSG_HEAD_485));
			//if(headRsp->length == 0 || alarm_rsp->alarmNum == 0)
            if(headRsp->length == 0 || ((alarm_rsp->alarmNum == 0) && (alarm_rsp->rsp_typ!= 4)))
				continue;

			/* check the message length */
			switch(alarm_rsp->rsp_type)
			{
				case 0:
					// general alarm
					calc_len = 2 + alarm_rsp->alarmNum * sizeof(ALARM_INFO);
					break;
				case 1:
					//fsa12500
					calc_len = 2 + alarm_rsp->alarmNum * sizeof(ONET_ALARM_INFO);
					break;
				case 2: 
					//fsa600 
					calc_len = 2 + alarm_rsp->alarmNum*sizeof(FSA600_ALARM_INFO);
					break;
				case 3:
					//fe8
					calc_len = 2 + alarm_rsp->alarmNum*sizeof(FE8_ALARM_INFO);
					if(alarmlog)
						logger (LOG_DEBUG, "FE8 alarm length calc_len is -%d\n", calc_len);
					break;
				case 4:
					//NTT
#ifndef __MTT__  	// 201611		
					calc_len = 2 + alarm_rsp->alarmNum * sizeof(NTT_ALARM_INFO);
#else
                    calc_len = 2 + alarm_rsp->alarmNum * sizeof(MTT_ALARM_INFO);
                    calc_len = 6;
#endif
					break;
				case 5:
					//Epon
					calc_len = 2 + alarm_rsp->alarmNum * sizeof(EPON_ALARM_INFO);
					break;
#ifdef __MTT__  	// 201611		
				case 6: // MTT
				    if (cardData[i].card_type == CARD_TYPE_MTT)
				    {
                        calc_len = 2 + alarm_rsp->alarmNum * sizeof(MTT_ALARM_INFO);
                        calc_len = 6;
                    }
#if 1 // 201708                    
                    else if (cardData[i].card_type == CARD_TYPE_MTT_411) //  201708
                    {
                        calc_len = 2 + alarm_rsp->alarmNum * sizeof(MTT411_ALARM_INFO);
                    }
                    else if (cardData[i].card_type == CARD_TYPE_MTT_441) //  201708
                    {
                        calc_len = 2 + alarm_rsp->alarmNum * sizeof(MTT441_ALARM_INFO);
                    }
#endif                    
    				break;
#endif
			}
			
			if(headRsp->length != calc_len)
			{
				logger (LOG_ERR,  "Error: Card Alarm response length error!,  length:%d, expect:%d.\n",  headRsp->length, calc_len);
				continue;
			}

			/* start to process the alarm*/ 
			if(db == NULL)
			{
				if( sqlite3_open(DYNAMIC_DB, &db) ){
					logger (LOG_ERR,   "Can't open database: %s\n", sqlite3_errmsg(db));
					return;
				}
			}
			
			if(alarm_rsp->rsp_type == 0)
			{
				for(j=0; j< alarm_rsp->alarmNum; j++)
				{
					genCardAlarm(db, 
        					alarm_rsp->data.alarms[j].alarm_type, 
        					getServerity(alarm_rsp->data.alarms[j].alarm_type),
							i, 
							alarm_rsp->data.alarms[j].port_no);
				}
			}

			if ( 1 == alarm_rsp->rsp_type ) {	//fsa12500	
				needUpdateOnuList = 0;
				needRefreshTree = 0;
				for(j=0; j< alarm_rsp->alarmNum; j++)
				{
					switch(alarm_rsp->data.fsa_alarms[j].alarm_type) {
						case ATYPE_ONUONLINE:
						case ATYPE_ONUOFFLINE:
							needRefreshTree = 1;
						case ATYPE_ONETRINGFAILURE:
						case ATYPE_ONETRINGRESTORE:
						case ATYPE_ONETRINGBROKEN:
						case ATYPE_ONETLINKBROKEN:
						case ATYPE_ONETLINKRESTORE:
						case ATYPE_ONUNAMECHG:
							needUpdateOnuList = 1;
							break;
						default :
							break;
					}

					if(alarm_rsp->data.fsa_alarms[j].alarm_type == ATYPE_ONUSNCHG)
					{
						UpdateOnuListDB_sn(db, i, alarm_rsp->data.fsa_alarms[j].node_id, alarm_rsp->data.fsa_alarms[j].reason);
						setRefreshState_Topo(i);
					}
					else if(alarm_rsp->data.fsa_alarms[j].alarm_type != ATYPE_ONUNAMECHG)
						genAlarm(db, 
    							alarm_rsp->data.fsa_alarms[j].alarm_type, 
    							getServerity(alarm_rsp->data.fsa_alarms[j].alarm_type),
    							i, 
    							alarm_rsp->data.fsa_alarms[j].port_no,
    							alarm_rsp->data.fsa_alarms[j].node_id,
    							alarm_rsp->data.fsa_alarms[j].reason);

				}

				if(needUpdateOnuList)
				{
					UpdateOnuList(db, i);
					setRefreshState_Topo(i);
					if(needRefreshTree)
						setRefreshState(CHANGE_FLAG_TREEV);
				}
			}

			if ( 2 == alarm_rsp->rsp_type )//fsa600
			{
				if ( 1 == alarm_rsp->alarmNum)
				{
					if(alarmlog)
					{
						logger(LOG_DEBUG, "last time fsa600 alarm info is %d - %d - %d - %d  - %d\n ", fsa600_alarm[i].data.fsa600_alarms.sfpstate, fsa600_alarm[i].data.fsa600_alarms.fxstate,fsa600_alarm[i].data.fsa600_alarms.rmt_tx1state,fsa600_alarm[i].data.fsa600_alarms.rmt_tx2state, fsa600_alarm[i].data.fsa600_alarms.rmtPwrDown);
						logger(LOG_DEBUG, "fsa600 alarm info is %d - %d - %d - %d  - %d\n ", alarm_rsp->data.fsa600_alarms.sfpstate, alarm_rsp->data.fsa600_alarms.fxstate, alarm_rsp->data.fsa600_alarms.rmt_tx1state, alarm_rsp->data.fsa600_alarms.rmt_tx2state, alarm_rsp->data.fsa600_alarms.rmtPwrDown);
					}
					needUpdateOnuList = 0 ;//init it.
					for ( j = 1; j <= 6; j++ )
					{
						/*insert the alarm records  into database.*/
						if ( ( fsa600_alarm[i].data.fsa600_alarms.sfpstate  & mask ) !=  (alarm_rsp->data.fsa600_alarms.sfpstate & mask) )
						{
							if(alarmlog)
								logger (LOG_DEBUG, "sfpstate fsa600_alarm and mask is %d - alarm_rsp and mask is %d\n", fsa600_alarm[i].data.fsa600_alarms.sfpstate  & mask, alarm_rsp->data.fsa600_alarms.sfpstate & mask);
							if ( mask ==  (alarm_rsp->data.fsa600_alarms.sfpstate & mask) )
								genAlarm(db, ATYPE_SFPINSERTED , getServerity(ATYPE_SFPINSERTED), i, j, 0, 0);//reason??
							else 
								genAlarm(db, ATYPE_SFPREMOVED , getServerity(ATYPE_SFPREMOVED), i, j, 0, 0);//reason??
							
						}	

						if ( ( fsa600_alarm[i].data.fsa600_alarms.rmt_tx1state & mask ) !=  (alarm_rsp->data.fsa600_alarms.rmt_tx1state & mask) )
						{
							if(alarmlog)
								logger (LOG_DEBUG, "rmt_tx1state fsa600_alarm and mask is %d - alarm_rsp and mask is %d\n", fsa600_alarm[i].data.fsa600_alarms.rmt_tx1state  & mask, alarm_rsp->data.fsa600_alarms.rmt_tx1state & mask);
							if ( mask ==  ( alarm_rsp->data.fsa600_alarms.rmt_tx1state  & mask) )
								genAlarm(db, ATYPE_ONUPORTUP , getServerity(ATYPE_ONUPORTUP), i, 1, j, 0);//reason??
							else 
								genAlarm(db, ATYPE_ONUPORTDOWN , getServerity(ATYPE_ONUPORTDOWN), i, 1, j, 0);//reason??
							
						}
						
						if ( ( fsa600_alarm[i].data.fsa600_alarms.rmt_tx2state & mask ) !=  (alarm_rsp->data.fsa600_alarms.rmt_tx2state & mask) )
						{
							if(alarmlog)
								logger (LOG_DEBUG, "rmt_tx2state fsa600_alarm and mask is %d - alarm_rsp and mask is %d\n", fsa600_alarm[i].data.fsa600_alarms.rmt_tx2state  & mask, alarm_rsp->data.fsa600_alarms.rmt_tx2state & mask);
							if ( mask ==  ( alarm_rsp->data.fsa600_alarms.rmt_tx2state   & mask) )
								genAlarm(db, ATYPE_ONUPORTUP , getServerity(ATYPE_ONUPORTUP), i, 2, j, 0);//reason??
							else 
								genAlarm(db, ATYPE_ONUPORTDOWN , getServerity(ATYPE_ONUPORTDOWN), i, 2, j, 0);//reason??
							
						}
						
						
						if ( ( fsa600_alarm[i].data.fsa600_alarms.fxstate & mask ) !=  (alarm_rsp->data.fsa600_alarms.fxstate & mask) )
						{
							needUpdateOnuList = 1;
							if(alarmlog)
								logger (LOG_DEBUG, "fxstate fsa600_alarm and mask is %d - alarm_rsp and mask is %d\n", fsa600_alarm[i].data.fsa600_alarms.fxstate  & mask, alarm_rsp->data.fsa600_alarms.fxstate & mask);
							if ( mask ==  ( alarm_rsp->data.fsa600_alarms.fxstate  & mask) )
								genAlarm(db, ATYPE_FXUP , getServerity(ATYPE_FXUP), i, j, 0, 0);//reason??
							else 
								genAlarm(db, ATYPE_FXDOWN , getServerity(ATYPE_FXDOWN), i, j, 0, 0);//reason??
							
						}
						
						
						if ( ( fsa600_alarm[i].data.fsa600_alarms.rmtPwrDown & mask ) !=  (alarm_rsp->data.fsa600_alarms.rmtPwrDown & mask) )
						{
							needUpdateOnuList = 1;
							if(alarmlog)
								logger (LOG_DEBUG, "rmtPwrDown fsa600_alarm and mask is %d - alarm_rsp and mask is %d\n", fsa600_alarm[i].data.fsa600_alarms.rmtPwrDown  & mask, alarm_rsp->data.fsa600_alarms.rmtPwrDown & mask);
							if (  mask ==  ( alarm_rsp->data.fsa600_alarms.rmtPwrDown  & mask) )
								genAlarm(db, ATYPE_RMTPOWERDOWN , getServerity(ATYPE_RMTPOWERDOWN), i, 0, j, 0);//reason??
							
							
						}
						
						
						mask = mask << 1;

							
					}

				
					/*if fx state changed or  remote modules power down,  need to re-acquire the number of remote modules, and then update the database*/
					if ( 0 != needUpdateOnuList )
					{
						rc = getONU100 ( i );
						if (  rc != 0 )
						{
							logger  (LOG_ERR , "get onu100 err %dl!\n", rc );
						}
						/*Database can not be nested*/
						rc = updateONU100Alarm ( db  , i , &fsa600_state);
						if (  rc != 0 )
						{
							logger  (LOG_ERR , "update onu100 for alarm  err %dl!\n", rc );
						}

						/*rc = configFSA600RMT ( db, i );
						if ( rc != 0 )
						{
							logger (LOG_ERR, "configure onu100 err %d !\n", rc );
						}*/
						/*different db config db.*/
						rc = configFSA600AndRMT ( i );
						if ( rc != 0 )
						{
							logger (LOG_ERR, "configure fsa600 and onu100 err %d !\n", rc );
						}
						
						setRefreshState(CHANGE_FLAG_TREEV);

					}
						
					needUpdateOnuList = 0;//clear it.
					mask = 0x01;
					memcpy(&fsa600_alarm[i], alarm_rsp,  alarm_rsp->alarmNum*sizeof(GET_ALARMS_RSP));//update it.
					if(alarmlog)
						logger(LOG_DEBUG, "after copy fsa600 alarm info is %d - %d - %d - %d  - %d\n ", fsa600_alarm[i].data.fsa600_alarms.sfpstate, fsa600_alarm[i].data.fsa600_alarms.fxstate, fsa600_alarm[i].data.fsa600_alarms.rmt_tx1state, fsa600_alarm[i].data.fsa600_alarms.rmt_tx2state, fsa600_alarm[i].data.fsa600_alarms.rmtPwrDown);
				}	

				
			}

			if ( 3 == alarm_rsp->rsp_type )
			{
				if ( 1 == alarm_rsp->alarmNum)
				{
					mask = 0x01;
					for ( j = 1; j <= 8; j++ )
					{
						if ( mask == (mask&alarm_rsp->data.fe8_alarms.port_up) )
						{
							if(alarmlog)
								logger (LOG_DEBUG, "FE8 alarm debug portup mask is %d -mask and port_up is %d\n", mask, mask&alarm_rsp->data.fe8_alarms.port_up);
							genAlarm(db, ATYPE_PORTUP, getServerity(ATYPE_PORTUP), i, j, 0, 0);//reason??
						}

						if ( mask == (mask&alarm_rsp->data.fe8_alarms.port_down))
						{
							if(alarmlog)
								logger (LOG_DEBUG, "FE8 alarm debug port down mask is %d -mask and port_down is %d\n", mask, mask&alarm_rsp->data.fe8_alarms.port_down);
							genAlarm(db, ATYPE_PORTDOWAN , getServerity(ATYPE_PORTDOWAN), i, j, 0, 0);//reason??
						}

						mask = mask << 1;
					}

					mask = 0x01;//clear it.
				}

			}
#ifndef __MTT__			
			if ( 4 == alarm_rsp->rsp_type ) //ntt or mtt
			{
				if ( 1 == alarm_rsp->alarmNum)
				{
					mask = 0x01;
					needUpdateNtuList = 0;
					for ( j = 1; j <= 8; j++ )
					{
						x=j;
						y=1;			
						if(j>4){
								x=j-4;
								y=2;
						}
						
						if(alarm_rsp->data.ntt_alarms.port_up & mask)
						{
							if(alarmlog)
								logger (LOG_DEBUG, "NTT alarm debug portup mask is %d -mask and port_up is %d\n", mask, mask&alarm_rsp->data.ntt_alarms.port_up);
							genAlarm(db, ATYPE_PORTUP, getServerity(ATYPE_PORTUP), i, x, y, 0);
						
						}
						if(alarm_rsp->data.ntt_alarms.port_down & mask)
						{
						
							if(alarmlog)
								logger (LOG_DEBUG, "NTT alarm debug port down mask is %d -mask and port_down is %d\n", mask, mask&alarm_rsp->data.ntt_alarms.port_down);
							genAlarm(db, ATYPE_PORTDOWAN , getServerity(ATYPE_PORTDOWAN), i, x, y, 0);
						}

						mask = mask << 1;
					}

					mask = 0x01;//clear it.	

					for(j = 1; j <=2 ; j++)
					{
						
						if(alarm_rsp->data.ntt_alarms.fiber_on & mask)
						{
							needUpdateNtuList = 1;
							if(alarmlog)
								logger (LOG_DEBUG, "NTT alarm debug fiber on mask is %d -mask and fiber_on is %d\n", mask, mask&alarm_rsp->data.ntt_alarms.fiber_on);
							if( mask == (mask&alarm_rsp->data.ntt_alarms.fiber_on))
								genAlarm(db, ATYPE_FXUP, getServerity(ATYPE_FXUP), i, j, 0, 0);
							
						}
						
						if(alarm_rsp->data.ntt_alarms.fiber_off & mask)
						{
							needUpdateNtuList = 1;
							if(alarmlog)
								logger (LOG_DEBUG, "NTT alarm debug fiber off mask is %d -mask and fiber_off is %d\n", mask, mask&alarm_rsp->data.ntt_alarms.fiber_off);
							if( mask == (mask&alarm_rsp->data.ntt_alarms.fiber_off))
								genAlarm(db, ATYPE_FXDOWN, getServerity(ATYPE_FXDOWN), i, j, 0, 0);
						}
						mask = mask << 1;
					}

					if( 0 != needUpdateNtuList )
					{
						rc = GetNttState(i,&ntt_state);
						if( rc != 0)
						{
							logger (LOG_ERR,"get ntt err %d!\n",rc);
						}
						updateNTTList(i,&ntt_state);
						setRefreshState(CHANGE_FLAG_TREEV);
					}

					needUpdateNtuList = 0;

				}
			}
#endif

			if ( 5 == alarm_rsp->rsp_type ) {	//EPON	
				needUpdateOnuList = 0;
				needRefreshTree = 0;
				for(j=0; j< alarm_rsp->alarmNum; j++)
				{
					switch(alarm_rsp->data.fsa_alarms[j].alarm_type) {
						case ATYPE_ONUONLINE:
						case ATYPE_ONUOFFLINE:
						case ATYPE_FXUP:
						case ATYPE_FXDOWN:
							needRefreshTree = 1;
							needUpdateOnuList = 1;
							break;
						default :
							break;
					}

					genAlarm(db, 
							alarm_rsp->data.epon_alarms[j].alarm_type, 
							getServerity(alarm_rsp->data.epon_alarms[j].alarm_type),
							i, 
							alarm_rsp->data.epon_alarms[j].port_no,
							alarm_rsp->data.epon_alarms[j].node_id,
							alarm_rsp->data.epon_alarms[j].pon);

				}

				if(needUpdateOnuList)
				{
					UpdateEPOnuList(db, i);
					if(needRefreshTree)
						setRefreshState(CHANGE_FLAG_TREEV);
				}
			}
#ifdef __MTT__  // 201611			
            if ( 4 == alarm_rsp->rsp_type ) 
#else
			if ( 6 == alarm_rsp->rsp_type ) //
#endif
			{
                // 需要重写告警解析
                //alarm_rsp->alarmNum = 0;
                MTT_ALARM_INFO* alm = (MTT_ALARM_INFO*)&alarm_rsp->data.mtt_alarms;
                int update_flag = 0;
                
                int mask = 0x01;
                for (i = 0; i < 8; i++)
                {
                    // 前端网口下线
                    mask =  mask << i;
                    if (alm->rmt_portdown & mask)
                    {
    				    genAlarm(db, ATYPE_PORTDOWAN, getServerity(ATYPE_PORTDOWAN), 
    				        cardData[i].card_no, 
    				        (i < 4 ? i + 1 : i - 3), 
    				        (i < 4 ? 1: 2), 0);
        				update_flag = 1;
    				}
                    if (alm->rmt_portup & mask)
                    {
    				    genAlarm(db, ATYPE_PORTUP, getServerity(ATYPE_PORTUP), 
    				        cardData[i].card_no, 
    				        (i < 4 ? i + 1 : i - 3), 
    				        (i < 4 ? 1: 2), 0);
        				update_flag = 1;
    				}
                    if ((alm->loc_portdown & mask))
                    {
        				update_flag = 1;
                        switch (mask)
                        {
                            case 1:
                            case 2:
                                genAlarm(db, ATYPE_PORTDOWAN, getServerity(ATYPE_PORTDOWAN), 
                                    cardData[i].card_no, mask,  0, 0);
                            break;
                            case 4:
                            case 8:
                                genAlarm(db, ATYPE_SFPREMOVED, getServerity(ATYPE_SFPREMOVED),
                                    cardData[i].card_no, mask>>2,  0, 0);
                            break;
                            case 0x10:
                            case 0x20:
                                genAlarm(db, ATYPE_RMTPOWERDOWN, getServerity(ATYPE_RMTPOWERDOWN),
                                    cardData[i].card_no, mask>>4,  0, 0);
                            break;
                            case 0x40:
                            case 0x80:
                                genAlarm(db, ATYPE_LOCETH_DOWN, getServerity(ATYPE_LOCETH_DOWN),
                                    cardData[i].card_no,  mask>>6,  0, 0);
                            break;
                        }
                    }
                    
                    if ((alm->loc_portup & mask))
                    {
        				update_flag = 1;
                        switch (mask)
                        {
                            case 1:
                            case 2:
                                genAlarm(db, ATYPE_PORTUP, getServerity(ATYPE_PORTUP),
                                    cardData[i].card_no, mask,  0, 0);
                            break;
                            case 4:
                            case 8:
                                genAlarm(db, ATYPE_SFPINSERTED, getServerity(ATYPE_SFPINSERTED),
                                    cardData[i].card_no, mask>>2,  0, 0);
                            break;
                            case 0x10:
                            case 0x20:
                                genAlarm(db, ATYPE_OVER_TEMP, getServerity(ATYPE_OVER_TEMP),
                                    cardData[i].card_no, mask>>4,  0, 0);
                            break;
                            case 0x40:
                            case 0x80:
                                genAlarm(db, ATYPE_LOCETH_UP, getServerity(ATYPE_LOCETH_UP),
                                    cardData[i].card_no,  mask>>6,  0, 0);
                            break;
                        }
                    }
                }
                           
				if( 0 != update_flag )
				{
                    rc = GetMttState(i, &mtt_state);
					if( rc != 0)
					{
						logger (LOG_ERR,"Get mtt err %d!\n", rc);
					}
					updateMTTList(i, &mtt_state);
					setRefreshState(CHANGE_FLAG_TREEV);
				}
			}

#endif

#if 1 // 201708 增加两种新的MTT卡
            if ( 6 == alarm_rsp->rsp_type )
            {
                if (cardData[i].card_type == CARD_TYPE_MTT_411)
                {
                    MTT411_ALARM_INFO* alm = (MTT411_ALARM_INFO*)&alarm_rsp->data.mtt_alarms;
                }
                else if (cardData[i].card_type == CARD_TYPE_MTT_441)
                {
                    MTT441_ALARM_INFO* alm = (MTT441_ALARM_INFO*)&alarm_rsp->data.mtt_alarms;
                }
            }

#endif
			
		} else {
			cardTOcount[i]++;
			if(cardTOcount[i] >= 2 ) {
				char sql_str[256];
				char *zErrMsg = 0;
				int rc;
				
				/* mark the card as offline */
				if(db == NULL)
				{
					if( sqlite3_open(DYNAMIC_DB, &db) ){
						logger (LOG_ERR,  "Can't open database: %s\n", sqlite3_errmsg(db));
						return;
					}
				}	
				
				sprintf(sql_str, "delete from Card where CardNo=%d;", cardData[i].card_no);
				rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
				if(rc != SQLITE_OK) {
					logger (LOG_ERR,  "Delete card error: %s\n", sqlite3_errmsg(db));
				}		

				genCardAlarm(db, ATYPE_CARDOFFLINE, getServerity(ATYPE_CARDOFFLINE), cardData[i].card_no, 0);
				setRefreshState(CHANGE_FLAG_TREEV|CHANGE_FLAG_FRAMEV);

				cardTOcount[i] = 0;
				memset(cardData + i, 0, sizeof(CARD_INFO));

			}

			
		}
	}

	if( db != NULL )
		sqlite3_close(db);	

}



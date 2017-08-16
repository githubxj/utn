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

CARD_INFO	cardData[1 + MAX_CARD_NUM_UTN];
ETHER_PORT_SET	FE8Card[1+MAX_CARD_NUM_UTN][9];
/*for fsa600 */
	char fxlink_state[6];
	char sfp_state [6];
	char link_state[2];     // 记录两个前端是否在线的状态 for ntu/mtu
	char ntu_state[8];
	
	NTT_STATE     ntt_state;
    MTT_STATE     mtt_state;    // 201611
    
#if 1 // 201708    
    MTT_411_STATE   mtt_411_state;
    MTT_441_STATE   mtt_441_state;
#endif

	FSA600_STATE_485 fsa600_state; //used for get the number of sfp inserted and FXlink state.
	GET_ALARMS_RSP fsa600_alarm[1+MAX_CARD_NUM_UTN];
	
extern	FSA600_GETRMT  RMT_Data[1+MAX_CARD_NUM_UTN][ 1 + MAX_ONU100_NUM];
	
extern int fd485;
extern int UpdateOnuList(sqlite3 *db, unsigned char card_no);

void cardmgmt_init()
{
	sqlite3 *db;  
	char *zErrMsg = 0;
	int rc;
	char sql_str[256];

	memset(cardData, 0, sizeof(cardData));
	
	/* clear the card in the dynamic database */
	rc = sqlite3_open(DYNAMIC_DB, &db);

	if( rc ){
		logger ( LOG_ERR,   "Can't open database: %s\n", sqlite3_errmsg(db));
		return;
	}

	rc = sqlite3_exec(db, 
				"delete from Card", 
				0, 
				0, 
				&zErrMsg);

	if(rc != SQLITE_OK) {
		logger ( LOG_ERR , "Database drop error: %s\n", sqlite3_errmsg(db));
	 }		
	
	sprintf(sql_str, "insert into Card(CardNo, CardType) values (%d, 1)", CTRL_CARDNO);
	rc = sqlite3_exec(db, 
				sql_str, 
				0, 
				0, 
				&zErrMsg);

	if(rc != SQLITE_OK) {
		logger (LOG_ERR,  "Database insert error: %s\n", sqlite3_errmsg(db));
	}
	sqlite3_close(db);
	cardData[CTRL_CARDNO].card_type =(unsigned char) CARD_TYPE_CTRL;
	
	//2010-03-18 for h264.
	/*sprintf(sql_str, "insert into Card(CardNo, CardType) values (%d, 5)", 4);
	rc = sqlite3_exec(db, 
				sql_str, 
				0, 
				0, 
				&zErrMsg);

	if(rc != SQLITE_OK) {
		logger (LOG_ERR,  "Database insert error: %s\n", sqlite3_errmsg(db));
	}
	//2010-03-18 for h264.
	

	//2010-03-18 for h264.
	cardData[4].card_type =(unsigned char) CARD_TYPE_ENCODE;*/
	
}


int getIGMPParam(SW_CONFIG *param)
{
	char file[64];
	struct stat conf_stat;

	snprintf(file, 64, "%s", SSW_PARAM_CONFIG);

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

	memset(param, 0, sizeof(SW_CONFIG));
	param->qry_time = iniGetInt(pIni, "IGMP", "QUERY_TIME");
	param->igmp_timeout= iniGetInt(pIni, "IGMP", "TIMEOUT");
	param->igmp_qportmap = iniGetInt(pIni, "IGMP", "QUERY_PORTMAP");
	param->igmp_rportmap = iniGetInt(pIni, "IGMP", "ROUTER_PORTMAP");
	iniFileFree(pIni);
	return 0;
}

int saveIGMPParam(const SW_CONFIG *param)
{
	char file[64];
	snprintf(file, 64, "%s", SSW_PARAM_CONFIG);

	IniFilePtr pIni = iniFileOpen(file);
	if(NULL == pIni)
		return -1;

  	if(param->setFlag&MD_SW_IGMPQRYTIME) 
		iniSetInt(pIni, "IGMP", "QUERY_TIME", param->qry_time);
  	if(param->setFlag&MD_SW_IGMP_TIMOUT) 
		iniSetInt(pIni, "IGMP", "TIMEOUT", param->igmp_timeout);
  	if(param->setFlag&MD_SW_IGMP_QPORTMAP) 
		iniSetInt(pIni, "IGMP", "QUERY_PORTMAP", param->igmp_qportmap);
  	if(param->setFlag&MD_SW_IGMP_RPORTMAP) 
		iniSetInt(pIni, "IGMP", "ROUTER_PORTMAP", param->igmp_rportmap);

	iniFileSave(pIni);
	iniFileFree(pIni);
	return 0;
}


int CTRL_Config_init()
{
	sqlite3 *db;
	char sql_str[256] , buff[64];
	char **result;
	char *zErrMsg;
	int rc, nrow, ncol,fldIndex ;
	SW_MC_STATE mcg;
	SW_VLAN vlan;
	SW_CONFIG igmp_param;
	int i;

	/* initialize IGMP parameters */
	if(getIGMPParam(&igmp_param) == 0)
	{
		igmp_param.setFlag = 0xFFFF;
		SetConfigParam(CTRL_CARDNO, &igmp_param);
	}
	else 
		logger (LOG_ERR, "Can not load initial IGMP parameters\n");

//	SetFEPort100MFull();

	memset(buff, 0, sizeof(buff ));
	rc = sqlite3_open (CONF_DB, &db );
	if ( rc)
	{
		logger ( LOG_ERR  , "can not open config database.%s", sqlite3_errmsg(db));
		return ERROR;
	}

	sprintf (sql_str, "select CardName from FE8Card where CardNo=%d ",CTRL_CARDNO);
	sqlite3_get_table (db, sql_str, &result, &nrow, &ncol,&zErrMsg);
	if (rc != SQLITE_OK)
	{
		logger ( LOG_ERR  , "select control card name err.sql=%s,zerrmsg=%s\n",sql_str, zErrMsg );
		return ERROR;
	}

	if (nrow >= 1)
	{
		fldIndex = ncol;
		if ( NULL != result[fldIndex] )
		{
			strncpy(cardData[CTRL_CARDNO].card_name , result[fldIndex], MAX_DISPLAYNAME_LEN);
		}
	}
	
	sqlite3_free_table (result);

	/*init something about multicast ......*/
	sprintf (sql_str, "select * from MULTICAST where flag=%d ",1);
	sqlite3_get_table (db, sql_str, &result, &nrow, &ncol,&zErrMsg);
	if (rc != SQLITE_OK)
	{
		logger ( LOG_ERR  , "select multicast err.sql=%s,zerrmsg=%s\n",sql_str, zErrMsg );
		return ERROR;
	}
	logger(LOG_DEBUG, "select multicast nrow %d, ncol %d\n ", nrow, ncol );
	if (nrow >= 1)
	{
		for ( i = 1 ; i <= nrow; i++  )
		{
			fldIndex = i*ncol;
			if ( NULL != result[fldIndex] )
			{
				strcpy(buff,  result[fldIndex]);
				sscanf(buff, "%10u", &mcg.grpip );//fix

			}
			if ( NULL != result[fldIndex + 1] )
			{
				memset(buff, 0, sizeof(buff));
				strcpy(buff,  result[fldIndex + 1]);
				sscanf(buff, "%10u", &mcg.bitmap );//fix

			}
			mcg.flag = 1;
			mcg.setFlag = 0xFF;
			if ( 0 != SaveAndDelMC( CTRL_CARDNO , &mcg, C_SW_SAVEMCG ))
			{
				logger(LOG_ERR, "init control card 's multicast err. ");
			}
			else
			{
				logger(LOG_DEBUG, "init control card 's multicast successful.");
			}
		}

	}
	sqlite3_free_table (result);

	/*init something about vlan  ......*/
	sprintf (sql_str, "select * from VLAN");
	sqlite3_get_table (db, sql_str, &result, &nrow, &ncol,&zErrMsg);
	if (rc != SQLITE_OK)
	{
		logger ( LOG_ERR  , "select vlan err.sql=%s,zerrmsg=%s\n",sql_str, zErrMsg );
		return ERROR;
	}
	logger(LOG_DEBUG, "select vlan nrow %d, ncol %d\n ", nrow, ncol );
	if (nrow >= 1)
	{
		for ( i = 1 ; i <= nrow; i++  )
		{
			fldIndex = i*ncol;
			if ( NULL != result[fldIndex] )
			{
				vlan.vlan_id = atoi ( result[fldIndex] );

			}
			if ( NULL != result[fldIndex + 2] )
			{
				vlan.port_bitmap = atoi(result[fldIndex + 2]);

			}
			vlan.ut_port_bitmap = vlan.port_bitmap;
			vlan.setFlag = 0xFF;
			if ( 0 != SaveAndDelVlan( CTRL_CARDNO , &vlan, C_SW_SAVEVLAN ))
			{
				logger(LOG_ERR, "init control card 's vlan err. ");
			}
			else
			{
				logger(LOG_DEBUG, "init control card 's vlan successful.");
			}
		}

	}
	sqlite3_free_table (result);
	
	sqlite3_close(db);
	
	/*init something about l3 interface  ......*/
	InitL3Interface();

	return 0;
	
}

unsigned char *macTrans(const char *src)
{

	char c;
	int i;
	unsigned char temp, temp2;
	static unsigned char mac[6];
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

unsigned char * getMAC (const char *raw, unsigned char array[] )
{
	unsigned char *pmacchanged ;
	pmacchanged = macTrans (raw);
		array[0] = pmacchanged[0];
		array[1] = pmacchanged[1];
		array[2] = pmacchanged[2];
		array[3] = pmacchanged[3];
		array[4] = pmacchanged[4];
		array[5] = pmacchanged[5];
		return array;
	

}
void configFE8Card(int CardNo)
{
	sqlite3 *db;  
	char *zErrMsg = 0;
	char sql_str[256];
	int rc, i, fldIndex, nrow, ncol;
	char **result;
	int  portIndex;
	char rawmac[12];
	CARD_SET_485FE fe8set ;
	logger (LOG_DEBUG, "here?.\n");
	if( sqlite3_open(CONF_DB, &db) ){
		logger (LOG_ERR,  "DB Error: Can't open database: %s\n", sqlite3_errmsg(db));
		return;
	}

	/* initialize the in-memory cache of  FE8  port configuration to defalut*/
	for(i = 1; i <=8; i++)
	{
		FE8Card[CardNo][i].admin_state = 1; /*enabled*/
		FE8Card[CardNo][i].bw_alloc_recv = (100*1024)/32; /*no control*/
		FE8Card[CardNo][i].bw_alloc_send = (100*1024)/32; /*no control*/	
		logger (LOG_DEBUG,  "init fe8card cardno is %d port  is %d bwalloc_recv is %d\n",CardNo, i , FE8Card[CardNo][i].bw_alloc_recv);
	}
	
	/* get FE8 card configuration from database */
	sprintf(sql_str, "select adminState, CardName  from FE8Card where CardNo=%d;", CardNo);
	rc = sqlite3_get_table(db, 
				sql_str,       /* SQL to be executed */
				&result,       /* Result written to a char *[]  that this points to */			
				&nrow,             /* Number of result rows written here */			
				&ncol,          /* Number of result columns written here */			
				&zErrMsg          /* Error msg written here */); 

		
	if( rc!=SQLITE_OK ){ 
		logger (LOG_ERR,  "DB:initfe8card,  select from FE8Card  Error: %s\n", zErrMsg);
		sqlite3_close(db);
		return ;
	}

	if(nrow < 1)
	{	/*no FE8 card config exist*/
		sqlite3_free_table(result); 
		sqlite3_close(db);
		return; 
	}
	logger (LOG_DEBUG, "before name.\n");
	fldIndex = ncol; /*the first row is column name */
	if ( NULL ==  result[fldIndex]  )
		cardData[CardNo].admin_state = 1;
	else 
	{
		cardData[CardNo].admin_state = atoi(result[fldIndex]);
	}
	if ( NULL == result[fldIndex+1] )
	{
		strncpy(cardData[CardNo].card_name, "card", MAX_DISPLAYNAME_LEN);
	}
	else{
		strncpy(cardData[CardNo].card_name, result[fldIndex+1], MAX_DISPLAYNAME_LEN);
	}
	
	logger (LOG_DEBUG, "after name.\n");
	
	sqlite3_free_table(result); 
	
	/* get FE8 card  port configuration from database */
	sprintf(sql_str, "select PortNo, adminState, portMode, portWM,  bwSend, bwRecv ,boundMac from FE8CardPort where CardNo=%d order by PortNo;", CardNo);
	rc = sqlite3_get_table(db, 
				sql_str,       /* SQL to be executed */
				&result,       /* Result written to a char *[]  that this points to */			
				&nrow,             /* Number of result rows written here */			
				&ncol,          /* Number of result columns written here */			
				&zErrMsg          /* Error msg written here */); 
	
	if( rc!=SQLITE_OK ){ 
		logger (LOG_ERR,  "DB:initfe8 ,select portinfo  Error: %s\n", zErrMsg);
		sqlite3_close(db);
		return ;
	}

	if(nrow < 1)
	{	/*no FE8 card config exist*/
		sqlite3_free_table(result); 
		sqlite3_close(db);
		return; 
	}
	else
	{

		for(i=1 ; i <= nrow; i++)      
		{
			fldIndex = i*ncol;
			if ( NULL != result[fldIndex] )	
			{
				portIndex = atoi(result[fldIndex]);
			}
			if ( NULL != result[fldIndex + 1] )	
			{
				FE8Card[CardNo][portIndex].admin_state = atoi(result[fldIndex + 1]);
			}
			if ( NULL != result[fldIndex + 2] )	
			{
				FE8Card[CardNo][portIndex].portMode = atoi(result[fldIndex + 2]);
			}
			if ( NULL != result[fldIndex + 3] )	
			{
				FE8Card[CardNo][portIndex].portWorkingMode = atoi(result[fldIndex + 3]);
				logger (LOG_DEBUG, "fe8card init workingmode is %d - %d \n" , atoi(result[fldIndex + 3]), FE8Card[CardNo][portIndex].portWorkingMode);
			}
			
			if ( NULL != result[fldIndex + 4] )	
			{
				FE8Card[CardNo][portIndex].bw_alloc_send = atoi(result[fldIndex + 4]);
			}

			if ( NULL != result[fldIndex + 5] )	
			{
				FE8Card[CardNo][portIndex].bw_alloc_recv = atoi(result[fldIndex + 5]);
			}

			if ( NULL != result[fldIndex + 6] )	
			{
				memcpy(rawmac, result[fldIndex + 6] ,12);
				logger  (LOG_DEBUG, "rawmac is %s\n",rawmac );
				getMAC(rawmac, FE8Card[CardNo][portIndex].bounded_mac );
				logger (LOG_DEBUG,  " fe8card'mac address is %02X%02X%02X%02X%02X%02X\n", FE8Card[CardNo][portIndex].bounded_mac[0], FE8Card[CardNo][portIndex].bounded_mac[1],FE8Card[CardNo][portIndex].bounded_mac[2],FE8Card[CardNo][portIndex].bounded_mac[3],FE8Card[CardNo][portIndex].bounded_mac[4], FE8Card[CardNo][portIndex].bounded_mac[5]);
			}
			
			

			fe8set.admin_state = cardData[CardNo].admin_state;
			fe8set.Index = portIndex;
			fe8set.port.admin_state = FE8Card[CardNo][portIndex].admin_state;
			fe8set.port.portMode = FE8Card[CardNo][portIndex].portMode;
			fe8set.port.portWorkingMode = FE8Card[CardNo][portIndex].portWorkingMode;
			fe8set.port.bw_alloc_send = FE8Card[CardNo][portIndex].bw_alloc_send;
			fe8set.port.bw_alloc_recv = FE8Card[CardNo][portIndex].bw_alloc_recv;
			memcpy(fe8set.port.bounded_mac, FE8Card[CardNo][portIndex].bounded_mac, 6);
			//send out the configure command
			logger (LOG_DEBUG ,  " set card fe8 portindex is %d\n", fe8set.Index);
			logger (LOG_DEBUG, "fe8card set portWorkingMode is %d\n", fe8set.port.portWorkingMode);
			if(SetCardFE8((unsigned char )CardNo , &fe8set) !=0)
				logger(LOG_ERR,  "Error: Configure FE8 card failed");
			
		}
		
		sqlite3_free_table(result); 
		sqlite3_close(db);
	}
		
	
}

void rmConfigFE8Card(int CardNo)
{
	sqlite3 *db;  
	char *zErrMsg = 0;
	char sql_str[256];
	int rc;

	if( sqlite3_open(CONF_DB, &db) ){
		logger (LOG_ERR , "DB Error: Can't open database: %s\n", sqlite3_errmsg(db));
		return;
	}

	sprintf(sql_str, "delete from FE8Card where CardNo=%d;", CardNo);
	rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
	if(rc != SQLITE_OK) {
		logger (LOG_ERR,  "DB Error: err_code=%s\n", sqlite3_errmsg(db));
	}

	sprintf(sql_str, "delete from FE8CardPort where CardNo=%d;", CardNo);
	rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
	if(rc != SQLITE_OK) {
		logger (LOG_ERR,  "DB Error: err_code=%s\n", sqlite3_errmsg(db));
	}
	
	sqlite3_close(db);	

}

int updateONU100 ( FSA600_STATE_485 * state , unsigned char cardno )
{
	sqlite3 *db;
	char sql_str [256];
	char *zErrMsg;
	int i, rc;

	char mask = 0x01;

	/*store the sfpstate and fxlinkstate in the arrrays.*/
	for ( int j = 0; j < 6; j++ )
	{

		if ( mask ==  (state->Fxlink & mask) )
		{
			fxlink_state[j] = 1;
		}
		else{
			fxlink_state[j] = 0;
		}

		if ( mask ==   (state->SfpState  & mask) )
		{
			sfp_state[j] = 1;
		}
		else{
			sfp_state[j] = 0;
		}
		//fxlink_state[i] = state->Fxlink & mask; 
		//sfp_state[i] = state->SfpState & mask;
		logger  (LOG_DEBUG , "DEBUG:mask is %d fxlink is %d sfp state is %d\n", mask, fxlink_state[j], sfp_state[j] );
		mask = mask << 1;
	}
	
	if ( sqlite3_open ( DYNAMIC_DB, &db ) )
	{
		logger  (LOG_ERR , "DB err cannot  open dynamic db for updateonu100 :%s\n", sqlite3_errmsg( db ));
		return ERROR;
	}

	/*first ,delete the records associated with the card number.then ,insert new records. */
	sprintf ( sql_str, "delete from ONUList where CardNo=%d", cardno );
	rc = sqlite3_exec ( db , sql_str, 0, 0, &zErrMsg);
	if ( rc != SQLITE_OK )
	{
		logger  (LOG_ERR, "database drop err :%s\n" , zErrMsg );
		sqlite3_close (db);
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
				sqlite3_close (db);
				return ERROR;
				
			}
		}
	}

	sqlite3_close ( db );
	return SUCCESS;

	
}

#if 1  // 201611
int updateMTTList ( unsigned char cardno, MTT_STATE * state  )
{
    sqlite3 *db;
    char sql_str [256];
    char *zErrMsg;
    int i, rc;


    char mask = 0x01;

    /*store the ntustate and fxlinkstate in the arrrays.*/


    if ((state->FD1State & 0x03) == 0x03)    link_state[0] = 1;
    else link_state[0] = 0;
    if ((state->FD2State & 0x03) == 0x03)    link_state[1] = 1;
    else link_state[1] = 0;
    

    if ( sqlite3_open ( DYNAMIC_DB, &db ) )
    {
        logger  (LOG_ERR , "DB err cannot  open dynamic db for updatemtt :%s\n", sqlite3_errmsg( db ));
        return ERROR;
    }

    /*first ,delete the records associated with the card number.then ,insert new records. */
    //sprintf ( sql_str, "delete from NTUList where CardNo=%d", cardno );
    sprintf ( sql_str, "delete from MTUList where CardNo=%d", cardno );
    rc = sqlite3_exec ( db , sql_str, 0, 0, &zErrMsg);
    if ( rc != SQLITE_OK )
    {
        logger  (LOG_ERR, "database drop err :%s\n" , zErrMsg );
        sqlite3_close (db);
        return ERROR;
    }

    for ( i = 0; i < MAX_MTU_NUM; i++ )
    {
        if ( 0 != link_state[i]) /*fx linked*/
        {
            //sprintf (sql_str,"insert into NTUList (CardNo,NtuId)  values (%d,%d)" , cardno, i+1);
            sprintf (sql_str,"insert into MTUList (CardNo,MtuId)  values (%d,%d)" , cardno, i+1);
            rc = sqlite3_exec (db , sql_str, 0, 0, &zErrMsg);
            if ( rc != SQLITE_OK )
            {
                logger  (LOG_ERR, "insert mtu into mtulist err:%s sql is %s!\n", zErrMsg , sql_str );
                sqlite3_close (db);
                return ERROR;
                
            }
        }
    }

    sqlite3_close ( db );
    return SUCCESS;
}
#endif

#if 1  // 201708
int updateMTT411List ( unsigned char cardno, MTT_411_STATE * state  )
{
    sqlite3 *db;
    char sql_str [256];
    char *zErrMsg;
    int i, rc;


    char mask = 0x01;

    /*store the ntustate and fxlinkstate in the arrrays.*/
    if (state->mtu1_optic == 2) // 故障
        link_state[0] =  0;
    else
        link_state[0] =  1;

     if (state->mtu2_optic == 2)
        link_state[1] =  0;
     else
        link_state[1] =  1;
            

    if ( sqlite3_open ( DYNAMIC_DB, &db ) )
    {
        logger  (LOG_ERR , "DB err cannot  open dynamic db for updatemtt :%s\n", sqlite3_errmsg( db ));
        return ERROR;
    }

    /*first ,delete the records associated with the card number.then ,insert new records. */
    sprintf ( sql_str, "delete from MTUList where CardNo=%d", cardno );
    rc = sqlite3_exec ( db , sql_str, 0, 0, &zErrMsg);
    if ( rc != SQLITE_OK )
    {
        logger  (LOG_ERR, "database drop err :%s\n" , zErrMsg );
        sqlite3_close (db);
        return ERROR;
    }

    for ( i = 0; i < MAX_MTU_NUM; i++ )
    {
        if ( 0 != link_state[i]) /*fx linked*/
        {
            sprintf (sql_str,"insert into MTUList (CardNo,MtuId)  values (%d,%d)" , cardno, i+1);
            rc = sqlite3_exec (db , sql_str, 0, 0, &zErrMsg);
            if ( rc != SQLITE_OK )
            {
                logger  (LOG_ERR, "insert mtu into mtulist err:%s sql is %s!\n", zErrMsg , sql_str );
                sqlite3_close (db);
                return ERROR;
                
            }
        }
    }

    sqlite3_close ( db );
    return SUCCESS;
}


int updateMTT441List ( unsigned char cardno, MTT_441_STATE * state  )
{
    sqlite3 *db;
    char sql_str [256];
    char *zErrMsg;
    int i, rc;


    char mask = 0x01;

    /*store the ntustate and fxlinkstate in the arrrays.*/


   if (state->mtu1_optical)    link_state[0] = 1;
   else                        link_state[0] = 0;
   if (state->mtu2_optical)    link_state[1] = 1;
   else                        link_state[1] = 0;
    

    if ( sqlite3_open ( DYNAMIC_DB, &db ) )
    {
        logger  (LOG_ERR , "DB err cannot  open dynamic db for updatemtt :%s\n", sqlite3_errmsg( db ));
        return ERROR;
    }

    /*first ,delete the records associated with the card number.then ,insert new records. */
    sprintf ( sql_str, "delete from MTUList where CardNo=%d", cardno );
    rc = sqlite3_exec ( db , sql_str, 0, 0, &zErrMsg);
    if ( rc != SQLITE_OK )
    {
        logger  (LOG_ERR, "database drop err :%s\n" , zErrMsg );
        sqlite3_close (db);
        return ERROR;
    }

    for ( i = 0; i < MAX_MTU_NUM; i++ )
    {
        if ( 0 != link_state[i]) /*fx linked*/
        {
            sprintf (sql_str,"insert into MTUList (CardNo,MtuId)  values (%d,%d)" , cardno, i+1);
            rc = sqlite3_exec (db , sql_str, 0, 0, &zErrMsg);
            if ( rc != SQLITE_OK )
            {
                logger  (LOG_ERR, "insert mtu into mtulist err:%s sql is %s!\n", zErrMsg , sql_str );
                sqlite3_close (db);
                return ERROR;
                
            }
        }
    }

    sqlite3_close ( db );
    return SUCCESS;
}
#endif



int updateNTTList ( unsigned char cardno,NTT_STATE * state  )
{
	sqlite3 *db;
	char sql_str [256];
	char *zErrMsg;
	int i, rc;


	char mask = 0x01;

	/*store the ntustate and fxlinkstate in the arrrays.*/

	for ( int j = 0; j < MAX_NTU_NUM; j++ )
	{
		if ( mask ==  (state->link & mask) )
		{
			link_state[j] = 1; // 上线
		}
		else
		{
			link_state[j] = 0;  // 下线
		}
	
		logger  (LOG_DEBUG , "DEBUG:mask is %d fxlink is %d \n", mask, link_state[j]);
		mask = mask << 1;
	}

	if ( sqlite3_open ( DYNAMIC_DB, &db ) )
	{
		logger  (LOG_ERR , "DB err cannot  open dynamic db for updatentt :%s\n", sqlite3_errmsg( db ));
		return ERROR;
	}

	/*first ,delete the records associated with the card number.then ,insert new records. */
	sprintf ( sql_str, "delete from NTUList where CardNo=%d", cardno );
	rc = sqlite3_exec ( db , sql_str, 0, 0, &zErrMsg);
	if ( rc != SQLITE_OK )
	{
		logger  (LOG_ERR, "database drop err :%s\n" , zErrMsg );
		sqlite3_close (db);
		return ERROR;
	}

	for ( i = 0; i < MAX_NTU_NUM; i++ )
	{
		if ( 0 != link_state[i]) /*fx linked*/
		{
			sprintf (sql_str,"insert into NTUList (CardNo,NtuId)  values (%d,%d)" , cardno, i+1);
			rc = sqlite3_exec (db , sql_str, 0, 0, &zErrMsg);
			if ( rc != SQLITE_OK )
			{
				logger  (LOG_ERR, "insert ntu into ntulist err:%s sql is %s!\n", zErrMsg , sql_str );
				sqlite3_close (db);
				return ERROR;
				
			}
		}
	}

	sqlite3_close ( db );
	return SUCCESS;

	
}


int configFSA600AndRMT (unsigned char cardno )
{

	sqlite3 *db;
	char sql_str [256];
	char *zErrMsg;
	int ncol, nrow,i, rc, fldIndex ;
	char **result;
	FSA600_SET_485 config600;
	FSA600_SETRMT configonu100;
	int Index, resetFlag;/*setdefault:2 , set:*.*/

	configonu100.setFlag = 0xFF;
	config600.setFlag = 0xFF;

	if ( sqlite3_open ( CONF_DB, &db ) )
	{
		logger  (LOG_ERR , "DB err cannot  open configure database for configfsa600 card :%s\n", sqlite3_errmsg( db ));
		return ERROR;
	}
	
	/*init local modules fsa600 card name*/
	sprintf (sql_str, "select CardName from FE8Card where CardNo=%d", cardno);
	rc = sqlite3_get_table(db, sql_str, &result, &nrow, &ncol, &zErrMsg);
	if( rc!=SQLITE_OK ){ 
		logger (LOG_ERR,  "DB:init fsa600 name select  err: %s\n", zErrMsg);
		sqlite3_close(db);
		return ERROR ;
	}
	logger (LOG_DEBUG, "init fsa600 name select.nrow is %d, ncol is %d\n", nrow, ncol);

	/*if the related records does not exist, this code will be skipped.*/
	for ( i = 1; i <= nrow; i++)
	{
		
		fldIndex = i * ncol;
		if ( NULL !=  result[fldIndex] )
		{
			strncpy(cardData[cardno].card_name, result[fldIndex], MAX_DISPLAYNAME_LEN);
		}
	}
	
	sqlite3_free_table(result);
	logger (LOG_DEBUG, "init fsa600 name successful!");
	
	/*init local modules Configuration data*/
	sprintf ( sql_str, "select localIndex, portState, transmitmMode, upStreamFlag,upStream, downStreamFlag, downStream , resetFlag from FSA600 where CardNo=%d", cardno );
	rc = sqlite3_get_table(db, sql_str,  &result,   &nrow,  &ncol,   &zErrMsg   ); 
	if( rc!=SQLITE_OK ){ 
		logger (LOG_ERR,  "DB:init fsa600 err ,select from FSA600 table  Error: %s\n", zErrMsg);
		sqlite3_close(db);
		return ERROR ;
	}
	logger (LOG_DEBUG, "start to init fsa600.nrow is %d, ncol is %d\n", nrow, ncol);
	
	/*if the related records does not exist, this code will be skipped.*/
	for ( i = 1; i <= nrow; i++)
	{
		
		fldIndex = i * ncol;

		if ( NULL !=  result[fldIndex] )
		{
			Index = atoi (result[fldIndex]);
		}

		if ( NULL !=  result[fldIndex + 1] )
		{
			config600.PortState = atoi(result[fldIndex + 1]);
		}

		if ( NULL !=  result[fldIndex + 2] )
		{
			config600.TransmitMode = atoi(result[fldIndex + 2]);
		}
		
		if ( NULL !=  result[fldIndex + 3] )
		{
			config600.UpStreamFlag = atoi(result[fldIndex + 3]);
		}

		if ( NULL !=  result[fldIndex + 4] )
		{
			config600.UpStream = atoi(result[fldIndex + 4]);
		}

		if ( NULL !=  result[fldIndex+ 5] )
		{
			config600.DownStreamFlag = atoi (result[fldIndex+ 5]);
		}

		if ( NULL !=  result[fldIndex + 6] )
		{
			config600.DownStream = atoi (result[fldIndex + 6]);
		}
		
		if ( NULL !=  result[fldIndex + 7] )
		{
			resetFlag = atoi (result[fldIndex + 7]);
		}
		
		logger (LOG_DEBUG, "start to init fsa600.resetflag is %d\n", resetFlag);

		 if ( 2 == resetFlag  )
		{
			
			if ( 0 != FSA600resetAndDFT (cardno, Index, C_FSA600_SETDFT) )
			{
				logger (LOG_ERR, "init fsa600:set FSA600 default err. cardno =%d--setflag=%d\n", cardno, resetFlag);
			}
			else{

				logger (LOG_DEBUG,"init fsa600 set FSA600 default successful !  cardno=%d--setflag=%d \n", cardno , resetFlag);
				sprintf (sql_str, "delete from FSA600 where CardNo=%d and localIndex=%d and resetFlag=%d", cardno, Index, resetFlag);
				rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
				if(rc != SQLITE_OK) 
				{
					logger ( LOG_ERR , "Database drop fsa600 record error: %s\n", sqlite3_errmsg(db));
				 }
			}	
		}
		else {

			if ( 0 != SetFSA600 (cardno, Index, &config600) )
			{
				logger (LOG_ERR, "init fsa600:set FSA600 err. cardno =%d\n", cardno);
			}
			else{

				logger (LOG_DEBUG,"init fsa600 set FSA600 successful !  cardno=%d \n", cardno );
			}


		}
		
	}
	
	sqlite3_free_table(result);
	
	logger (LOG_DEBUG, "start to init onu100.\n");
	/*init remote modules configuration data.*/
	for (int j = 0; j <= 5; j++)
	{
		if ( 1 == fxlink_state[j] )
		{	
			sprintf ( sql_str, "select transmitmMode, cfgWorkMode1, cfgWorkMode2 ,resetFlag, onuName  from ONU100 where CardNo=%d and onuId=%d", cardno, j+1 );
			rc = sqlite3_get_table(db, sql_str,  &result, &nrow,  &ncol,   &zErrMsg ); 
			if( rc!=SQLITE_OK ){ 
				logger (LOG_ERR,  "DB:init onu100 err ,select from ONU100 table  Error: %s\n", zErrMsg);
				sqlite3_close(db);
				return ERROR ;
			}

			/*if the related records does not exist, this code will be skipped.*/
			logger (LOG_DEBUG, "init onu100  select.nrow is %d, ncol is %d\n", nrow, ncol);	
			for ( i = 1; i <= nrow; i++)
			{
				fldIndex = i * ncol;
				Index = j+1 ; //onuID 
				configonu100.OnuId = Index;

				if ( NULL !=  result[fldIndex] )
				{
					configonu100.TransmitMode = atoi(result[fldIndex]);
				}

				if ( NULL !=  result [fldIndex + 1] )
				{
					configonu100.CfgWorkMode1 = atoi(result [fldIndex + 1]);
				}

				if ( NULL != result[fldIndex + 2] )
				{
					configonu100.CfgWorkMode2 = atoi (result[fldIndex + 2]);
				}

				if ( NULL != result[fldIndex + 3] )
				{
					resetFlag = atoi (result[fldIndex + 3]);
				}

				if ( NULL != result[fldIndex + 4 ] )
				{
					//strncpy(onu100_Info[cardno][j+1].onu_name, result[fldIndex + 4 ], MAX_DISPLAYNAME_LEN);
					strncpy(RMT_Data[cardno][j+1].onu_name , result[fldIndex + 4 ], MAX_DISPLAYNAME_LEN);
					memcpy(&configonu100.onu_name, result[fldIndex + 4 ], MAX_DISPLAYNAME_LEN );

				}
				
				 if ( 2 == resetFlag  )
				{
					
					if ( 0 != FSA600resetAndDFT (cardno, Index, C_FSA600_SETDFTRMT) )
					{
						logger (LOG_ERR, "init fsa600:set onu100 default err. cardno =%d--setflag=%d\n", cardno, resetFlag);
					}
					else{

						logger (LOG_DEBUG,"init fsa600 set onu100 default successful !  cardno=%d--setflag=%d \n", cardno , resetFlag);
						sprintf (sql_str, "delete from ONU100 where CardNo=%d and onuId=%d and resetFlag=%d", cardno, Index, resetFlag);
						rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
						if(rc != SQLITE_OK) 
						{
							logger ( LOG_ERR , "Database drop onu100 record error: %s\n", sqlite3_errmsg(db));
						 }
					}	
				}
				else{

					if ( 0 != SetOnu100 (cardno, Index, &configonu100) )
					{
						logger (LOG_ERR, "init onu100:set onu100 err. cardno =%d\n", cardno);
					}
					else{

						logger (LOG_DEBUG,"init onu100 set onu100 successful !  cardno=%d \n", cardno );
					}
				}
				
			}
			sqlite3_free_table(result);
	
		}
	}

	
	sqlite3_close(db);
	return SUCCESS;

}

int  getONU100 ( int  cardno ) //get the number of sfp inserted and FXlink state.
{

	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	FSA600_STATE_485 *state_rsp;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_FSA600_STATE;
	headReq->src = CTRL_CARDNO;
	headReq->length = 0;
		
	headReq->dst = cardno;
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR, "Error: Get ONU100 error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(FSA600_STATE_485))
		{
			logger ( LOG_ERR, "Error: Get ONU100 response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(FSA600_STATE_485));
			return(ERROR_COMM_PARSE);
		}
			
		state_rsp = (FSA600_STATE_485 *) (recvbuff + sizeof(MSG_HEAD_485));	
		memcpy( &fsa600_state, state_rsp , sizeof(FSA600_STATE_485));
		return 0;
	}
	return(ERROR_COMM_485);

}

int GetNttState(int cardno, NTT_STATE *nttstate)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	NTT_STATE    *state_rsp;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_NTT_GETSTATE;
	headReq->src   = CTRL_CARDNO;
	headReq->dst = cardno;
	headReq->length = 0;		
		
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR, "Error: Get ntt error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(NTT_STATE))
		{
			logger ( LOG_ERR, "Error: Get ntt response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(NTT_STATE));
			return(ERROR_COMM_PARSE);
		}

		state_rsp = (NTT_STATE *) (recvbuff + sizeof(MSG_HEAD_485));	
	
		memcpy( nttstate, state_rsp , sizeof(NTT_STATE));
	
		return 0;
	}
	return(ERROR_COMM_485);
}

#if 1 // 201611
int GetMttState(int cardno, MTT_STATE *mttstate)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	MTT_STATE    *state_rsp;
	int  i;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_MTT_GETSTATE;
	headReq->src   = CTRL_CARDNO;
	headReq->dst = cardno;
	headReq->length = 0;		
		
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR, "Error: Get ntt error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(MTT_STATE))
		{
			logger ( LOG_ERR, "Error: Get ntt response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(NTT_STATE));
			return(ERROR_COMM_PARSE);
		}

		state_rsp = (MTT_STATE *) (recvbuff + sizeof(MSG_HEAD_485));
		printf("recv mtt info:\n");
		for (i = 0; i < sizeof(MSG_HEAD_485)+sizeof(MTT_STATE); i++)
		    printf("%02X ", (unsigned char)recvbuff[i]);
		printf("\n");
	
		memcpy( mttstate, state_rsp , sizeof(MTT_STATE));
	
		return 0;
	}
	return(ERROR_COMM_485);
}
#endif

#if 1 // 201611
int GetMtt411State(int cardno, MTT_411_STATE *mttstate)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	MTT_STATE    *state_rsp;
	int  i;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_MTT_GETSTATE;
	headReq->src   = CTRL_CARDNO;
	headReq->dst = cardno;
	headReq->length = 0;		
		
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR, "Error: Get ntt error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(MTT_411_STATE))
		{
			logger ( LOG_ERR, "Error: Get ntt response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(NTT_STATE));
			return(ERROR_COMM_PARSE);
		}

		state_rsp = (MTT_411_STATE *) (recvbuff + sizeof(MSG_HEAD_485));
		printf("recv mtt info:\n");
		for (i = 0; i < sizeof(MSG_HEAD_485)+sizeof(MTT_411_STATE); i++)
		    printf("%02X ", (unsigned char)recvbuff[i]);
		printf("\n");
	
		memcpy( mtt_411_state, state_rsp , sizeof(MTT_411_STATE));
	
		return 0;
	}
	return(ERROR_COMM_485);
}

int GetMtt441State(int cardno, MTT_441_STATE *mttstate)
{
	char 			sendbuff[MAX_485MSG_FULLLEN];
	char 			recvbuff[MAX_485MSG_FULLLEN];
	MSG_HEAD_485 	*headReq, *headRsp;
	MTT_441_STATE    *state_rsp;
	int  i;
	
	memset(sendbuff, 0, sizeof(MSG_HEAD_485));
	headReq = (MSG_HEAD_485*)sendbuff;
	headRsp = (MSG_HEAD_485*)recvbuff;
	headReq->code = C_MTT_GETSTATE;
	headReq->src   = CTRL_CARDNO;
	headReq->dst = cardno;
	headReq->length = 0;		
		
	if(!msg485_sendwaitrsp(fd485, sendbuff, recvbuff , TIMEOUT_485MSG))
	{	
			/* Got the response*/
		if(headRsp->rsp != 0)
		{
			logger (LOG_ERR, "Error: Get ntt error response !,  error:%d.\n",  headRsp->rsp);
			return(headRsp->rsp);
		}				

		if(headRsp->length != sizeof(MTT_441_STATE))
		{
			logger ( LOG_ERR, "Error: Get ntt response length error!,  length:%d, expect:%d.\n",  headRsp->length, sizeof(NTT_STATE));
			return(ERROR_COMM_PARSE);
		}

		state_rsp = (MTT_441_STATE *) (recvbuff + sizeof(MSG_HEAD_485));
		printf("recv mtt info:\n");
		for (i = 0; i < sizeof(MSG_HEAD_485)+sizeof(MTT_441_STATE); i++)
		    printf("%02X ", (unsigned char)recvbuff[i]);
		printf("\n");
	
		memcpy( mtt_441_state, state_rsp , sizeof(MTT_441_STATE));
	
		return 0;
	}
	return(ERROR_COMM_485);
}
#endif

/* *******************************************************************
* 	check whether the card is up											
*   this function is called periodically by main, it send out the message to the card, 
*   If response is received, the card will be marked up and a new record will be 
*   inserted to the dynamic database and card_detected alarm will be issued.
************************************************************************/
void check_cardUP()
{
	static int curCard= 1;
	int 		loopEnd, endCard;
	CARD_INFO card_info;

	int timeout;
	sqlite3 *db;  
	char *zErrMsg = 0;
	char sql_str[256];
	int rc;

	timeout = 0;
	db = NULL;	
	loopEnd = 0;
	endCard = curCard;
	
	/* we will send the get card message one by one, and deal with no more than 2 times message timeout, 
	    then yeild to the main process, and let the unscaned card in the next checking round*/
	while(timeout < 2 && !loopEnd)  
	{
		curCard ++;
		if(curCard > MAX_CARD_NUM_UTN)
			curCard = 1;
		
		if(curCard == endCard) 
			loopEnd = 1;   /* all the cards have been checked in this round */
		
		/* if the card is already in the system, do not send out the checking message */
		if(cardData[curCard].card_no != 0)
			continue;

		memset(&card_info, 0, sizeof(CARD_INFO));
		card_info.card_no = curCard;

		if(curCard == CTRL_CARDNO)
			card_info.card_no = GW_CARDNO; 			//send message to Giga swicth card;

		if(GetCardInfo(&card_info) != 0)
		{
			logger (LOG_DEBUG , "Debug, Get card info error, card=%d \n", curCard);
			timeout ++;
			continue;
		} 

		if((card_info.card_no != curCard) && ((card_info.card_no != GW_CARDNO)))
		{
			logger (LOG_ERR ,  "Error: weired, message process error!\n");
			continue;
		}

		/* insert the card into dynamic database */
		if(db == NULL)
		{
			if( sqlite3_open(DYNAMIC_DB, &db) ){
				printf( "DB Error: Can't open database: %s\n", sqlite3_errmsg(db));
				continue;
			}
		}
		sprintf(sql_str, "insert into Card(CardNo, CardType) values(%d, %d);", card_info.card_no, card_info.card_type);
		rc = sqlite3_exec(db, sql_str, 0, 0, &zErrMsg);
		if(rc != SQLITE_OK) {
			logger (LOG_ERR , "DB Error: Insert card= %s, err_code=%d\n", sqlite3_errcode(db), card_info.card_no );
		}

		memcpy(&(cardData[curCard]), &card_info, sizeof(card_info));
		logger (LOG_DEBUG , "current card type%d\n",cardData[curCard].card_type);
		
		/* Generate the alarm */
		genCardAlarm(db, ATYPE_CARDONLINE, getServerity(ATYPE_CARDONLINE), card_info.card_no, 0);

		if(cardData[curCard].card_type == CARD_TYPE_FE8)
			configFE8Card(curCard);
		else if ( cardData[curCard].card_type == CARD_TYPE_FSA600 )
		{
			rc = getONU100 ( curCard );
			if (  rc != 0 )
			{
				logger  (LOG_ERR , "get onu100 err %dl!\n", rc );
			}

			rc  = updateONU100( &fsa600_state, curCard );
			if ( 0 !=  rc )
			{
				logger (LOG_ERR ,  "call updateONU100 err !\n");
			}
			
			rc = configFSA600AndRMT ( curCard );
			if ( rc != 0 )
			{
				logger (LOG_ERR, "configure fsa600 and onu100 err %d !\n", rc );
			}

			rc = getAlarm( curCard,  &fsa600_alarm[curCard]);
			if ( rc != 0 )
			{
				logger (LOG_ERR, "first time get fsa600 alarm  err %d !\n", rc );
			}
		}
#ifndef __MTT__		// 201611
		else if(cardData[curCard].card_type == CARD_TYPE_NTT
		        || cardData[curCard].card_type == CARD_TYPE_NTT_S)  // 20150115
#else 		
        else if(cardData[curCard].card_type == CARD_TYPE_NTT_S)  // 20150115
#endif
		{
			rc = GetNttState(curCard,&ntt_state);
			if( rc != 0)
			{
				logger (LOG_ERR,"get ntt err %d!\n",rc);
			}
			updateNTTList(curCard, &ntt_state);
			
		}
#if 0		
#ifdef __MTT__		// 201611
		else if(cardData[curCard].card_type == CARD_TYPE_MTT)
		{
            rc = GetMttState(curCard, &mtt_state);
			if( rc != 0)
			{
				logger (LOG_ERR,"get mtt err %d!\n",rc);
			}
			updateMTTList(curCard, &mtt_state);
		}
#endif
#endif
#if 1 // 201708
		else if(cardData[curCard].card_type == CARD_TYPE_MTT_411)
		{
            rc = GetMtt411State(curCard, &mtt_state);
			if( rc != 0)
			{
				logger (LOG_ERR,"get mtt err %d!\n",rc);
			}
			updateMTT411List(curCard, &mtt_411_state);
		}
		else if(cardData[curCard].card_type == CARD_TYPE_MTT_441)
		{
            rc = GetMtt441State(curCard, &mtt_441_state);
			if( rc != 0)
			{
				logger (LOG_ERR,"get mtt err %d!\n",rc);
			}
			updateMTT441List(curCard, &mtt_441_state);
		}
#endif		
		else if(cardData[curCard].card_type == CARD_TYPE_EPN)
		{
			UpdateEPOnuList(db, curCard);			
		}
		else 
		{
			rmConfigFE8Card(curCard);
			if(cardData[curCard].card_type == CARD_TYPE_FSA)
				UpdateOnuList(db, curCard);
		}

		setRefreshState(CHANGE_FLAG_TREEV|CHANGE_FLAG_FRAMEV);

	}

	
	if( db != NULL )
		sqlite3_close(db);	
	
}


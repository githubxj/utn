#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <netinet/in.h>

#include "CgiRequest.h"
#include "CGISession.h"
#include "Lang.h"
#include "msg.h"
#include "Comm.h"
#include "CGetSfp.h"

using namespace std;


const char *CGetSfp::getTransMode(unsigned char type)
{	switch(type)
	{		
			case 1:			return("SingleMode");
			case 2:			return("MultiMode");
			default:			return("unsupported");
	}
}


const char *CGetSfp::getInterfaceType(unsigned char type)
{

	if ( 1 == type )
		{return("SC");}
	else if ( 7 == type )
	{
		return("LC");
	}
	else if (34 == type )
	{
		return ("RJ45");
	}
	else {
		return("unsupported");

	}
	/*switch(type)
	{
		case 1:	return("SC");
		case 7: 	return("LC");
		case 34:	return ("RJ45");
		default:	return("unsupported");
	}*/
}



int CGetSfp::getSFPInfo()
{
	std::string card_no = getReqValue("cardno");
	std::string onuno = getReqValue ("onunode");
	std::string sfpno = getReqValue ("sfpno");

	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	SFP_INFO *sfp_info;
	char buff[32];	
	char field_name[32];
	
	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	msg_header->length = sizeof(SFP_INFO);
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	msg_header->node_id = (unsigned char )std::atoi(onuno.c_str() );
	
	msg_header->msg_code = C_SFP_GET;
	sfp_info = (SFP_INFO*) (send_buff + sizeof (MSG_HEAD_CMD ));
	sfp_info->sfpIndex = (unsigned char )std::atoi(sfpno.c_str() );


	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
		
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);
	

	if(msgrsp_header->length != sizeof(SFP_INFO))
		return(ERROR_COMM_PARSE);
	
	sfp_info = (SFP_INFO*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
	if (sfp_info->sfpCompliance == 0xFF )
		return ERR_SFP_INCOMPATIBLE;
	
	sfp_info->sfpVendor[MAX_DISPLAYNAME_LEN-1] = '\0';
	addUserVariable("sfp_vendor", sfp_info->sfpVendor);

	//sfp_info->sfpPartNumber[MAX_DISPLAYNAME_LEN-1] = '\0';
	memcpy(buff, sfp_info->sfpPartNumber,MAX_DISPLAYNAME_LEN);
	buff[MAX_DISPLAYNAME_LEN] = '\0';
	//sprintf(buff, "%s",sfp_info->sfpPartNumber );
	addUserVariable("part_num", buff );
	
	sprintf (buff, "%d", sfp_info->sfpCompliance );
	addUserVariable("compliance", buff);
	if ( sfp_info->sfpTransCode & 0x01 )
	{
		addUserVariable("trans_mode", getTransMode(1));
		sprintf(buff,"%d  km",sfp_info->sfpSmLength );
		addUserVariable("trans_len", buff );
	}
	else if ( (sfp_info->sfpTransCode & 0x02) ||(sfp_info->sfpTransCode &0x04) )
	{
		addUserVariable("trans_mode", getTransMode(2));
		sprintf(buff,"%d  m",sfp_info->sfpSmLength*10 );
		addUserVariable("trans_len", buff );
	}
	else{
		addUserVariable("trans_mode", getTransMode(3));

	}

	sprintf(buff,"%d  Mbps", sfp_info->sfpBrSpeed *100 );
	addUserVariable("trans_speed", buff);

	addUserVariable("connect_type" , getInterfaceType(sfp_info->sfpConnector));
	
	sprintf(buff,"%d ", sfp_info->sfpTemperature);
	addUserVariable("temperature", buff);
		
	sprintf(buff,"%d  nm", ntohs(sfp_info->sfpWavelength));
	addUserVariable("wave_len", buff);
		
	sprintf(buff,"%d  m", sfp_info->sfpCopperLength );
	addUserVariable("copper_len", buff);
	sprintf(buff, "%d  uW", ntohs(sfp_info->sfpTranPower)/10);
	addUserVariable("tran_power" , buff);
	
	sprintf(buff, "%d  uW", ntohs(sfp_info->sfpRecvPower)/10);
	addUserVariable("recv_power" , buff);	
	
	m_desthtml = "sfp.html";

	
	
	

	return 0;

}

void CGetSfp::output()
{
	CCGISession* cs;	

    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 

	int ret = getSFPInfo();
	
	if(ret == 0) {
		if(outputHTMLFile(m_desthtml, true) != 0)
		{  
			outputError("Cann't open sfp.html\n");
		}	
	} 
	
	else 
		outputError(multilang::getError(ret));

	free(cs);

}


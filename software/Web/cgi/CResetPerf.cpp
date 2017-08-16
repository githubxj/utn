#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

#include "CgiRequest.h"
#include "CGISession.h"
#include "Lang.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"

#include "CResetPerf.h"


int CResetPerf::ResetPerf()//for all port perf resert .
{

		std::string card_no = getReqValue("cardno");
		std::string port_no = getReqValue("portno");
		std::string onu_no = getReqValue("onuno");
		char send_buff[MAX_CMDBUFF_SIZE];
		char recv_buff[MAX_CMDBUFF_SIZE];
		MSG_HEAD_CMD *msg_header;
		MSG_HEAD_CMD_RSP *msgrsp_header;
		FE_PORTINDEX *feportindex;
		
		msg_header = (MSG_HEAD_CMD *)send_buff;
		feportindex = (FE_PORTINDEX *)(send_buff+sizeof(MSG_HEAD_CMD));
		memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
		msg_header->msg_code = SET_PERF_RESET;
		msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
		msgrsp_header->node_id  = (unsigned char) std::atoi(onu_no.c_str());
		msg_header->length = sizeof(FE_PORTINDEX);
		feportindex->portIndex = (unsigned char) std::atoi(port_no.c_str());

		m_desthtml = "perf.html";

		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		if(msgrsp_header->length != 0)
			return(ERROR_COMM_PARSE);



		std::string s = "0";
		addUserVariable("port_recv", s );
		addUserVariable("port_send", s );
		
		
		return 0;
		
}





void CResetPerf::output()
{
	
	CCGISession* cs;	
	std::string result;
	
    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 

	int ret = ResetPerf();
	
	if(ret == 0) {
		if(outputHTMLFile(m_desthtml, true) != 0)
		{
			outputError("Cann't open card.html\n");
		}	
	} 
	else 
		outputError(multilang::getError(ret));
	
	/* result = multilang::getError(ret);

	outputError(result);*/


	free(cs);

}



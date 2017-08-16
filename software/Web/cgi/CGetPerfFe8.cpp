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
#include "CGetPerfFe8.h"

using namespace std;

int CGetPerfFe8::getCardFe8PerfInfo()//for all  etherport perf
{

		std::string card_no = getReqValue("cardno");
		std::string port_no = getReqValue("portno");
		std::string onu_no = getReqValue("onuno");
		
		char send_buff[MAX_CMDBUFF_SIZE];
		char recv_buff[MAX_CMDBUFF_SIZE];
		MSG_HEAD_CMD *msg_header;
		MSG_HEAD_CMD_RSP *msgrsp_header;
		FE_PORTINDEX *feportindex;
		PORT_PERF *portperf;

		msg_header = (MSG_HEAD_CMD *)send_buff;
		feportindex = (FE_PORTINDEX *)(send_buff+sizeof(MSG_HEAD_CMD));
		memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
		
		
		msg_header->msg_code = GET_PORT_PERF;
		msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());	
		msg_header->node_id = (unsigned char) std::atoi(onu_no.c_str());
		msg_header->length = sizeof(FE_PORTINDEX);

		char buff[32];
		m_desthtml = "fe8_perf.html";
		feportindex->portIndex = (unsigned char) std::atoi(port_no.c_str());
		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		if(msgrsp_header->length != sizeof(PORT_PERF))
			return(ERROR_COMM_PARSE);
		portperf = (PORT_PERF *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));
		
		
		sprintf(buff,"%d",(portperf->perfRxByteHi)+(portperf->perfRxByteLo));
		addUserVariable("port_recv", buff);
		sprintf(buff,"%d",(portperf->perfTxByteHi)+(portperf->perfTxByteLo));
		addUserVariable("port_send", buff);
	
	return 0;

}

void CGetPerfFe8::output()
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

	int ret = getCardFe8PerfInfo();
	
	if(ret == 0) {
		if(outputHTMLFile(m_desthtml, true) != 0)
		{
			outputError("Cann't open card.html\n");
		}	
	} 
	else 
		outputError(multilang::getError(ret));

	free(cs);

}
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
#include "const.h"
#include "CPerf.h"

using namespace std;

int CGetPerf::getCardPerfInfo()//for all  etherport perf
{

		std::string card_no = getReqValue("cardno");
		std::string port_no = getReqValue("portno");
		std::string onu_no = getReqValue("onuno");
		std::string action = getReqValue("action");
		std::string flag = getReqValue ("flag");
			
		char send_buff[MAX_CMDBUFF_SIZE];
		char recv_buff[MAX_CMDBUFF_SIZE];
		char buff[256];
		MSG_HEAD_CMD *msg_header;
		MSG_HEAD_CMD_RSP *msgrsp_header;
		FE_PORTINDEX *feportindex;
		PORT_PERF *portperf;

		msg_header = (MSG_HEAD_CMD *)send_buff;
		feportindex = (FE_PORTINDEX *)(send_buff+sizeof(MSG_HEAD_CMD));
		memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
		
		if ( action == "reset")
			{ msg_header->msg_code = SET_PERF_RESET; }
		else 
			{ msg_header->msg_code = GET_PORT_PERF;   }
			
		
		msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());	
		msg_header->node_id = (unsigned char) std::atoi(onu_no.c_str());
		msg_header->length = sizeof(FE_PORTINDEX);
	
		addUserVariable("head_name", multilang::getPerfhead(msg_header->card_no, msg_header->node_id) );
		
				
				feportindex->portIndex = (unsigned char) std::atoi(port_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
			
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
		
				
				
				if ( action == "reset")
				{
					//std::string s = "0";
					//addUserVariable("port_recv", s );
					//addUserVariable("port_send", s );
					sprintf(buff, "sessionID=%s&cardno=%s&onuno=%s&portno=%s", getReqValue("sessionID").c_str(), getReqValue("cardno").c_str(), getReqValue("onuno").c_str() ,getReqValue("portno").c_str());
					redirectHTMLFile("/cgi-bin/perfview.cgi", buff);
					return (-1);
				}
				else {
					if(msgrsp_header->length != sizeof(PORT_PERF))
						return(ERROR_COMM_PARSE);
					portperf = (PORT_PERF *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));
					sprintf(buff,"%u", ntohl(portperf->perfRxByteHi)+ntohl(portperf->perfRxByteLo));
					addUserVariable("port_recv", buff);
					sprintf(buff,"%u", ntohl(portperf->perfTxByteHi)+ntohl(portperf->perfTxByteLo));
					addUserVariable("port_send", buff);
			}
	
	return 0;

}

void CGetPerf::output()
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

	int ret = getCardPerfInfo();
	
	if(ret == 0) {
		if(outputHTMLFile("perf.html", true) != 0)
		{
			outputError("Cann't open perf.html\n");
		}	
	} 
	else if (-1 != ret )
		outputError(multilang::getError(ret));

	free(cs);

}

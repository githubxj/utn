#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sstream> 
#include <vector>
#include <netinet/in.h>

#include "CgiRequest.h"
#include "CGISession.h"
#include "Lang.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"
#include "CGetEPOnu.h"

using namespace std;


const char *CGetEPOnu::getLink(unsigned char type)
{	switch(type)
	{		case 1:			return("Up");
			case 0:			return("Down");
			default:			return("Unknown");
			}
}

const char *CGetEPOnu::getBaudrate(unsigned char type)
{
	switch ( type )
	{
		{
			case 0:
				return ("	<option value='0' selected> 1200</option>\
					<option value='1' > 2400</option>\
					<option value='2' > 4800</option>\
					<option value='3' > 9600</option>\
					<option value='4' > 19200</option>\
					<option value='5' > 38400</option>\
					<option value='6' > 57600</option>\
					<option value='7' > 115200</option>\
					");
			case 1:
				return ("<option value='0' > 1200</option>\
					<option value='1' selected> 2400</option>\
					<option value='2' > 4800</option>\
					<option value='3' > 9600</option>\
					<option value='4' > 19200</option>\
					<option value='5' > 38400</option>\
					<option value='6' > 57600</option>\
					<option value='7' > 115200</option>\
					");
			case 2:
				return ("<option value='0' > 1200</option>\
					<option value='1' > 2400</option>\
					<option value='2' selected> 4800</option>\
					<option value='3' > 9600</option>\
					<option value='4' > 19200</option>\
					<option value='5' > 38400</option>\
					<option value='6' > 57600</option>\
					<option value='7' > 115200</option>\
					");
			case 3:
				return ("<option value='0' > 1200</option>\
					<option value='1' > 2400</option>\
					<option value='2' > 4800</option>\
					<option value='3' selected> 9600</option>\
					<option value='4' > 19200</option>\
					<option value='5' > 38400</option>\
					<option value='6' > 57600</option>\
					<option value='7' > 115200</option>\
					</select>");
			case 4:
				return ("<option value='0' > 1200</option>\
					<option value='1' > 2400</option>\
					<option value='2' > 4800</option>\
					<option value='3' > 9600</option>\
					<option value='4' selected> 19200</option>\
					<option value='5' > 38400</option>\
					<option value='6' > 57600</option>\
					<option value='7' > 115200</option>\
					");
			case 5:
				return ("<option value='0' > 1200</option>\
					<option value='1' > 2400</option>\
					<option value='2' > 4800</option>\
					<option value='3' > 9600</option>\
					<option value='4' > 19200</option>\
					<option value='5' selected> 38400</option>\
					<option value='6' > 57600</option>\
					<option value='7' > 115200</option>\
					");
			case 6:
				return ("<option value='0' > 1200</option>\
					<option value='1' > 2400</option>\
					<option value='2' > 4800</option>\
					<option value='3' > 9600</option>\
					<option value='4' > 19200</option>\
					<option value='5' > 38400</option>\
					<option value='6' selected> 57600</option>\
					<option value='7' > 115200</option>\
					");
			case 7:
				return ("<option value='0' > 1200</option>\
					<option value='1' > 2400</option>\
					<option value='2' > 4800</option>\
					<option value='3' > 9600</option>\
					<option value='4' > 19200</option>\
					<option value='5' > 38400</option>\
					<option value='6' > 57600</option>\
					<option value='7' selected> 115200</option>\
					");
			default:
				return ("<option value='0' > 1200</option>\
					<option value='1' > 2400</option>\
					<option value='2' > 4800</option>\
					<option value='3' > 9600</option>\
					<option value='4' > 19200</option>\
					<option value='5' > 38400</option>\
					<option value='6' > 57600</option>\
					<option value='7' > 115200</option>\
					<option value='8' selected> not-support</option>\
					");
		
			}

	}

}


const char *CGetEPOnu::getPortselect(unsigned char type)
{
	switch(type)
	{
		{
			case 0:
				return ("<option value='0' selected> RS232</option>\
					<option value='1' > RS485</option>");
			case 1:
				return ("<option value='0' > RS232</option>\
					<option value='1' selected> RS485</option>");
			default:
				return ("<option value='0' > RS232</option>\
						<option value='1' > RS485</option>\
						<option value='2' selected>not-support</option>");
		
			}	
	}

}

int CGetEPOnu::getOnuData()
{
	std::string card_no = getReqValue("cardno");
	std::string node_id = getReqValue("onunode");
	std::string flag = getReqValue ("flag");
	std::string pon = getReqValue ("pon");

	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	EPONU_ONU_REQ	*onu_req;
	EPONU_INFO 		*onu_info;
	EPONU_UART 		*uart;
	EPONU_NETWORK	*net;
	EPONU_BW		*bw;
	EPONU_IO		*io;
	stringstream outss;
	char buff[256];	
	char field_name[32];
	
	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	msg_header->length = sizeof(EPONU_ONU_REQ);
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	msg_header->node_id = (unsigned char) std::atoi(node_id.c_str());

	onu_req = (EPONU_ONU_REQ * ) (send_buff + sizeof(MSG_HEAD_CMD));
	onu_req->node_id = msg_header->node_id;
	onu_req->pon = (unsigned char) std::atoi(pon.c_str());
	
	if ( flag == "base")
	{
		msg_header->msg_code = C_EP_GET_ONUSTAT;		
	}
	else if ( flag == "uart" )
	{
		msg_header->msg_code = C_EP_GET_ONUUART;
	}
	else if ( flag == "net" )
	{
		msg_header->msg_code = C_EP_GET_ONUNETWORK;
	}
	else if ( flag == "bw" )
	{
		msg_header->msg_code = C_EP_GET_ONUBW;
	}
	else if ( flag == "io" )
	{
		msg_header->msg_code = C_EP_GET_ONUIO;
	}
	else
	{
		m_desthtml = "eponu.html";
		return 0;
	}

	
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
		
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);
	
	if (flag == "base" )
	{
		if(msgrsp_header->length != sizeof(EPONU_INFO))
			return(ERROR_COMM_PARSE);
		
		onu_info = (EPONU_INFO*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
		sprintf(buff,"%d", onu_info->node_id);
		addUserVariable("node_id", buff);
		memcpy(buff , onu_info->onu_name, MAX_NAME2_LEN );
		buff[MAX_NAME2_LEN] = '\0';
		addUserVariable("onuname", buff);

		sprintf(buff, "%3.1f km", onu_info->distance/10.0);
		addUserVariable("distance", buff);

		sprintf(buff, "%d.%d", onu_info->version_FW[0],  onu_info->version_FW[1]);
		addUserVariable("version_fw", buff);

		if(onu_info->state)
			addUserVariable("state", "Online");
		else
			addUserVariable("state", "Offline");
		
		for(int i=0; i<4; i++)
		{
			sprintf(field_name, "port%d", i+1);
			addUserVariable(field_name, getLink(onu_info->port_state[i]));
		}

		m_desthtml = "eponu_base.html";

	}
	else if ( flag == "uart" )
	{
		if(msgrsp_header->length != sizeof(EPONU_UART))
			return(ERROR_COMM_PARSE);
		
		uart = (EPONU_UART *)(recv_buff +  sizeof(MSG_HEAD_CMD_RSP));

		addUserVariable("baudrate", getBaudrate(uart->baudrate));
		addUserVariable("port_sel", getPortselect(uart->mode));
		sprintf(buff, "%d", uart->tcp_port);
		addUserVariable("tcp_port", buff);
		
		m_desthtml = "eponu_uart.html";
		
	}
	else if ( flag == "bw" )
	{
		if(msgrsp_header->length != sizeof(EPONU_BW))
			return(ERROR_COMM_PARSE);
		
		bw = (EPONU_BW *)(recv_buff +  sizeof(MSG_HEAD_CMD_RSP));

		sprintf(buff, "%d", ntohl(bw->max_bandwidth));
		addUserVariable("max_bw", buff);
		
		if(bw->vlan) 
			addUserVariable("vlan_checked","checked");
		
		m_desthtml = "eponu_bw.html";
		
	}
	else if ( flag == "net" )
	{
		if(msgrsp_header->length != sizeof(EPONU_NETWORK))
			return(ERROR_COMM_PARSE);
		
		net = (EPONU_NETWORK *)(recv_buff +  sizeof(MSG_HEAD_CMD_RSP));

		net->ip[15] = 0;
		addUserVariable("ip", net->ip);
		net->mask[15] = 0;
		addUserVariable("mask", net->mask);
		net->gateway[15] = 0;
		addUserVariable("gateway", net->gateway);
		
		m_desthtml = "eponu_net.html";
		
	}
	else if ( flag == "io" )
	{
		if (msgrsp_header->length != sizeof(EPONU_IO))
			return(ERROR_COMM_PARSE);

		io = (EPONU_IO*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));

		io->as_ip[15] = 0;
		addUserVariable("as_ip", io->as_ip);
		sprintf(buff, "%d", ntohs(io->as_port));
		addUserVariable("as_port", buff);
		sprintf(buff, "%d", ntohs(io->local_port));
		addUserVariable("local_port", buff);
		sprintf(buff, "%d", ntohs(io->dev_id));
		addUserVariable("dev_id", buff);
		
		addUserVariable( "alarm_mode" , multilang::getAlarmMode(io->alarm_mode));
		addUserVariable( "alarm_format" , multilang::getAlarmFormat(io->alarm_format));

		m_desthtml = "eponu_io.html";
	}
	
	return 0;

}

void CGetEPOnu::output()
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

	int ret = getOnuData();
	
	if(ret == 0) {
		if(outputHTMLFile(m_desthtml, true) != 0)
		{  
			outputError("Cann't open eponu.html\n");
		}	
	} 
	else 
		outputError(multilang::getError(ret));

	free(cs);

}


#include <iostream>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

#include "CGISession.h"
#include "Tools.h"
#include "Lang.h"
#include "CgiRequest.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"

#include "CEPOnuChg.h"

using namespace std;

int CEPOnuChg::updataOnuInfo()
{

	std::string card_no = getReqValue("cardno");
	std::string node_id = getReqValue("onunode");
	std::string action = getReqValue("action");
	std::string pon = getReqValue ("pon");

	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;

	EPONU_ONU_REQ	*onu_req;
	EPONU_NAME *onu_set;
	EPONU_NETWORK *net;
	EPONU_UART *uart;
	EPONU_BW *bw;
	EPONU_IO *io;
	EPONU_UPPATH *up_path;
#if 1// 20150417	
	EPONU_IOCTRL *ioctrl;
#endif	
	unsigned char pon_id;
	
	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	msg_header->node_id = (unsigned char) std::atoi(node_id.c_str());
	pon_id = (unsigned char) std::atoi(pon.c_str());

	if(action == "reset")
	{
		msg_header->length = sizeof (EPONU_ONU_REQ);
		onu_req = (EPONU_ONU_REQ *) (send_buff + sizeof(MSG_HEAD_CMD));
		onu_req->pon = pon_id;
		onu_req->node_id = msg_header->node_id;
		msg_header->msg_code = C_EP_RESET_ONU;		
	}
	else if ( action == "base")
	{
		msg_header->msg_code = C_EP_SET_ONUNAME;
		msg_header->length = sizeof (EPONU_NAME);
		onu_set = (EPONU_NAME *) (send_buff + sizeof(MSG_HEAD_CMD));
		
		onu_set->pon = pon_id;
		onu_set->node_id = msg_header->node_id;
		memcpy(onu_set->onu_name, getReqValue("onuname").c_str(), MAX_NAME2_LEN);
	}
	else if ( action == "net")
	{
		msg_header->msg_code = C_EP_SET_ONUNETWORK;
		msg_header->length = sizeof(EPONU_NETWORK);
		net = (EPONU_NETWORK *)(send_buff + sizeof(MSG_HEAD_CMD));

		memset(net, 0, sizeof(EPONU_NETWORK));
		net->pon = pon_id;
		net->node_id = msg_header->node_id;
		strncpy(net->ip, getReqValue("ip").c_str(), 15);
		strncpy(net->mask, getReqValue("mask").c_str(), 15);
		strncpy(net->gateway, getReqValue("gateway").c_str(), 15);		
	}
	else if ( action == "uart")
	{
		msg_header->msg_code = C_EP_SET_ONUUART;
		msg_header->length = sizeof(EPONU_UART);
		uart = (EPONU_UART *)(send_buff + sizeof(MSG_HEAD_CMD));
		
		uart->pon = pon_id;
		uart->node_id = msg_header->node_id;
		uart->tcp_port = htons(std::atoi(getReqValue("tcp_port").c_str()));
		uart->baudrate = (unsigned char )std::atoi(getReqValue("baudrate").c_str());
	}
	else if ( action == "bw")
	{
		msg_header->msg_code = C_EP_SET_ONUBW;
		msg_header->length = sizeof(EPONU_BW);
		bw = (EPONU_BW *)(send_buff + sizeof(MSG_HEAD_CMD));

		bw->pon = pon_id;
		bw->node_id = msg_header->node_id;
		bw->max_bandwidth = htonl(std::atoi(getReqValue("max_bw").c_str()));
		if( "on" == getReqValue("vlan"))
			bw->vlan = 1;
		else
			bw->vlan = 0;
	}
	else if ( action == "io")
	{
		msg_header->msg_code = C_EP_SET_ONUIO;
		msg_header->length = sizeof(EPONU_IO);
		io = (EPONU_IO *)(send_buff + sizeof(MSG_HEAD_CMD));

		memset(io, 0, sizeof(EPONU_IO));
		io->pon = pon_id;
		io->node_id = msg_header->node_id;
		strncpy(io->as_ip, getReqValue("as_ip").c_str(), 15);
		io->as_port = htons(std::atoi(getReqValue("as_port").c_str()));
		io->local_port = htons(std::atoi(getReqValue("local_port").c_str()));
		io->dev_id = htons(std::atoi(getReqValue("dev_id").c_str()));
		io->alarm_mode = (unsigned char )std::atoi(getReqValue("alarm_mode").c_str());
		io->alarm_format = (unsigned char )std::atoi(getReqValue("alarm_format").c_str());
	}
	else if ( action == "up")
	{
		msg_header->msg_code = C_EP_SET_ONU_UPPATH;
		msg_header->length = sizeof(EPONU_UPPATH);
		up_path = (EPONU_UPPATH *)(send_buff + sizeof(MSG_HEAD_CMD));

		memset(up_path, 0, sizeof(EPONU_UPPATH));
		up_path->pon = pon_id;
		up_path->node_id = msg_header->node_id;
		strncpy(up_path->path, getReqValue("path").c_str(), MAX_NAME2_LEN);
	}
	#if 1 // 20150417
	else if (action == "ioctrl") // io¿ØÖÆÊä³ö
	{
		msg_header->msg_code = C_EP_SET_ONUIOCTRL;
		msg_header->length = sizeof(EPONU_IOCTRL);
		ioctrl= (EPONU_IOCTRL *)(send_buff + sizeof(MSG_HEAD_CMD));
		memset(ioctrl, 0, sizeof(EPONU_IOCTRL));
        ioctrl->pon = pon_id;
        ioctrl->node_id = msg_header->node_id;
        ioctrl->iono = (unsigned char )std::atoi(getReqValue("iono").c_str());;
        ioctrl->iocmd = (unsigned char )std::atoi(getReqValue("ioctrl_cmd").c_str());;
	}
	#endif
	
	if(comm::send_cmd(send_buff, recv_buff) != 0)  
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);
			
	return ERROR_NO;
	
	
}


void CEPOnuChg::output()
{
	CCGISession* cs;
	std::string flag = getReqValue("flag");
	int ret;

    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	}

	ret = updataOnuInfo();

	outputError(multilang::getError(ret));
	
	free(cs);

}



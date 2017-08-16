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
#include "CRefreshCheck.h"

using namespace std;

void CRefreshCheck::output()
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	RERESH_CHECK_REQ *req;
	int ret;

	std::string type = getReqValue("type");
	std::string card_no = getReqValue("cardno");

	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
		
	msg_header->msg_code = REFRESH_CHECK;
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());	
	msg_header->node_id = 0;
	msg_header->length = sizeof(RERESH_CHECK_REQ);

	req = (RERESH_CHECK_REQ *)(send_buff+sizeof(MSG_HEAD_CMD));
	req->sessionID = std::atol(getReqValue("sessionID").substr(0,9).c_str());
	if(type == "tree")
		req->type = 1;
	else if(type == "frame")
		req->type = 2;
	else if(type == "topo")
		req->type = 3;
	else
		req->type = 0;

	if((req->sessionID == 0) ||req->type == 0)
	{
		ret = 0;
	}
	else if(comm::send_cmd(send_buff, recv_buff) == 0) 
	{	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		ret = msgrsp_header->rsp_code;
	}
	
	std::cout << "Content-type:text/html\r\n\r\n";
	std::cout << ret << std::endl;

	return;

}

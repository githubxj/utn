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

#include "COnuChg.h"

using namespace std;

int COnuChg::updataOnuInfo()
{

	std::string card_no = getReqValue("cardno");
	std::string node_id = getReqValue("onunode");
	std::string action = getReqValue("action");
	std::string portno = getReqValue ("portno");

	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	ONU_SET *onu_set;
	ETHER_PORT_SET *port_set;
	FE_PORTINDEX *portIndex;
	int bw_send, bw_recv;
	std::string mac;
	ONU_UART_CONF *uart;
	ONU_PORTMIRROR_CONF *portMirror;
	ONU_QoS_CONF *qosSet;
	unsigned char temp;
	std::string tmp;

	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	msg_header->node_id = (unsigned char) std::atoi(node_id.c_str());

	if(action == "reset")
	{

		msg_header->msg_code = C_ONU_RESET;
		
	}
	else if ( action == "setbase")
	{
		msg_header->msg_code = C_ONU_SET;
		msg_header->length = sizeof (ONU_SET);
		onu_set = (ONU_SET *) (send_buff + sizeof(MSG_HEAD_CMD));
		
		onu_set->setFlag = 0xFF;
		memcpy(onu_set->onu_name, getReqValue("onuname").c_str(), MAX_DISPLAYNAME_LEN);
		onu_set->admin_state = (unsigned char)std::atoi(getReqValue("admin_state").c_str());
		onu_set->broadcaststorm_ctrl = (unsigned char )std::atoi(getReqValue("storm_ctrl").c_str());
		onu_set->storm_threshold = htons((unsigned short )std::atoi(getReqValue("storm_threshold").c_str()));
	}
	else if ( action == "port" )
	{
		msg_header->msg_code = C_ETHER_SET;
		msg_header->length = sizeof(ETHER_PORT_SET) + sizeof (FE_PORTINDEX);
		port_set = (ETHER_PORT_SET *)(send_buff + sizeof(MSG_HEAD_CMD));
		portIndex = (FE_PORTINDEX *)(send_buff + sizeof(MSG_HEAD_CMD) + sizeof (ETHER_PORT_SET));
		portIndex->portIndex = (unsigned char )std::atoi(portno.c_str());
		port_set->setFlag = 0xFF;
		
		
		port_set->admin_state = (unsigned char)std::atoi(getReqValue("fe_admin_state").c_str());
		
		bw_send = std::atoi(getReqValue("fe_bw_alloc_send").c_str());
		port_set->bw_alloc_send = bw_send/32;
		port_set->bw_alloc_send = htons(port_set->bw_alloc_send);
		
		bw_recv = std::atoi(getReqValue("fe_bw_alloc_recv").c_str());
		port_set->bw_alloc_recv= bw_recv/32;
		port_set->bw_alloc_recv= htons(port_set->bw_alloc_recv);
		
		port_set->portMode = (unsigned char)std::atoi(getReqValue("fe_port_mode").c_str());
		port_set->portWorkingMode = (unsigned char )std::atoi(getReqValue("fe_working_mode").c_str() );

		mac = getReqValue("fe_bounded_mac");
		cgicc::getMac(mac, port_set->bounded_mac);

	}
	else if ( action == "datach")
	{
		unsigned char tmp;
		msg_header->msg_code = C_ONU_SETDATACH;
		msg_header->length = sizeof(ONU_UART_CONF);
		uart = (ONU_UART_CONF *)(send_buff + sizeof(MSG_HEAD_CMD));
		uart->setFlag = 0xFF;
		uart->port = (unsigned char )std::atoi(getReqValue("port").c_str());
		uart->baudrate = (unsigned char )std::atoi(getReqValue("baudrate").c_str());
		uart->time = (unsigned char )std::atoi(getReqValue("data_buff").c_str());
		uart->function = (unsigned char )std::atoi(getReqValue("port_sel").c_str());
/*		uart->control = (unsigned char)std::atoi(getReqValue("databit").c_str());
		tmp = (unsigned char)std::atoi(getReqValue("stopbit").c_str());
		uart->control |=((tmp&0x01)<<2);
		tmp = (unsigned char)std::atoi(getReqValue("parity").c_str());
		uart->control |=((tmp&0x07)<<3);
*/
	}
	else if ( action == "io")
	{
		ONU_IO * io_set;
		msg_header->msg_code = C_ONU_SETIO;
		msg_header->length = sizeof(ONU_IO);
		io_set = (ONU_IO *)(send_buff + sizeof(MSG_HEAD_CMD));
		io_set->out_select = (unsigned char )std::atoi(getReqValue("out_sel").c_str());
		io_set->out_value = (unsigned char )std::atoi(getReqValue("out_value").c_str());

		//change the select value to bit mask
		io_set->out_value =  io_set->out_value << io_set->out_select;
		io_set->out_select = 1 << io_set->out_select;
	}
	else if ( action == "portmirror")
	{
		msg_header->msg_code = C_ONU_SETPORTMIRROR;
		msg_header->length = sizeof(ONU_PORTMIRROR_CONF);

		portMirror = (ONU_PORTMIRROR_CONF *)(send_buff + sizeof(MSG_HEAD_CMD));
		portMirror->setFlag = 0xFF;
		
		tmp = getReqValue("port1Mirror");
		if ( tmp != "")
			portMirror->port1[0] = 0x01;
		else
			portMirror->port1[0] = 0x00;
		tmp = getReqValue("port1Rx");
		if ( tmp != "")
			portMirror->port1[1] = 0x01;
		else
			portMirror->port1[1] = 0x00;
		tmp = getReqValue("port1Tx");
		if ( tmp == "on")
			portMirror->port1[2] = 0x01;
		else
			portMirror->port1[2] = 0x00;

		tmp = getReqValue("port2Mirror");
		if ( tmp == "on")
			portMirror->port2[0] = 0x01;
		else
			portMirror->port2[0] = 0x00;
		tmp = getReqValue("port2Rx");
		if ( tmp == "on")
			portMirror->port2[1] = 0x01;
		else
			portMirror->port2[1] = 0x00;
		tmp = getReqValue("port2Tx");
		if ( tmp == "on")
			portMirror->port2[2] = 0x01;
		else
			portMirror->port2[2] = 0x00;

		tmp = getReqValue("port3Mirror");
		if ( tmp == "on")
			portMirror->port3[0] = 0x01;
		else
			portMirror->port3[0] = 0x00;
		tmp = getReqValue("port3Rx");
		if ( tmp == "on")
			portMirror->port3[1] = 0x01;
		else
			portMirror->port3[1] = 0x00;
		tmp = getReqValue("port3Tx");
		if ( tmp == "on")
			portMirror->port3[2] = 0x01;
		else
			portMirror->port3[2] = 0x00;

		tmp = getReqValue("port4Mirror");
		if ( tmp == "on")
			portMirror->port4[0] = 0x01;
		else
			portMirror->port4[0] = 0x00;
		tmp = getReqValue("port4Rx");
		if ( tmp == "on")
			portMirror->port4[1] = 0x01;
		else
			portMirror->port4[1] = 0x00;
		tmp = getReqValue("port4Tx");
		if ( tmp == "on")
			portMirror->port4[2] = 0x01;
		else
			portMirror->port4[2] = 0x00;

		tmp = getReqValue("port5Mirror");
		if ( tmp == "on")
			portMirror->port5[0] = 0x01;
		else
			portMirror->port5[0] = 0x00;
		tmp = getReqValue("port5Rx");
		if ( tmp == "on")
			portMirror->port5[1] = 0x01;
		else
			portMirror->port5[1] = 0x00;
		tmp = getReqValue("port5Tx");
		if ( tmp == "on")
			portMirror->port5[2] = 0x01;
		else
			portMirror->port5[2] = 0x00;

		tmp = getReqValue("badframes");
		if ( tmp == "on")
			portMirror->badframes = 0x01;
		else
			portMirror->badframes = 0x00;
		tmp = getReqValue("sourceanddestination");
		if ( tmp == "on")
			portMirror->sourceanddestination = 0x01;
		else
			portMirror->sourceanddestination = 0x00;

		/*tmp = getReqValue("port1Mirror");
		if (strcmp(tmp.c_str(), "1")==0)
			portMirror->port1[0] = 1;
		else
			portMirror->port1[0] = 0;*/
		
		//portMirror->port1[0] = (unsigned char )std::atoi(getReqValue("port1Mirror").c_str());
		/*portMirror->port1[1] = (unsigned char )std::atoi(getReqValue("port1Rx").c_str());
		portMirror->port1[2] = (unsigned char )std::atoi(getReqValue("port1Tx").c_str());
		portMirror->port2[0] = (unsigned char )std::atoi(getReqValue("port2Mirror").c_str());
		portMirror->port2[1] = (unsigned char )std::atoi(getReqValue("port2Rx").c_str());
		portMirror->port2[2] = (unsigned char )std::atoi(getReqValue("port2Tx").c_str());
		portMirror->port3[0] = (unsigned char )std::atoi(getReqValue("port3Mirror").c_str());
		portMirror->port3[1] = (unsigned char )std::atoi(getReqValue("port3Rx").c_str());
		portMirror->port3[2] = (unsigned char )std::atoi(getReqValue("port3Tx").c_str());
		portMirror->port4[0] = (unsigned char )std::atoi(getReqValue("port4Mirror").c_str());
		portMirror->port4[1] = (unsigned char )std::atoi(getReqValue("port4Rx").c_str());
		portMirror->port4[2] = (unsigned char )std::atoi(getReqValue("port4Tx").c_str());
		portMirror->port5[0] = (unsigned char )std::atoi(getReqValue("port5Mirror").c_str());
		portMirror->port5[1] = (unsigned char )std::atoi(getReqValue("port5Rx").c_str());
		portMirror->port5[2] = (unsigned char )std::atoi(getReqValue("port5Tx").c_str());
		portMirror->badframes = (unsigned char )std::atoi(getReqValue("badframes").c_str());
		portMirror->sourceanddestination = (unsigned char )std::atoi(getReqValue("sourceanddestination").c_str());*/
	}
	else if ( action == "qos")
	{
		msg_header->msg_code = C_ONU_SETQoS;
		msg_header->length = sizeof (ONU_QoS_CONF);
		qosSet = (ONU_QoS_CONF *) (send_buff + sizeof(MSG_HEAD_CMD));
		
		qosSet->setFlag = 0xFF;
		qosSet->priorityEnable[0] = (unsigned char )std::atoi(getReqValue("port1priority_enable").c_str());
		qosSet->priorityEnable[1] = (unsigned char )std::atoi(getReqValue("port2priority_enable").c_str());
		qosSet->priorityEnable[2] = (unsigned char )std::atoi(getReqValue("port3priority_enable").c_str());
		qosSet->priorityEnable[3] = (unsigned char )std::atoi(getReqValue("port4priority_enable").c_str());
		qosSet->priorityEnable[4] = (unsigned char )std::atoi(getReqValue("port5priority_enable").c_str());
		qosSet->prioritySchemeSelect= (unsigned char )std::atoi(getReqValue("priorityScheme_select").c_str());
	}
	
	if(comm::send_cmd(send_buff, recv_buff) != 0)  
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);
			
	return ERROR_NO;
	
	
}

int COnuChg::Addvlan()
{
	std::string fid = getReqValue("fid");
	std::string cardno = getReqValue("cardno");
	std::string onunode = getReqValue("onunode");

	addUserVariable("filter_id",  fid);
	addUserVariable("card_no",  cardno);
	addUserVariable("onu_node",  onunode);
	m_desthtml = "onuvlan.html";
	return 0;
}


int COnuChg::Modifyvlan()
{
	
	std::string vid = getReqValue("vid");
	std::string fid = getReqValue("fid");
	std::string bitmap = getReqValue("bitmap");
	std::string cardno = getReqValue("cardno");
	std::string onunode = getReqValue("onunode");
	unsigned char portmap = (unsigned char)std::atoi(bitmap.c_str());
	unsigned char i , mask;
	char buff[10];

	memset(buff, 0, sizeof(buff));
	mask = 0x01;
	for (i = 0; i < 5; i++)
	{
		
		if ( mask == ( portmap & mask ))
		{
			sprintf(buff, "port%d", i+1);
			
			addUserVariable(buff , "checked");
		}

		mask = mask << 1;
		
	}
	
	addUserVariable("vlan_id",  vid);
	addUserVariable("filter_id",  fid);
	addUserVariable("card_no",  cardno);
	addUserVariable("onu_node",  onunode);
	m_desthtml = "onuvlan.html";
	
	return 0;
}


int COnuChg::Savevlan()
{
	std::string card_no = getReqValue("cardno");
	std::string node_id = getReqValue("onunode");
	std::string vid = getReqValue("vlan_id");
	std::string fid = getReqValue("filter_id");
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	char buff[128];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	std::string temp;
	unsigned char map = 0 , i;
	ONU_VLAN_CONF *vlan;

	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	msg_header->node_id = (unsigned char) std::atoi(node_id.c_str());

	msg_header->msg_code = C_ONU_SAVEVLAN;
	msg_header->length = sizeof(ONU_VLAN_CONF);
	vlan = (ONU_VLAN_CONF *)(send_buff + sizeof(MSG_HEAD_CMD));

	vlan->setFlag = 0xFF;
	vlan->valid= 0x01;
	memset(buff, 0, sizeof(buff));
	//add bitmap
	for ( i = 0 ; i < 5; i++ )
	{
		sprintf(buff, "port%d" , i + 1);
		temp = getReqValue(buff);
		if ( temp == "on")
		{
			map |= 0x01 << i; 
		}
	}
	vlan->membership = map;
	vlan->id= (unsigned char)std::atoi(fid.c_str());
	vlan->vlan_id = (unsigned short)std::atoi(vid.c_str());
	
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	sprintf(buff, "sessionID=%s&cardno=%s&onunode=%s&flag=initonu", getReqValue("sessionID").c_str(),card_no.c_str(),node_id.c_str());
	//sprintf(buff, "sessionID=%s&cardno=%s&onunode=%s&flag=initonu", getReqValue("sessionID").c_str(),getReqValue("cardno").c_str,getReqValue("onunode").c_str);
	redirectHTMLFile("/cgi-bin/show_onu.cgi", buff);
	return 0;
}


int COnuChg::Delvlan()
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	char buff[128];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	std::string cardno = getReqValue("cardno");
	std::string onunode = getReqValue("onunode");
	std::string vid = getReqValue("vid");
	std::string fid = getReqValue("fid");
	std::string bitmap = getReqValue("bitmap");
	ONU_VLAN_CONF *vlan;

	msg_header = (MSG_HEAD_CMD *)send_buff;
	vlan = (ONU_VLAN_CONF *)(send_buff +  sizeof(MSG_HEAD_CMD ));
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	memset(buff, 0, sizeof(buff));

	msg_header->msg_code = C_ONU_DELVLAN;
	msg_header->card_no = (unsigned char) std::atoi(cardno.c_str());
	msg_header->node_id = (unsigned char) std::atoi(onunode.c_str());
	msg_header->length = sizeof(ONU_VLAN_CONF);

	vlan->setFlag = 0xFF;
	vlan->valid = 0x00;
	vlan->membership = (unsigned char)std::atoi(bitmap.c_str());
	vlan->id= (unsigned char)std::atoi(fid.c_str());
	vlan->vlan_id = (unsigned short)std::atoi(vid.c_str());
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	sprintf(buff, "sessionID=%s&cardno=%s&onunode=%s&flag=initonu", getReqValue("sessionID").c_str(),cardno.c_str(),onunode.c_str());
	redirectHTMLFile("/cgi-bin/show_onu.cgi", buff);
	return 0;

}


void COnuChg::output()
{
	std::string result;
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

	if(flag == "add")
	{
		ret = Addvlan();
		if ( 0 == ret )
		{
			//output with the HTML template
			if( outputHTMLFile(m_desthtml, true) != 0)
			{
				outputError("Cann't open onuvlan.html\n");
			}
		}
		else
		{
			outputError(multilang::getError(ret));

		}
	}
	else if ( flag == "modify" )
	{
		ret = Modifyvlan();
		if ( 0 == ret )
		{
			//output with the HTML template
			if( outputHTMLFile(m_desthtml, true) != 0)
			{
				outputError("Cann't open onuvlan.html\n");
			}
		}
		else
		{
			outputError(multilang::getError(ret));

		}
		
	}
	else if ( flag == "save")
	{
		ret = Savevlan();//success redirect
		if ( 0 != ret )
		{
			outputError(multilang::getError(ret));
		}
		
	}
	else if ( flag == "delete" )
	{
		ret = Delvlan();//success redirect
		if ( 0 != ret )
		{
			outputError(multilang::getError(ret));
		}
	}
	else
	{
		int ret = updataOnuInfo();
		result = multilang::getError(ret);

		outputError(result);
	}
	
	free(cs);

}



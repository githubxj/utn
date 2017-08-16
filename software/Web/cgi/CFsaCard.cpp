#include <iostream>
#include <sstream> 
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sstream> 
#include <vector>
#include <syslog.h>
#include <netinet/in.h>

#include "CgiRequest.h"
#include "CGISession.h"
#include "Lang.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"
#include "logger.h"
#include "CFsaCard.h"
#include "CppSQLite3.h"

using namespace std;

const char *CFsaCard::getLink(unsigned char type)
{	switch(type)
	{		
		case 1:	
			return("Up");
		case 2:	
		case 0:	
			return ("Down");
		default:
			return("Unknown");
	}
}

const char *CFsaCard::getWorkingModefsa(unsigned char type, int portno)
{
	switch(portno)
	{
		case 1:
			switch(type)
				{

					case 1:
							return("<select  class='fixsize'  name='ge1_working_mode' id='ge1_working_mode'> \
								      <option value='1' selected> Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize'  name='ge1_working_mode' id='ge1_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' selected> 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 3:
							return("<select  class='fixsize'  name='ge1_working_mode' id='ge1_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' selected> 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 4:
							return("<select class='fixsize'  name='ge1_working_mode' id='ge1_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' selected> 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 5:
							return("<select class='fixsize'  name='ge1_working_mode' id='ge1_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' selected> 10M-Half</option>\
								      </select>");			
		
						default:
							return("<select class='fixsize'  name='ge1_working_mode' id='ge1_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      <option value='6' selected> not-supported</option></select>");	
		}
		case 2:
			switch(type)
				{

					case 1:
							return("<select class='fixsize'  name='ge2_working_mode' id='ge2_working_mode'> \
								      <option value='1' selected> Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize'  name='ge2_working_mode' id='ge2_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' selected> 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 3:
							return("<select class='fixsize'  name='ge2_working_mode' id='ge2_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' selected> 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 4:
							return("<select class='fixsize'  name='ge2_working_mode' id='ge2_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' selected> 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 5:
							return("<select class='fixsize'  name='ge2_working_mode' id='ge2_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' selected> 10M-Half</option>\
								      </select>");			
		
						default:
							return("<select class='fixsize'  name='ge2_working_mode' id='ge2_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      <option value='6' selected> not-supported</option></select>");	
		}
	}

}


const char *CFsaCard::getModeFSA(unsigned char type, int portno)
{

	switch(portno){
	case 1:
		
		switch(type) {
			case 1:
				return ("<select class='fixsize' id='ge1_port_mode' name='ge1_port_mode'  onchange='onWorkingModeChg(\"ge1_port_mode\");'>\
					<option value='1' selected> learning mode</option>\	
					<option value='2' > bounded MAC</option>\
					</select>");
			case 2:
				return ("<select class='fixsize'  id='ge1_port_mode' name='ge1_port_mode'  onchange='onWorkingModeChg(\"ge1_port_mode\");'>\
					<option value='1' > learning mode</option>\	
					<option value='2' selected> bounded MAC</option>\
					</select>");
			default:
				return ("<select class='fixsize' id='ge1_port_mode' name='ge1_port_mode' onchange='onWorkingModeChg(\"ge1_port_mode\");'>\
					<option value='1'>learning mode</option>\
					<option value='2'>bounded MAC</option>\
					<option value ='3' selected >Not-Support</option>\
					</select>");
			
			}

	case 2:
		
		switch(type) {
			case 1:
				return ("<select class='fixsize'  id='ge2_port_mode' name='ge2_port_mode' onchange='onWorkingModeChg(\"ge2_port_mode\");'>\
					<option value='1' selected> learning mode</option>\	
					<option value='2' > bounded MAC</option>\
					</select>");
			case 2:
				return ("<select class='fixsize'  id='ge2_port_mode' name='ge2_port_mode' onchange='onWorkingModeChg(\"ge2_port_mode\");'>\
					<option value='1' > learning mode</option>\	
					<option value='2' selected> bounded MAC</option>\
					</select>");
			default:
				return ("<select class='fixsize' id='ge2_port_mode' name='ge2_port_mode' onchange='onWorkingModeChg(\"ge2_port_mode\");'>\
					<option value='1'>learning mode</option>\
					<option value='2'>bounded MAC</option>\
					<option value ='3' selected >Not-Support</option>\
					</select>");
		
			}	
		}
}

int CFsaCard::procFsaReq()
{
	std::string card_no = getReqValue("cardno");
	std::string flag = getReqValue("flag");
	std::string sfpno = getReqValue ( "sfpIndex");
	std::string portno = getReqValue ("portno");
	
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	FSA_CARD_STATE *cardfsa_info;
	FSA_NETWORK *fsa_net;
	CARD_SET_FSA *cardfsa_set;
	FSA_LOWSPD_CONFIG *fsa_ls_config;
	FSA_LOWSPD_STATE *fsa_ls_stat;
	
	char buff[256];
	std::string gemac1,gemac2;
	std::string temp;
	int i;

	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

//	logger(LOG_ERR, "Process Fsa req: %s\n", flag.c_str());

	if ( flag == "base")
	{
		msg_header->msg_code = C_FSA_GET;

		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		if(msgrsp_header->length != sizeof(FSA_CARD_STATE))
			return(ERROR_COMM_PARSE);

		cardfsa_info = (FSA_CARD_STATE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
					
		if ( cardfsa_info->spf_n_state == 1)//remove
			addUserVariable ("sfp_n_state", multilang::getSfpState(cardfsa_info->spf_n_state));
		else
		{
			sprintf(buff, "%s%s",  multilang::getSfpState(cardfsa_info->spf_n_state), 
					"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='/cgi-bin/sfpinfo.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=0&sfpno=1'><IMG src='/images/sfp_info.gif'></a>");
					addUserVariable ("sfp_n_state", buff);
		}
				
		if ( cardfsa_info->spf_f_state == 1)//remove
			addUserVariable ("sfp_f_state", multilang::getSfpState(cardfsa_info->spf_f_state));
		else
		{
			sprintf(buff, "%s%s",  multilang::getSfpState(cardfsa_info->spf_f_state), 
					"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='/cgi-bin/sfpinfo.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=0&sfpno=2'><IMG src='/images/sfp_info.gif'></a>");
					addUserVariable ("sfp_f_state", buff);
		}
				
		if(cardfsa_info->network_state == 1)
			addUserVariable("network_state", "Ring");
		else
			addUserVariable("network_state", "Links");


		addUserVariable( "statistic_type" , multilang::getstatistictype( cardfsa_info->statistic_type ));
		addUserVariable("storm_ctrl", multilang::getstormstate(cardfsa_info->broadcaststorm_ctrl));
		addUserVariable("igmp_snooping", multilang::getsnoopingstate(cardfsa_info->igmp_snooping&0x03) );
		sprintf(buff, "%d", cardfsa_info->storm_threshold);
		addUserVariable("storm_threshold", buff);

		if(cardfsa_info->igmp_snooping&0x80)
		{
			//process port group clock delay
			if(cardfsa_info->igmp_snooping&0x40)
				addUserVariable("cdg3" , "checked");
			if(cardfsa_info->igmp_snooping&0x20)
				addUserVariable("cdg2" , "checked");
			if(cardfsa_info->igmp_snooping&0x10)
				addUserVariable("cdg1" , "checked");
		}
			
		addAdminStateVariable("ge1_admin_state", cardfsa_info->geport_state[0].admin_state);
		addUserVariable("ge1_port_state", getLink(cardfsa_info->geport_state[0].port_state));
		//sprintf(buff, "%d", cardfsa_info->geport_state[0].bw_alloc_send);
		//addUserVariable("ge1_bw_alloc_send", buff);
		//sprintf(buff, "%d", cardfsa_info->geport_state[0].bw_alloc_recv);
		//addUserVariable("ge1_bw_alloc_recv", buff);

		//addUserVariable("ge1_port_mode", getModeFSA( cardfsa_info->geport_state[0].portMode, 1));
		//addUserVariable("ge1_working_mode", getWorkingModefsa(cardfsa_info->geport_state[0].portWorkingMode, 1));
		//addUserVariable("ge1_bounded_mac", cgicc::getMACaddr(cardfsa_info->geport_state[0].bounded_mac));
			
		addAdminStateVariable("ge2_admin_state", cardfsa_info->geport_state[1].admin_state);
		addUserVariable("ge2_port_state", getLink(cardfsa_info->geport_state[1].port_state));			
		//sprintf(buff, "%d", cardfsa_info->geport_state[1].bw_alloc_send);
		//addUserVariable("ge2_bw_alloc_send", buff);
		//sprintf(buff, "%d", cardfsa_info->geport_state[1].bw_alloc_recv);
		//addUserVariable("ge2_bw_alloc_recv", buff);

		//addUserVariable("ge2_port_mode", getModeFSA(cardfsa_info->geport_state[1].portMode , 2));
		//addUserVariable("ge2_working_mode", getWorkingModefsa(cardfsa_info->geport_state[1].portWorkingMode , 2));
		//addUserVariable("ge2_bounded_mac", cgicc::getMACaddr(cardfsa_info->geport_state[1].bounded_mac));
		m_desthtml = "fsabase_table.html";
	
	}else if(flag == "dat")
	{
		msg_header->msg_code = C_FSA_GET;

		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		if(msgrsp_header->length != sizeof(FSA_CARD_STATE))
			return(ERROR_COMM_PARSE);

		cardfsa_info = (FSA_CARD_STATE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
					
		if ( cardfsa_info->spf_n_state == 1)//remove
			addUserVariable ("sfp_n_state", multilang::getSfpState(cardfsa_info->spf_n_state));
		else
		{
			sprintf(buff, "%s%s",  multilang::getSfpState(cardfsa_info->spf_n_state), 
					"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='/cgi-bin/sfpinfo.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=0&sfpno=1'><IMG src='/images/sfp_info.gif'></a>");
					addUserVariable ("sfp_n_state", buff);
		}
				
		if ( cardfsa_info->spf_f_state == 1)//remove
			addUserVariable ("sfp_f_state", multilang::getSfpState(cardfsa_info->spf_f_state));
		else
		{
			sprintf(buff, "%s%s",  multilang::getSfpState(cardfsa_info->spf_f_state), 
					"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href='/cgi-bin/sfpinfo.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=0&sfpno=2'><IMG src='/images/sfp_info.gif'></a>");
					addUserVariable ("sfp_f_state", buff);
		}
				
		if(cardfsa_info->network_state == 1)
			addUserVariable("network_state", "Ring");
		else
			addUserVariable("network_state", "Links");

		m_desthtml = "fsadat_table.html";
	
	}else if (flag == "setbase") {
			msg_header->msg_code = C_FSA_SET;
			msg_header->length = sizeof(CARD_SET_FSA);
			cardfsa_set = (CARD_SET_FSA *)(send_buff+sizeof(MSG_HEAD_CMD));
			memcpy(cardfsa_set->card_name, getReqValue("card_name").c_str(), MAX_DISPLAYNAME_LEN);
			cardfsa_set->admin_state = (unsigned char)std::atoi(getReqValue("admin_state").c_str());
			cardfsa_set->setFlag = 0xFF;
			
			cardfsa_set->statistic_type = (unsigned char)std::atoi(getReqValue("statistic_type").c_str());
			cardfsa_set->igmp_snooping = (unsigned char )std::atoi(getReqValue("igmp_snooping").c_str());
			cardfsa_set->broadcaststorm_ctrl = (unsigned char)std::atoi(getReqValue("storm_ctrl").c_str());
			cardfsa_set->storm_threshold = (unsigned char)std::atoi(getReqValue("storm_threshold").c_str());
			
			cardfsa_set->port[0].setFlag = 0xFF;
			cardfsa_set->port[0].admin_state = (unsigned char)std::atoi(getReqValue("ge1_admin_state").c_str());
			//cardfsa_set->port[0].bw_alloc_send = (unsigned short)std::atoi(getReqValue("ge1_bw_alloc_send").c_str());
			//cardfsa_set->port[0].bw_alloc_recv = (unsigned short)std::atoi(getReqValue("ge1_bw_alloc_recv").c_str());
			//cardfsa_set->port[0].portMode = (unsigned char)std::atoi(getReqValue("ge1_port_mode").c_str());

			//gemac1 = getReqValue("ge1_bounded_mac");
			//cgicc::getMac(gemac1 , cardfsa_set->port[0].bounded_mac);
			
			cardfsa_set->port[1].setFlag = 0xFF;
			cardfsa_set->port[1].admin_state = (unsigned char)std::atoi(getReqValue("ge2_admin_state").c_str());
			//cardfsa_set->port[1].bw_alloc_send = (unsigned short)std::atoi(getReqValue("ge2_bw_alloc_send").c_str());
			//cardfsa_set->port[1].bw_alloc_recv = (unsigned short)std::atoi(getReqValue("ge2_bw_alloc_recv").c_str());
			//cardfsa_set->port[1].portMode = (unsigned char)std::atoi(getReqValue("ge2_port_mode").c_str());

			//gemac2 = getReqValue("ge2_bounded_mac");
			//cgicc::getMac(gemac2 , cardfsa_set->port[1].bounded_mac);

			//process port group clock delay
			cardfsa_set->igmp_snooping |= 0x80;
			if(getReqValue("cdg3") == "on")
				cardfsa_set->igmp_snooping |= 0x40;
			if(getReqValue("cdg2") == "on")
				cardfsa_set->igmp_snooping |= 0x20;
			if(getReqValue("cdg1") == "on")
				cardfsa_set->igmp_snooping |= 0x10;
				
			if(comm::send_cmd(send_buff, recv_buff) != 0) 
				return ERROR_COMM_LOCAL;
	
			msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
			if(msgrsp_header->rsp_code != 0) 
				return(msgrsp_header->rsp_code);

			usleep(500000);
			m_desthtml = "";			
			
	}else if ( flag == "net") 	{
		msg_header->msg_code = C_FSA_NET_GET;

		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		if(msgrsp_header->length != sizeof(FSA_NETWORK))
			return(ERROR_COMM_PARSE);

		fsa_net = (FSA_NETWORK*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));

		fsa_net->ip[15] = 0;
		addUserVariable("ip", fsa_net->ip);
		fsa_net->mask[15] = 0;
		addUserVariable("mask", fsa_net->mask);
		fsa_net->gateway[15] = 0;
		addUserVariable("gateway", fsa_net->gateway);
		m_desthtml = "fsanet_table.html";
		
	}else if (flag == "setnet") {
			msg_header->msg_code = C_FSA_NET_SET;
			msg_header->length = sizeof(FSA_NETWORK);
			fsa_net = (FSA_NETWORK *)(send_buff+sizeof(MSG_HEAD_CMD));
			
			memset(fsa_net, 0, sizeof(FSA_NETWORK));
			strncpy(fsa_net->ip, getReqValue("ip").c_str(), 15);
			strncpy(fsa_net->mask, getReqValue("mask").c_str(), 15);
			strncpy(fsa_net->gateway, getReqValue("gateway").c_str(), 15);

			if(comm::send_cmd(send_buff, recv_buff) != 0) 
				return ERROR_COMM_LOCAL;
	
			msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
			if(msgrsp_header->rsp_code != 0) 
				return(msgrsp_header->rsp_code);

			usleep(800000);
			m_desthtml = "";	
		
	}else if ( flag == "ls") 	{
		msg_header->msg_code = C_FSA_LOWSPD_GET;

		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		if(msgrsp_header->length != sizeof(FSA_LOWSPD_CONFIG))
			return(ERROR_COMM_PARSE);

		fsa_ls_config = (FSA_LOWSPD_CONFIG*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));

		sprintf(buff, "%d", ntohs(fsa_ls_config->rs_port));
		addUserVariable("tcp_port", buff);
		sprintf(buff, "%d", ntohs(fsa_ls_config->rs_port_step));
		addUserVariable("tcp_port_step", buff);

		fsa_ls_config->as_ip[15] = 0;
		addUserVariable("as_ip", fsa_ls_config->as_ip);
		sprintf(buff, "%d", ntohs(fsa_ls_config->as_port));
		addUserVariable("as_port", buff);
		sprintf(buff, "%d", ntohs(fsa_ls_config->dev_id));
		addUserVariable("dev_id", buff);
		sprintf(buff, "%d", ntohs(fsa_ls_config->dev_id_step));
		addUserVariable("dev_id_step", buff);

		for(i=0; i<6; i++)
		{
			sprintf(buff, "amask%d", i+1);
			if(fsa_ls_config->alarm_mask[i] == 0)
				addUserVariable(buff , "checked");
		}
		
		addUserVariable( "alarm_mode" , multilang::getAlarmMode(fsa_ls_config->alarm_mode));
		addUserVariable( "alarm_format" , multilang::getAlarmFormat(fsa_ls_config->alarm_format));
		
		fsa_ls_config->as_ip2[15] = 0;
		addUserVariable("as_ip2", fsa_ls_config->as_ip2);
		sprintf(buff, "%d", ntohs(fsa_ls_config->as_port2));
		addUserVariable("as_port2", buff);
		addUserVariable( "alarm_format2" , multilang::getAlarmFormat2(fsa_ls_config->alarm_format2));

		m_desthtml = "fsals_table.html";
		
	}else if (flag == "setls") {
			msg_header->msg_code = C_FSA_LOWSPD_SET;
			msg_header->length = sizeof(FSA_LOWSPD_CONFIG);
			fsa_ls_config = (FSA_LOWSPD_CONFIG *)(send_buff+sizeof(MSG_HEAD_CMD));
			
			memset(fsa_ls_config, 0, sizeof(FSA_LOWSPD_CONFIG));

			fsa_ls_config->rs_port = htons(std::atoi(getReqValue("tcp_port").c_str()));
			fsa_ls_config->rs_port_step = htons(std::atoi(getReqValue("tcp_port_step").c_str()));
			
			strncpy(fsa_ls_config->as_ip, getReqValue("as_ip").c_str(), 15);
			fsa_ls_config->as_port = htons(std::atoi(getReqValue("as_port").c_str()));
			fsa_ls_config->dev_id = htons(std::atoi(getReqValue("dev_id").c_str()));
			fsa_ls_config->dev_id_step = htons(std::atoi(getReqValue("dev_id_step").c_str()));

			for(i=0; i<6; i++)
			{
				sprintf(buff, "amask%d", i+1);
				temp = getReqValue(buff);
				if(temp != "on")
					fsa_ls_config->alarm_mask[i] =htonl(0xFFFFFF);
			}

			fsa_ls_config->alarm_mode = std::atoi(getReqValue("alarm_mode").c_str());
			fsa_ls_config->alarm_format = std::atoi(getReqValue("alarm_format").c_str());
	
			strncpy(fsa_ls_config->as_ip2, getReqValue("as_ip2").c_str(), 15);
			fsa_ls_config->as_port2 = htons(std::atoi(getReqValue("as_port2").c_str()));
			fsa_ls_config->alarm_format2 = std::atoi(getReqValue("alarm_format2").c_str());

			if(comm::send_cmd(send_buff, recv_buff) != 0) 
				return ERROR_COMM_LOCAL;
	
			msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
			if(msgrsp_header->rsp_code != 0) 
				return(msgrsp_header->rsp_code);

			usleep(800000);
			m_desthtml = "";	
	}else if ( flag == "stat") 	{
		CppSQLite3DB db;
		char sql[256];
		int node_id;
		stringstream	outss;	
		std::string type = getReqValue("type");
		int uart_no=2;

		if(type == "dau")
			uart_no=5;
		
		// get broadcast channel 
		msg_header->msg_code = C_FSA_LOWSPD_STAT;
		msg_header->node_id = 0;

		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		if(msgrsp_header->length != sizeof(FSA_LOWSPD_STATE))
			return(ERROR_COMM_PARSE);
		
		fsa_ls_stat = (FSA_LOWSPD_STATE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
		sprintf(buff, "%d", ntohs(fsa_ls_stat->rs_connection[0]));
		addUserVariable("b1_con", buff);
		sprintf(buff, "%d", ntohs(fsa_ls_stat->rs_port[0]));
		addUserVariable("b1_port", buff);
		sprintf(buff, "%d", ntohl(fsa_ls_stat->rs_snd[0]));
		addUserVariable("b1_snd", buff);
		sprintf(buff, "%d", ntohs(fsa_ls_stat->rs_connection[1]));
		addUserVariable("b2_con", buff);
		sprintf(buff, "%d", ntohs(fsa_ls_stat->rs_port[1]));
		addUserVariable("b2_port", buff);
		sprintf(buff, "%d", ntohl(fsa_ls_stat->rs_snd[1]));
		addUserVariable("b2_snd", buff);

		//get ONU data
		try {
			db.open(DYNAMIC_DB);
			sprintf(sql, "select NodeId  from ONUList where CardNo=%d order by  NodeId;", msg_header->card_no);
			CppSQLite3Query q = db.execQuery(sql);

			while (!q.eof())
	      		{
	       		node_id = q.getIntField(0);

				msg_header->node_id = node_id;
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
				{
					q.nextRow();
					continue;
				}
				
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
				{
					q.nextRow();
					continue;
				}

				if(msgrsp_header->length != sizeof(FSA_LOWSPD_STATE))
				{
					q.nextRow();
					continue;
				}
		
				fsa_ls_stat = (FSA_LOWSPD_STATE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));

				outss << "<tr><td class='bar' colspan='7'> </td></tr>";
				for(int i=0; i<uart_no; i++)
				{

					if(type == "dau")
						outss << "<tr><td>DAU-"<< node_id << "-" << i+1 <<"</td>";
					else
					{
#ifdef COVOND
						outss << "<tr><td>MAU-"<< node_id << "-" << i+1 <<"</td>";
#else
						outss << "<tr><td>ONU-"<< node_id <<  "-" << i+1 <<"</td>";
#endif
					}
					outss << "<td>"<< ntohs(fsa_ls_stat->rs_port[i]) << "</td>";
					outss << "<td>"<< ntohs(fsa_ls_stat->rs_connection[i]) << "</td>";
					outss << "<td>"<< ntohl(fsa_ls_stat->rs_snd[i]) << "</td>";
					outss << "<td>"<< ntohl(fsa_ls_stat->rs_rcv[i]) << "</td>";
					outss << "<td>"<< ntohs(fsa_ls_stat->alarm_dev_id[i]) << "</td>";
					outss << "<td>"<< ntohs(fsa_ls_stat->alarm_count[i]) << "</td>";
					outss << "</tr>";
				}
				
				q.nextRow();
			}
		}
		catch (CppSQLite3Exception& e)
		{
			outss << "Database error!<br>";
			outss << e.errorCode() << ":" << e.errorMessage();
		}	
		
		addUserVariable("stat_records", outss.str());
		m_desthtml = "fsastat_table.html";
		
	}else if (flag == "stat_reset") {
			msg_header->msg_code = C_FSA_LOWSPD_STATRESET;

			if(comm::send_cmd(send_buff, recv_buff) != 0) 
				return ERROR_COMM_LOCAL;
	
			msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
			if(msgrsp_header->rsp_code != 0) 
				return(msgrsp_header->rsp_code);

			m_desthtml = "";	
	}
	return 0;

}
void CFsaCard::output()
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

	int ret = procFsaReq();
	
	if(ret == 0) {
		if(m_desthtml == "")
			outputError(multilang::getError(0));
		else  if(outputHTMLFile(m_desthtml, true) != 0)
		{  
			outputError("Cann't open fsa.html\n");
		}
	} 
	else 
		outputError(multilang::getError(ret));

	free(cs);

}



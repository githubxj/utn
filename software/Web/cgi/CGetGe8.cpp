#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sstream> 
#include <vector>
#include <syslog.h>




#include "CgiRequest.h"
#include "CGISession.h"
#include "Lang.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"
#include "CGetGe8.h"
#include "logger.h"

using namespace std;

GE8_NET_STATE *netstate;



const char *CGetGE8::getLink(unsigned char type)
{	switch(type)
	{		case 1:			return("Up");
			case 2:			return("Down");
			default:			return("Unknown");
			}
}

const char *CGetGE8::getWorkingModeGE8(unsigned char type)
{
	switch(type)
				{

					case 0:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
									  <option value='0' selected> 10M-full</option>\
								      <option value='1' > 100m-full</option>\
								      <option value='2' > 1000M-Full</option>\
								      <option value='3' > auto</option>\
								      <option value='4' > 10M-Half</option>\
								      <option value='5' > 100M-Half</option>\
								      </select>");			
						case 1:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
									  <option value='0' > 10M-full</option>\
								      <option value='1' selected> 100m-full</option>\
								      <option value='2' > 1000M-Full</option>\
								      <option value='3' > auto</option>\
								      <option value='4' > 10M-Half</option>\
								      <option value='5' > 100M-Half</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
									  <option value='0' > 10M-full</option>\
								      <option value='1' > 100m-full</option>\
								      <option value='2' selected> 1000M-Full</option>\
								      <option value='3' > auto</option>\
								      <option value='4' > 10M-Half</option>\
								      <option value='5' > 100M-Half</option>\
								      </select>");			
						case 3:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
									  <option value='0' > 10M-full</option>\
								      <option value='1' > 100m-full</option>\
								      <option value='2' > 1000M-Full</option>\
								      <option value='3' selected> auto</option>\
								      <option value='4' > 10M-Half</option>\
								      <option value='5' > 100M-Half</option>\
								      </select>");			
						case 4:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
									  <option value='0' > 10M-full</option>\
								      <option value='1' > 100m-full</option>\
								      <option value='2' > 1000M-Full</option>\
								      <option value='3' > auto</option>\
								      <option value='4' selected> 10M-Half</option>\
								      <option value='5' > 100M-Half</option>\
								      </select>");			
						case 5:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
									  <option value='0' > 10M-full</option>\
								      <option value='1' > 100m-full</option>\
								      <option value='2' > 1000M-Full</option>\
								      <option value='3' > auto</option>\
								      <option value='4' > 10M-Half</option>\
								      <option value='5' selected> 100M-Half</option>\
								      </select>");	
						default:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      <option value='6' selected> not-supported</option></select>");	
		}

}
//add by yp

const char *CGetGE8::getStateGe8(unsigned char type)
{
	switch(type)
				{
					case 0:
						return ("<select class='fixsize'  name='fe_admin_state'  id='fe_admin_state'> \
							<option value='0' selected> Disable</option>\
							<option value='1' > Enable</option>\
							</select>");
					case 1:
						return ("<select class='fixsize'  name='fe_admin_state' id='fe_admin_state'> \
							<option value='0' > Disable</option>\
							<option value='1' selected> Enable</option>\
							</select>");
					default:
						return ("<select class='fixsize'  name='fe_admin_state' id='fe_admin_state'> \
								<option value='1' > Enable</option>\
								<option value='0' >Disable</option>\
								<option value='2' selected>not-support</option>\
								</select>");
				
					}

}

const char *CGetGE8::getModeGE8(unsigned char type)
{

switch(type)
			{
				case 0:
					return ("<select class='fixsize' id='fe_port_mode' name='fe_port_mode' onchange='onWorkingModeChg(\"fe_port_mode\");'> \
						<option value='0' selected> Disable</option>\
						<option value='1' > Enable</option>\
						</select>");
				case 1:
					return ("<select class='fixsize' id='fe_port_mode' name='fe_port_mode' onchange='onWorkingModeChg(\"fe_port_mode\");'> \
						<option value='0' > Disable</option>\
						<option value='1' selected> Enable</option>\
						</select>");
				default:
					return ("<select class='fixsize' id='fe_port_mode' name='fe_port_mode' onchange='onWorkingModeChg(\"fe_port_mode\");'> \
							<option value='1' > Enable</option>\
							<option value='0' > Disable</option>\
							<option value='2' selected>not-support</option>\
							</select>");
			
				}

}

const char *CGetGE8::getBaudrate(unsigned char type)
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


const char *CGetGE8::getprioritySchemeSelect(unsigned char type)
{
	switch ( type )
	{
		{
			case 0x00:
				return ("<select class='fixsize'  name='priorityScheme_select' id='priorityScheme_select'> \
						<option value='0' selected> High priority first</option>\
						<option value='1' > 10/1</option>\
						<option value='16' > 5/1</option>\
						<option value='17' > 2/1</option>\
						</select>");
			case 0x01:
				return ("<select class='fixsize'  name='priorityScheme_select' id='priorityScheme_select'>\
					<option value='0' > High priority first</option>\
					<option value='1' selected> 10/1</option>\
					<option value='16' > 5/1</option>\
						<option value='17' > 2/1</option>\
						</select>");
			case 0x10:
				return ("<select class='fixsize'  name='priorityScheme_select' id='priorityScheme_select'>\
						<option value='0' > High priority first</option>\
						<option value='1' > 10/1</option>\
						<option value='16' selected> 5/1</option>\
						<option value='17' > 2/1</option>\
						</select>");
			case 0x11:
				return ("<select class='fixsize'  name='priorityScheme_select' id='priorityScheme_select'>\
						<option value='0' > High priority first</option>\
					 	<option value='1' > 10/1</option>\
						<option value='16' > 5/1</option>\
						<option value='17' selected> 2/1</option>\
						</select>");
		}
	}
	return ("");
}


const char *CGetGE8::getPortselect(unsigned char type)
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


const char *CGetGE8::getmembers(unsigned char map)
{
	unsigned char i , mask;
	char temp[5];
	char static buff[256];
	
	memset(temp, 0 , sizeof(temp));
	memset(buff, 0, sizeof (buff) );
	mask = 0x01;
	for ( i = 0 ; i < 8; i++ )
	{
		if ( mask == ( map & mask) )
		{
			sprintf(temp, "p%d ", i+1);
			strcat(buff, temp);
		}
		mask = mask << 1;
	}

	return buff;

}


int covert_mac(unsigned char *r_mac,char *t_mac)
{
int i=0,t;
for(i=0;i<6;i++)
{
	t=r_mac[i]>>4;
	if(t>9)
	{
		t=t-9;
		t_mac[2*i]=t+64;
	}else
	{
		t=t+48;
		t_mac[2*i]=t;
	}
	
	t=r_mac[i] & 0x0f;
	if(t>9)
	{
		t=t-9;
		t_mac[2*i+1]=t+64;
	}else
	{
		t=t+48;
		t_mac[2*i+1]=t;
	}
}
}



int CGetGE8::getGe8Data()
{
	std::string card_no = getReqValue("cardno");
	std::string node_id = getReqValue("onunode");
	std::string flag = getReqValue ("flag");
	std::string portno = getReqValue ("portno");
	char mac[12];
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	char buff_port[10],buff_igmp[100],buf_igmp_dat[100],buf_mode_port[100];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	int ret,count_yp=0;
	//ONU_INFO *	onu_info;
	//ONU_UART_CONF *uart;
	//ONU_PORTMIRROR_CONF *portMirror;
	//ONU_QoS_CONF *qos;
	ONU_VLAN_CONF *vlan;
	FE_PORT_STATE *port_info;
	FE_PORT_STATE *portIndex;
	GE_PORT_STATE *portlink,*porstate;
	GE8_STATISTICS *portstatistics;
	GE8_IGMP       *igmp_yp;
	stringstream outss;
	stringstream outadd;
	char buff[256];	
	char field_name[32];
	char id_buff[8];
	std::string temp;
	unsigned char map = 0 , i;
	FILE *fd_yp;
	memset(buf_igmp_dat,' ',100);
	
	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	msg_header->length = 0;
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	//std::string cardno = getReqValue("cardno");
	//std::string onunode = getReqValue("onunode");
	std::string vid = getReqValue("vid");
	std::string fid = getReqValue("fid");
	std::string bitmap = getReqValue("bitmap");
	//msg_header->node_id = (unsigned char) std::atoi(node_id.c_str());
	//logger(LOG_ERR,"flag= %s\n",flag);
	
	if ( flag == "base")
	{
		//msg_header->msg_code = C_ONU_GET;
		
		m_desthtml = "GE8.html";
		
	}
	else if(flag == "add")
	{
		addUserVariable("card_no",  card_no);
		m_desthtml = "ge8vlan.html";
		
	}
	else if(flag == "modify")
	{
			
			unsigned char portmap = (unsigned char)std::atoi(bitmap.c_str());
			unsigned char i , mask;
			char buff_mo[10];
		
			memset(buff_mo, 0, sizeof(buff_mo));
			mask = 0x01;
			for (i = 0; i < 8; i++)
			{
			
				if ( mask == ( portmap & mask ))
				{
					sprintf(buff_mo, "port%d", i+1);
				
					addUserVariable(buff_mo , "checked");
				}
	
				mask = mask << 1;
			
			}
		
			addUserVariable("vlan_id",	vid);
			addUserVariable("filter_id",  fid);
			addUserVariable("card_no",	card_no);
			m_desthtml = "ge8vlan.html";
		}
	
	else
	{
	if ( flag == "ipinfo" )
	{
			msg_header->msg_code = C_GE8_NET_GET;
			
	}
	
	else if ( flag == "vlan" )
	{
		msg_header->msg_code = C_GE8_GETVLAN;
	}else if( flag == "delete")
	{
	
		
		//msg_header = (MSG_HEAD_CMD *)send_buff;
		vlan = (ONU_VLAN_CONF *)(send_buff +  sizeof(MSG_HEAD_CMD ));
		msg_header->msg_code = C_GE8_DELVLAN;
		//msg_header->card_no = (unsigned char) std::atoi(cardno.c_str());
		msg_header->node_id = (unsigned char) std::atoi(node_id.c_str());
		msg_header->length = sizeof(ONU_VLAN_CONF);
		
		vlan->setFlag = 0xFF;
		vlan->valid = 0x00;
		vlan->membership = (unsigned char)std::atoi(bitmap.c_str());
		vlan->id= (unsigned char)std::atoi(fid.c_str());
		vlan->vlan_id = (unsigned short)std::atoi(vid.c_str());
	}
	else if(flag == "save")
	{
		vid = getReqValue("vlan_id");
		vlan = (ONU_VLAN_CONF *)(send_buff +  sizeof(MSG_HEAD_CMD ));
		//logger(LOG_ERR,"send = %d\n",msg_header->card_no);
		msg_header->msg_code = C_GE8_SAVEVLAN;
		msg_header->length = sizeof(ONU_VLAN_CONF);
		vlan->setFlag = 0xFF;
		vlan->valid= 0x01;
		for ( i = 0 ; i < 8; i++ )
		{
			sprintf(buff, "port%d" , i + 1);
			temp = getReqValue(buff);
			if ( temp == "on")
			{
				map |= 0x01 << i; 
			}
		}
		vlan->membership=map;
		vlan->id= (unsigned char)std::atoi(fid.c_str());
		vlan->vlan_id = (unsigned short)std::atoi(vid.c_str());
		//logger(LOG_ERR,"send = %d\n",msg_header->card_no);
	}
	else if(flag == "portmode")
	{
		//logger(LOG_ERR,"portmode1\n");
		msg_header->msg_code = C_GE8_PORTMODE;
	}else if(flag == "port")
	{
		
		msg_header->msg_code = C_GE8_GETPORT;
		msg_header->length = sizeof(GE_PORT_STATE);
		porstate = (GE_PORT_STATE *) (send_buff + sizeof (MSG_HEAD_CMD ));
		porstate->portIndex = (unsigned char )std::atoi(portno.c_str() );
		//logger(LOG_ERR,"portno= %d\n",porstate->portIndex);
	}else if(flag=="flowinfo")
	{
		msg_header->msg_code = C_GE8_STATISTICS;
		msg_header->length = 8*sizeof(GE8_STATISTICS);
		portstatistics = (GE8_STATISTICS *) (send_buff + sizeof (MSG_HEAD_CMD ));
		for(count_yp=0;count_yp<8;count_yp++)
		{
			portstatistics->portIndex = count_yp;
			portstatistics++;
		}
	}else if(flag == "igmp")
	{
		msg_header->msg_code = C_GE8_GETIGMP;
		
	}


	
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
		
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;

	if(msgrsp_header->rsp_code != 0) 
	{
		//logger(LOG_ERR,"rsp error\n");

		return(msgrsp_header->rsp_code);
	}
	
	if (flag == "ipinfo" )
	{
		//logger(LOG_ERR,"ipinfo2\n");
		if(msgrsp_header->length != sizeof(GE8_NET_STATE))
				return(ERROR_COMM_PARSE);
	}


	
	
	
	/*else if ( flag == "vlan" )
	{
		if (msgrsp_header->length != sizeof(ONU_VLAN_CONF))
			return(ERROR_COMM_PARSE);
	}*/
	
	if ( flag == "base")
	{
		
		m_desthtml = "GE8.html";

	}
	else if ( flag == "ipinfo" )
	{
			//logger(LOG_ERR,"ipinfo3\n");
			netstate = (GE8_NET_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );
			sprintf (buff, "%s", netstate->ipaddr);
			addUserVariable("encode_ip", buff);
			sprintf (buff, "%s", netstate->mask);
			addUserVariable("encode_mask", buff);
			sprintf (buff, "%s", netstate->gateway);
			addUserVariable("encode_gateway", buff);
			
			covert_mac(netstate->mac,mac);			
			sprintf (buff, "%s", mac);
			addUserVariable("encode_mac", buff);
			//sprintf(buf_mode_port,"port_mode%d",i+1);
			if(netstate->enable==1)
			addUserVariable("ip_enable" , "checked");
			else;
			
			
			
			m_desthtml = "GE8_IP.html";
	}
	else if ( flag == "vlan" )
	{
		unsigned char i,id,members;
		unsigned short vlanid;
		unsigned char *totalnum,num;
		unsigned char validVlanNum = 0;
		totalnum = ( unsigned char *)(recv_buff + sizeof(MSG_HEAD_CMD_RSP ));
		num = (*totalnum);
		if ( msgrsp_header->length != (num*sizeof(ONU_VLAN_CONF) + sizeof(unsigned char)) )
			{
			//	logger(LOG_ERR,"vlan length\n");
				return (ERROR_COMM_PARSE);
			}
		vlan = (ONU_VLAN_CONF*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP) + sizeof(unsigned char) );
		
		if ( 0 == num )
		{
			outss << "";
		}
		else
		{
			for ( i = 0; i < num; i++ )
			{
				if(vlan->valid == 0x01)
				{
					validVlanNum = validVlanNum + 1;
					outss << "<tr>";
					//id
					id = vlan->id;
					//logger(LOG_ERR,"vlan id%d\n",id);
					sprintf(id_buff,"%d", id);
					outss << "<td>" <<  id_buff<< "</td>";

					sprintf(id_buff,"%d", id);
					//addUserVariable("id", id_buff);

					//vlanid
					
					vlanid = vlan->vlan_id;
					//logger(LOG_ERR,"vlan vid%d\n",vlanid);
					outss << "<td>" << vlanid << "</td>";

					//members
					members = vlan->membership;
					outss << "<td>" <<  getmembers( members ) << "</td>";
				//	logger(LOG_ERR,"vlan member%d\n",members);
					sprintf(field_name,"%d", members);
					//addUserVariable("members", field_name);

					//config
					sprintf(buff,"%d", msg_header->node_id);
					addUserVariable("node_id", buff);
					
					outss << "<td>" <<  "<a class='pagelink' href='/cgi-bin/show_ge8.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=${node_id}&flag=modify&fid=" << id_buff << "&vid=" << vlanid << "&bitmap=" << field_name << "'>" << multilang::getLabel("modify")<< "</a>" << "&nbsp;&nbsp;&nbsp;&nbsp;";
					outss <<  "<a class='pagelink' href='/cgi-bin/show_ge8.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=${node_id}&flag=delete&fid=" << id_buff << "&vid=" << vlanid << "&bitmap=" << field_name << "'>" << multilang::getLabel("delete")<< "</a>" << "</td>";

					outss << "</tr>\n";
				}
				else
				{
					outss << "";
				}

				vlan = vlan + 1;
			}
		}

		addUserVariable("vlan_items", outss.str());

		vlan = vlan - num + validVlanNum;
		if(vlan->valid == 0x01)
			sprintf(buff,"%d", vlan->id + 1);
		else
			sprintf(buff,"%d", vlan->id);
		addUserVariable("fid", buff);

		sprintf(buff,"%d", msg_header->node_id);
		addUserVariable("node_id", buff);

		outadd << "<tr>";
		outadd << "<td colspan=4>" <<"<a class='pagelink' href='/cgi-bin/show_ge8.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=${node_id}&flag=add&fid=${fid}'>" << multilang::getLabel("add")<< "</a>" << "</td>";
		outadd << "</tr>\n";

		addUserVariable("Add", outadd.str());
			
		m_desthtml = "ge8vlan_table.html";
	}
	else if(flag == "delete")
	{
		//logger(LOG_ERR,"delete2\n");
		sprintf(buff, "sessionID=%s&cardno=%s&onunode=%s&flag=base", getReqValue("sessionID").c_str(),card_no.c_str(),node_id.c_str());
		redirectHTMLFile("/cgi-bin/show_ge8.cgi", buff);
	}
	else if(flag == "modify")
	{
		
	}
	else if(flag == "save")
	{
		sprintf(buff, "sessionID=%s&cardno=%s&onunode=%s&flag=base", getReqValue("sessionID").c_str(),card_no.c_str(),node_id.c_str());
		redirectHTMLFile("/cgi-bin/show_ge8.cgi", buff);
	}
	else if(flag == "portmode")
	{
		//logger(LOG_ERR,"portmode2\n");
		//logger(LOG_ERR,"port length  %d\n",msgrsp_header->length);
		if ( msgrsp_header->length != (8*sizeof(GE_PORT_STATE)))
		{
			
			return (ERROR_COMM_PARSE);
		}
		portlink=(GE_PORT_STATE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
		for ( i = 0; i < 8; i++ )
		{
			
			//logger(LOG_ERR,"bw_alloc_send %d,bw_alloc_recv %d,port= %x\n",portlink->bw_alloc_send,portlink->bw_alloc_send,portlink->port_state);
			if(portlink->port_state!=0xff)
			{
				sprintf(buff_port, "port%d", i+1);
				sprintf(buf_mode_port,"port_mode%d",i+1);
				switch(portlink->port_state)
				{
					case 1:
						addUserVariable(buff_port, "<img  title='ON'  src='/images/green.gif' >");
						
						addUserVariable(buf_mode_port, "100M-FULL");
						break;
					case 2:
						addUserVariable(buff_port, "<img  title='ON'  src='/images/green.gif' >");
						//sprintf(buf_mode_port,"port_mode%d",i+1);
						addUserVariable(buf_mode_port, "1000M-FULL");
						break;
					case 4:
						addUserVariable(buff_port, "<img  title='ON'  src='/images/green.gif'  >");
					//	sprintf(buf_mode_port,"port_mode%d",i+1);
						addUserVariable(buf_mode_port, "10M-FULL");
						break;
					case 5:
						addUserVariable(buff_port, "<img  title='ON'  src='/images/green.gif'  >");
						//sprintf(buf_mode_port,"port_mode%d",i+1);
						addUserVariable(buf_mode_port, "10M-HALF");
						break;
					default:
						break;
				}
				//logger(LOG_ERR,"port= %x\n",portlink->port_state);
			}else
			{
				sprintf(buff_port, "port%d", i+1);
				sprintf(buf_mode_port,"port_mode%d",i+1);
				//addUserVariable(buff_port , "checked");
				addUserVariable(buff_port, "<img  title='ON'  src='/images/red.gif'>");
				addUserVariable(buf_mode_port, "DOWN");
			}
			portlink=portlink+1;
		}
		m_desthtml="ge8link_table.html";
		
	}
	
	else if ( flag == "port" )
		{
			porstate = (GE_PORT_STATE *) (recv_buff + sizeof (MSG_HEAD_CMD_RSP));
			sprintf (buff, "%d", porstate->portIndex);
			
			addUserVariable("portno" ,	buff);
			
			addUserVariable("fe_admin_state", getStateGe8(porstate->admin_state));

			//addUserVariable("fe_port_state" , getLink(porstate->port_state ) );
			
			addUserVariable("fe_port_mode" , getModeGE8(porstate->portMode ));
			addUserVariable("fe_working_mode", getWorkingModeGE8(porstate->portWorkingMode ));
			
			sprintf (buff, "%d", (unsigned char )std::atoi(card_no.c_str() ));
			addUserVariable("card_no", buff);
			
			m_desthtml = "ge8port_table.html";
	
		}else if(flag == "flowinfo")
		{
			//logger(LOG_ERR,"flowinfo\n");
			if ( msgrsp_header->length != (8*sizeof(GE8_STATISTICS)))
			{
				logger(LOG_ERR,"length error\n");
			//	return (ERROR_COMM_PARSE);
				
			}
			portstatistics = (GE8_STATISTICS *) (recv_buff + sizeof (MSG_HEAD_CMD_RSP ));
			for(count_yp=0;count_yp<8;count_yp++)
			{
				//logger(LOG_ERR,"rpack %d,tpack %d,rbyte %d,tbyte %d\n",portstatistics->Rx_Packets,portstatistics->Tx_Packets,portstatistics->Rx_Packets_B,portstatistics->Tx_Packets_B);
				if(count_yp == 0)
				{
					sprintf (buff, "%d", portstatistics->Rx_Packets);
					addUserVariable("port_recv_pack1" ,	buff);
					//sprintf(buff,"igmp_vlanid%d",igmp_i);
				
					//logger(LOG_ERR,"igmp_vid%s\n",getReqValue("port_recv_pack1").c_str());
					sprintf (buff, "%d", portstatistics->Tx_Packets);
					addUserVariable("port_send_pack1" ,	buff);
					sprintf (buff, "%d", portstatistics->Rx_Packets_B);
					addUserVariable("port_recv_byte1" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets_B);
					addUserVariable("port_send_byte1" ,	buff);
				}else if(count_yp == 1)
				{
					sprintf (buff, "%d", portstatistics->Rx_Packets);
					addUserVariable("port_recv_pack2" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets);
					addUserVariable("port_send_pack2" ,	buff);
					sprintf (buff, "%d", portstatistics->Rx_Packets_B);
					addUserVariable("port_recv_byte2" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets_B);
					addUserVariable("port_send_byte2" ,	buff);
				}else if(count_yp == 2)
				{
					sprintf (buff, "%d", portstatistics->Rx_Packets);
					addUserVariable("port_recv_pack3" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets);
					addUserVariable("port_send_pack3" ,	buff);
					sprintf (buff, "%d", portstatistics->Rx_Packets_B);
					addUserVariable("port_recv_byte3" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets_B);
					addUserVariable("port_send_byte3" ,	buff);
				}else if(count_yp == 3)
				{
					sprintf (buff, "%d", portstatistics->Rx_Packets);
					addUserVariable("port_recv_pack4" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets);
					addUserVariable("port_send_pack4" ,	buff);
					sprintf (buff, "%d", portstatistics->Rx_Packets_B);
					addUserVariable("port_recv_byte4" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets_B);
					addUserVariable("port_send_byte4" ,	buff);
				}else if(count_yp == 4)
				{
					sprintf (buff, "%d", portstatistics->Rx_Packets);
					addUserVariable("port_recv_pack5" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets);
					addUserVariable("port_send_pack5" ,	buff);
					sprintf (buff, "%d", portstatistics->Rx_Packets_B);
					addUserVariable("port_recv_byte5" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets_B);
					addUserVariable("port_send_byte5" ,	buff);
				}else if(count_yp == 5)
				{
					sprintf (buff, "%d", portstatistics->Rx_Packets);
					addUserVariable("port_recv_pack6" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets);
					addUserVariable("port_send_pack6" ,	buff);
					sprintf (buff, "%d", portstatistics->Rx_Packets_B);
					addUserVariable("port_recv_byte6" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets_B);
					addUserVariable("port_send_byte6" ,	buff);
				}else if(count_yp == 6)
				{
					sprintf (buff, "%d", portstatistics->Rx_Packets);
					addUserVariable("port_recv_pack7" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets);
					addUserVariable("port_send_pack7" ,	buff);
					sprintf (buff, "%d", portstatistics->Rx_Packets_B);
					addUserVariable("port_recv_byte7" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets_B);
					addUserVariable("port_send_byte7" ,	buff);
				}else if(count_yp == 7)
				{
					sprintf (buff, "%d", portstatistics->Rx_Packets);
					addUserVariable("port_recv_pack8" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets);
					addUserVariable("port_send_pack8" ,	buff);
					sprintf (buff, "%d", portstatistics->Rx_Packets_B);
					addUserVariable("port_recv_byte8" ,	buff);
					sprintf (buff, "%d", portstatistics->Tx_Packets_B);
					addUserVariable("port_send_byte8" ,	buff);
				}
				portstatistics++;

				
			}
			logger(LOG_ERR,"flowinfo end\n");
			m_desthtml = "ge8perf_table.html";
		}
		else if(flag == "igmp")
		{
			//logger(LOG_ERR,"card no%s\n",card_no.c_str());
			int igmp_i=0,igmp_vlan_id,igmpnum=0;
			std::string igmpstate;
			unsigned char routermask=0x01;
			char tmp_buf[10];
			if ( msgrsp_header->length != sizeof(GE8_IGMP) )
			{
			//	logger(LOG_ERR,"vlan length\n");
				return ERROR_COMM_PARSE;
			}
			igmp_yp=(GE8_IGMP *)(recv_buff + sizeof (MSG_HEAD_CMD_RSP ));
			if(igmp_yp->State == 1)
			{
				addUserVariable("igmp_enable" , "checked");
				//igmpstate=getReqValue("igmp_enable");
			//	logger(LOG_ERR,"state %s\n",igmpstate.c_str());
				buf_igmp_dat[0]=1;
			}else
			{
				buf_igmp_dat[0]=0;
				
			}
			if(igmp_yp->Ipmc_flood==1)
			{
				addUserVariable("igmp_flood" , "checked");
				buf_igmp_dat[2]=1;
			}else
			{
				buf_igmp_dat[2]=0;
			}
			
			sprintf(buff_igmp, "%d", igmp_yp->Route_port);
			addUserVariable("router_port" ,buff_igmp);
			//addUserVariable("igmp_vlannum" ,"2");
			for(igmp_i=0;igmp_i<8;igmp_i++)
			{
				if(((igmp_yp->Route_port)&( routermask <<igmp_i))!=0 )
				{
					//logger(LOG_ERR,"enter router port\n");
					sprintf(buff_igmp, "igmp_p%d", igmp_i);
					addUserVariable(buff_igmp , "checked");
				//	logger(LOG_ERR,"buff_igmp %s\n",getReqValue(buff_igmp).c_str());
				}
			}
			
			buf_igmp_dat[4]=igmp_yp->Route_port;
			igmp_i=0;
			while(igmp_yp->Vid[igmp_i]!=0xff)
			{
				igmp_i++;
				igmpnum++;
			}
			buf_igmp_dat[6]=igmpnum;
			
			for(igmp_i=0;igmp_i<igmpnum;igmp_i++)
			{
				
					outss << "<tr>";
					igmp_vlan_id=igmp_yp->Vid[igmp_i];
					sprintf(buff_igmp, "igmp_vlanid%d",igmp_i);
					sprintf(tmp_buf,"%d",igmp_vlan_id);
					addUserVariable(buff_igmp,tmp_buf);
					//logger(LOG_ERR,"vid %d  %s\n",igmp_i,getReqValue(buff_igmp).c_str());
					outss << "<td> <name='"<<buff_igmp<<"'>${"<<buff_igmp<<"}"<< "</td>";
					outss << "<input type='hidden' name="<<buff_igmp<<" value="<<tmp_buf<<">";
					//logger(LOG_ERR,"vid %d  %s\n",igmp_i,buff_igmp);
					sprintf(buff_igmp, "Snoop_state%d", igmp_i);
					
					if(igmp_yp->Snoop_state[igmp_i]==1)
					{
					
					outss << "<td>" <<"<input type='CHECKBOX' checked='true' name="<<buff_igmp<<" "<< buff_igmp<<"></input>" << "</td>";
					//addUserVariable(buff_igmp ,"checked" );
					//outss << "<td>"<<buff_igmp<<"</td>";
					addUserVariable(buff_igmp ,"true" );
					
					}else
					{
					outss << "<td>" <<"<input type='CHECKBOX'  name="<<buff_igmp<<" "<< buff_igmp<<"></input>" << "</td>";
					addUserVariable(buff_igmp ,"false" );
					
					}

					sprintf(buff_igmp, "Query_state%d", igmp_i);
					if(igmp_yp->Query_state[igmp_i]==1)
					{
					
					outss << "<td>" <<"<input type='CHECKBOX' checked='true'  name="<<buff_igmp<<" "<<buff_igmp<<"></input>" << "</td>";
					//addUserVariable(buff_igmp ,"checked" );
					addUserVariable(buff_igmp ,"true" );
					
					}else
					{
					outss << "<td>" <<"<input type='CHECKBOX'  name="<<buff_igmp<<" "<< buff_igmp<<"></input>" << "</td>";
					addUserVariable(buff_igmp ,"false" );
					
					}
					outss << "</tr>\n";
				
				
			}
			
			sprintf(tmp_buf,"%d",igmp_i);
			addUserVariable("igmp_vlannum",tmp_buf);
			
			addUserVariable("igmp_iems", outss.str());
			m_desthtml = "ge8igmp_table.html";

			
		/*
			if ( 0 == num )
			{
				outss << "";
			}
			else
			{
			for ( i = 0; i < num; i++ )
			{
				if(vlan->valid == 0x01)
				{
					validVlanNum = validVlanNum + 1;
					outss << "<tr>";
					//id
					id = vlan->id;
					//logger(LOG_ERR,"vlan id%d\n",id);
					outss << "<td>" << id << "</td>";

					sprintf(id_buff,"%d", id);
					//addUserVariable("id", id_buff);

					//vlanid
					
					vlanid = vlan->vlan_id;
					//logger(LOG_ERR,"vlan vid%d\n",vlanid);
					outss << "<td>" << vlanid << "</td>";

					//members
					members = vlan->membership;
					outss << "<td>" <<  getmembers( members ) << "</td>";
				//	logger(LOG_ERR,"vlan member%d\n",members);
					sprintf(field_name,"%d", members);
					//addUserVariable("members", field_name);

					//config
					sprintf(buff,"%d", msg_header->node_id);
					addUserVariable("node_id", buff);
					
					outss << "</tr>\n";
				}
				else
				{
					outss << "";
				}

				vlan = vlan + 1;
			}
		}

		addUserVariable("vlan_items", outss.str());

		vlan = vlan - num + validVlanNum;
		if(vlan->valid == 0x01)
			sprintf(buff,"%d", vlan->id + 1);
		else
			sprintf(buff,"%d", vlan->id);
		addUserVariable("fid", buff);

		sprintf(buff,"%d", msg_header->node_id);
		addUserVariable("node_id", buff);

		outadd << "<tr>";
		outadd << "<td colspan=4>" <<"<a class='pagelink' href='/cgi-bin/show_ge8.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=${node_id}&flag=add&fid=${fid}'>" << multilang::getLabel("add")<< "</a>" << "</td>";
		outadd << "</tr>\n";

		addUserVariable("Add", outadd.str());*/
			
		//m_desthtml = "ge8vlan_table.html";
	
		}

	}
	
	return 0;

}
void CGetGE8::output()
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

	int ret = getGe8Data();
	
	if(ret == 0) {
		if(outputHTMLFile(m_desthtml, true) != 0)
		{  
			outputError("Cann't open onu.html\n");
		}
		//logger(LOG_ERR,"vid1 %s\n",getReqValue("igmp_vlanid1").c_str());
	} 
	else 
		outputError(multilang::getError(ret));

	free(cs);

}



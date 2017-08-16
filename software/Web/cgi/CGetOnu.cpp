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
#include "CGetOnu.h"

using namespace std;


const char *CGetOnu::getLink(unsigned char type)
{	switch(type)
	{		case 1:			return("Up");
			case 2:			return("Down");
			default:			return("Unknown");
			}
}

const char *CGetOnu::getWorkingModeONU(unsigned char type)
{
	switch(type)
				{

					case 1:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' selected> Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' selected> 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 3:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' selected> 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 4:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' selected> 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 5:
							return("<select class='fixsize'   name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' selected> 10M-Half</option>\
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

const char *CGetOnu::getModeONU(unsigned char type)
{

switch(type)
			{
				case 1:
					return ("<select class='fixsize' id='fe_port_mode'  name='fe_port_mode' onchange='onWorkingModeChg(\"fe_port_mode\");'> \
						<option value='1' selected> learning mode</option>\
						<option value='2' > bounded MAC</option>\
						</select>");
				case 2:
					return ("<select class='fixsize' id='fe_port_mode'  name='fe_port_mode' onchange='onWorkingModeChg(\"fe_port_mode\");'> \
						<option value='1' > learning mode</option>\
						<option value='2' selected> bounded MAC</option>\
						</select>");
				default:
					return ("<select class='fixsize' id='fe_port_mode'  name='fe_port_mode' onchange='onWorkingModeChg(\"fe_port_mode\");'> \
							<option value='1' > learning mode</option>\
							<option value='2' > bounded MAC</option>\
							<option value='3' selected>not-support</option>\
							</select>");
			
				}

}

const char *CGetOnu::getBaudrate(unsigned char type)
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


const char *CGetOnu::getprioritySchemeSelect(unsigned char type)
{
	switch ( type )
	{
		{
			case 0x00:
				return ("<select class='fixsize'  name='priorityScheme_select'  id='priorityScheme_select'> \
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


const char *CGetOnu::getPortselect(unsigned char type)
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

const char *CGetOnu::getStopbit(unsigned char type)
{
	switch(type)
	{
		{
			case 0:
				return ("<option value='0' selected> 1</option>\
					<option value='1' > 2</option>");
			case 1:
				return ("<option value='0' > 1</option>\
					<option value='1' selected> 2</option>");
			default:
				return ("<option value='0' > 1</option>\
						<option value='1' > 2</option>\
						<option value='2' selected>not-support</option>");
		
			}	
	}

}

const char *CGetOnu::getDatabit(unsigned char type)
{
	switch(type)
	{
		{
			case 0:
				return ("<option value='0' selected> 5</option>\
						<option value='1' > 6</option>\
						<option value='2' > 7</option>\
						<option value='3' > 8</option>");
			case 1:
				return ("<option value='0' > 5</option>\
						<option value='1' selected> 6</option>\
						<option value='2' > 7</option>\
						<option value='3' > 8</option>");
			case 2:
				return ("<option value='0' > 5</option>\
						<option value='1' > 6</option>\
						<option value='2' selected> 7</option>\
						<option value='3' > 8</option>");
			case 3:
				return ("<option value='0' > 5</option>\
						<option value='1' > 6</option>\
						<option value='2' > 7</option>\
						<option value='3' selected> 8</option>");
			default:
				return ("<option value='0' > 5</option>\
						<option value='1' > 6</option>\
						<option value='2' > 7</option>\
						<option value='3' > 8</option>\
						<option value='7' selected>not-support</option>");
		
			}	
	}

}

const char *CGetOnu::getParity(unsigned char type)
{
	switch(type)
	{
		{
			case 0:
			case 2:
			case 4:
			case 6:
				return ("<option value='0' selected> None</option>\
						<option value='1' > Odd</option>\
						<option value='3' > Even</option>\
						<option value='5' > Mark</option>\
						<option value='7' > Space</option>");
			case 1:
				return ("<option value='0' > None</option>\
						<option value='1' selected> Odd</option>\
						<option value='3' > Even</option>\
						<option value='5' > Mark</option>\
						<option value='7' > Space</option>");
			case 3:
				return ("<option value='0' > None</option>\
						<option value='1' > Odd</option>\
						<option value='3' selected> Even</option>\
						<option value='5' > Mark</option>\
						<option value='7' > Space</option>");
			case 5:
				return ("<option value='0' > None</option>\
						<option value='1' > Odd</option>\
						<option value='3' > Even</option>\
						<option value='5' selected> Mark</option>\
						<option value='7' > Space</option>");
			case 7:
				return ("<option value='0' > None</option>\
						<option value='1' > Odd</option>\
						<option value='3' > Even</option>\
						<option value='5' > Mark</option>\
						<option value='7' selected> Space</option>");
			default:
				return ("<option value='0' > None</option>\
						<option value='1' > Odd</option>\
						<option value='3' > Even</option>\
						<option value='5' > Mark</option>\
						<option value='7' > Space</option>\
						<option value='8' selected>not-support</option>");
		
			}	
	}

}

const char *CGetOnu::getmembers(unsigned char map)
{
	unsigned char i , mask;
	char temp[5];
	char static buff[256];
	
	memset(temp, 0 , sizeof(temp));
	memset(buff, 0, sizeof (buff) );
	mask = 0x01;
	for ( i = 0 ; i < 5; i++ )
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



int CGetOnu::getOnuData()
{
	std::string card_no = getReqValue("cardno");
	std::string node_id = getReqValue("onunode");
	std::string flag = getReqValue ("flag");
	std::string portno = getReqValue ("portno");

	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	ONU_INFO *	onu_info;
	ONU_UART_CONF *uart;
	ONU_PORTMIRROR_CONF *portMirror;
	ONU_QoS_CONF *qos;
	ONU_VLAN_CONF *vlan;
	FE_PORT_STATE *port_info;
	FE_PORT_STATE *portIndex;
	stringstream outss;
	stringstream outadd;
	char buff[256];	
	char field_name[32];
	char id_buff[8];
	
	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	msg_header->length = 0;
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	msg_header->node_id = (unsigned char) std::atoi(node_id.c_str());

	if ( flag == "initonu" )
	{
		addUserVariable("cardno", card_no);
		addUserVariable("node_id", node_id);
		m_desthtml = "onu.html";
		return 0;
	}
	if ( flag == "dau" )
	{
		addUserVariable("cardno", card_no);
		addUserVariable("node_id", node_id);
		m_desthtml = "dau.html";
		return 0;
	}
	if ( flag == "base")
	{
		msg_header->msg_code = C_ONU_GET;
		
	}
	else if ( flag == "datach" )
	{
		msg_header->msg_code = C_ONU_GETDATACH;
	}
	else if ( flag == "io" )
	{
		msg_header->msg_code = C_ONU_GETIO;
	}
	else if ( flag == "portmirror" )
	{
		msg_header->msg_code = C_ONU_GETPORTMIRROR;
	}
	else if ( flag == "qos" )
	{
		msg_header->msg_code = C_ONU_GETQoS;
	}
	else if ( flag == "vlan" )
	{
		msg_header->msg_code = C_ONU_GETVLAN;
	}
	else if ( flag == "port" )
	{
		msg_header->msg_code = C_ETHER_GET;
		msg_header->length = sizeof (FE_PORT_STATE);
		portIndex = (FE_PORT_STATE *) (send_buff + sizeof (MSG_HEAD_CMD ));
		portIndex->portIndex = (unsigned char )std::atoi(portno.c_str() );
	}

	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
		
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);
	
	if (flag == "base" )
	{

		if(msgrsp_header->length != sizeof(ONU_INFO))
			return(ERROR_COMM_PARSE);
	}
	else if ( flag == "port" )
	{
		if(msgrsp_header->length != sizeof(FE_PORT_STATE))
			return(ERROR_COMM_PARSE);
	}
	else if ( flag == "datach" )
	{
		unsigned char uart_num = *(recv_buff  + sizeof(MSG_HEAD_CMD_RSP));
		if(msgrsp_header->length != (1 + uart_num * sizeof(ONU_UART_CONF)))
			return(ERROR_COMM_PARSE);
	}
	else if ( flag == "io" )
	{
		if (msgrsp_header->length != sizeof(ONU_IO))
			return(ERROR_COMM_PARSE);
	}
	else if ( flag == "portmirror" )
	{
		if (msgrsp_header->length != sizeof(ONU_PORTMIRROR_CONF))
			return(ERROR_COMM_PARSE);
	}
	else if ( flag == "qos" )
	{
		if (msgrsp_header->length != sizeof(ONU_QoS_CONF))
			return(ERROR_COMM_PARSE);
	}
	/*else if ( flag == "vlan" )
	{
		if (msgrsp_header->length != sizeof(ONU_VLAN_CONF))
			return(ERROR_COMM_PARSE);
	}*/
	
	if ( flag == "base")
	{
		onu_info = (ONU_INFO*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
		sprintf(buff,"%d", onu_info->node_id);
		addUserVariable("node_id", buff);
		sprintf(buff,"%d", onu_info->node_sn);
		addUserVariable("node_sn", buff);
		memcpy(buff , onu_info->onu_name, MAX_DISPLAYNAME_LEN );
		buff[MAX_DISPLAYNAME_LEN] = '\0';
		addUserVariable("onuname", buff);
		addAdminStateVariable("admin_state", onu_info->admin_state);
		addUserVariable("storm_ctrl", multilang::getstormstate(onu_info->broadcaststorm_ctrl));
		sprintf(buff, "%d",  ntohs(onu_info->storm_threshold));
		addUserVariable("storm_threshold",buff);

		if ( onu_info->spf_n_state == 1)
			addUserVariable ("sfp_n_state", multilang::getSfpState(onu_info->spf_n_state)); //removed
		else
		{
			sprintf(buff, "%s%s",  multilang::getSfpState(onu_info->spf_n_state), 
				"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  <a href='/cgi-bin/sfpinfo.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=${node_id}&sfpno=1'><IMG src='/images/sfp_info.gif'></a>");
			addUserVariable ("sfp_n_state", buff);
		}
		
		if ( onu_info->spf_f_state == 1)
			addUserVariable ("sfp_f_state", multilang::getSfpState(onu_info->spf_f_state)); //removed
		else
		{
			sprintf(buff, "%s%s",  multilang::getSfpState(onu_info->spf_f_state), 
				"&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  <a href='/cgi-bin/sfpinfo.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=${node_id}&sfpno=2'><IMG src='/images/sfp_info.gif'></a>");
			addUserVariable ("sfp_f_state", buff);
		}
		
		
		m_desthtml = "onubase_table.html";

	}
	else if ( flag == "datach" )
	{
		std::string type = getReqValue ("type");
		
		// support two uart for each ONU currently
		uart = (ONU_UART_CONF *)(recv_buff + 1+ sizeof(MSG_HEAD_CMD_RSP));

		//port 1
		addUserVariable("baudrate1", getBaudrate(uart[0].baudrate));
		addUserVariable("port_sel1", multilang::getdatach(uart[0].function));
		sprintf(buff, "%d", uart[0].time);
		addUserVariable("data_buff1", buff);

		//port 2
		addUserVariable("baudrate2", getBaudrate(uart[1].baudrate));
		addUserVariable("port_sel2", getPortselect(uart[1].function));
		sprintf(buff, "%d", uart[1].time);
		addUserVariable("data_buff2", buff);

		if(type == "dau")
		{
/*			//port1
			addUserVariable("databit1", getDatabit(uart[0].control&0x03));
			addUserVariable("stopbit1", getStopbit((uart[0].control>>2)&0x01));
			addUserVariable("parity1", getParity((uart[0].control>>3)&0x07));

			//port2
			addUserVariable("databit2", getDatabit(uart[1].control&0x03));
			addUserVariable("stopbit2", getStopbit((uart[1].control>>2)&0x01));
			addUserVariable("parity2", getParity((uart[1].control>>3)&0x07));
*/			
			//port 3
			addUserVariable("baudrate3", getBaudrate(uart[2].baudrate));
			sprintf(buff, "%d", uart[2].time);
			addUserVariable("data_buff3", buff);
			if(uart[2].function&0x80)
				addUserVariable("port_sel3", multilang::getLabel("none"));
			
			//port 4
			addUserVariable("baudrate4", getBaudrate(uart[3].baudrate));
			sprintf(buff, "%d", uart[3].time);
			addUserVariable("data_buff4", buff);
			if(uart[3].function&0x80)
				addUserVariable("port_sel4", multilang::getLabel("none"));
			
			//port 5
			addUserVariable("baudrate5", getBaudrate(uart[4].baudrate));
			sprintf(buff, "%d", uart[4].time);
			addUserVariable("data_buff5", buff);
			if(uart[4].function&0x80)
				addUserVariable("port_sel5", multilang::getLabel("none"));
			
			m_desthtml = "daudc_table.html";
		}
		else
			m_desthtml = "onudc_table.html";
		
	}
	else if ( flag == "io" )
	{
		ONU_IO * io_stat;		
		char v_name[8];
		unsigned char mask;

		io_stat = (ONU_IO*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));

		mask = 1;
		for(int i=0; i<8; i++)
		{
			sprintf(v_name, "I%d", i+1);
			if(mask & io_stat->in)
				addUserVariable(v_name, "<img  title='OFF'  src='/images/green.gif'>");
			else
				addUserVariable(v_name, "<img  title='ON'  src='/images/red.gif'>");
			mask <<= 1;
		}

		m_desthtml = "onuio_table.html";
	}
	else if ( flag == "portmirror" )
	{
		portMirror = (ONU_PORTMIRROR_CONF *)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
		//port1
		if( portMirror->port1[0] != 0 )
			addUserVariable("port1Mirror","checked");
		else
			addUserVariable("port1Mirror","");
		
		if( portMirror->port1[1] != 0 )
			addUserVariable("port1Rx","checked");
		else
			addUserVariable("port1Rx","");

		if( portMirror->port1[2] != 0 )
			addUserVariable("port1Tx","checked");
		else
			addUserVariable("port1Tx","");

		//port2
		if( portMirror->port2[0] != 0 )
			addUserVariable("port2Mirror","checked");
		else
			addUserVariable("port2Mirror","");
		
		if( portMirror->port2[1] != 0 )
			addUserVariable("port2Rx","checked");
		else
			addUserVariable("port2Rx","");

		if( portMirror->port2[2] != 0 )
			addUserVariable("port2Tx","checked");
		else
			addUserVariable("port2Tx","");

		//port3
		if( portMirror->port3[0] != 0 )
			addUserVariable("port3Mirror","checked");
		else
			addUserVariable("port3Mirror","");
		
		if( portMirror->port3[1] != 0 )
			addUserVariable("port3Rx","checked");
		else
			addUserVariable("port3Rx","");

		if( portMirror->port3[2] != 0 )
			addUserVariable("port3Tx","checked");
		else
			addUserVariable("port3Tx","");

		//port4
		if( portMirror->port4[0] != 0 )
			addUserVariable("port4Mirror","checked");
		else
			addUserVariable("port4Mirror","");
		
		if( portMirror->port4[1] != 0 )
			addUserVariable("port4Rx","checked");
		else
			addUserVariable("port4Rx","");

		if( portMirror->port4[2] != 0 )
			addUserVariable("port4Tx","checked");
		else
			addUserVariable("port4Tx","");

		//port5
		if( portMirror->port5[0] != 0 )
			addUserVariable("port5Mirror","checked");
		else
			addUserVariable("port5Mirror","");
		
		if( portMirror->port5[1] != 0 )
			addUserVariable("port5Rx","checked");
		else
			addUserVariable("port5Rx","");

		if( portMirror->port5[2] != 0 )
			addUserVariable("port5Tx","checked");
		else
			addUserVariable("port5Tx","");


		if(portMirror->badframes != 0 )
			addUserVariable("badframes","checked");
		else
			addUserVariable("badframes","");

		if(portMirror->sourceanddestination != 0 )
			addUserVariable("sourceanddestination","checked");
		else
			addUserVariable("sourceanddestination","");
		
		m_desthtml = "onupm_table.html";
	}
	else if ( flag == "qos" )
	{
		qos= (ONU_QoS_CONF*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));

		addUserVariable("port1priority_enable", multilang::getport1priorityEnable(qos->priorityEnable[0]));
		addUserVariable("port2priority_enable", multilang::getport2priorityEnable(qos->priorityEnable[1]));
		addUserVariable("port3priority_enable", multilang::getport3priorityEnable(qos->priorityEnable[2]));
		addUserVariable("port4priority_enable", multilang::getport4priorityEnable(qos->priorityEnable[3]));
		addUserVariable("port5priority_enable", multilang::getport5priorityEnable(qos->priorityEnable[4]));

		addUserVariable("priorityScheme_select", getprioritySchemeSelect(qos->prioritySchemeSelect));

		m_desthtml = "onuqos_table.html";
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
			return (ERROR_COMM_PARSE);

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
					outss << "<td>" <<(1 + id) << "</td>";

					sprintf(id_buff,"%d", id);
					//addUserVariable("id", id_buff);

					//vlanid
					vlanid = vlan->vlan_id;
					outss << "<td>" << vlanid << "</td>";

					//members
					members = vlan->membership;
					outss << "<td>" <<  getmembers( members ) << "</td>";

					sprintf(field_name,"%d", members);
					//addUserVariable("members", field_name);

					//config
					sprintf(buff,"%d", msg_header->node_id);
					addUserVariable("node_id", buff);
					
					outss << "<td>" <<  "<a class='pagelink' href='/cgi-bin/chgonu.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=${node_id}&flag=modify&fid=" << id_buff << "&vid=" << vlanid << "&bitmap=" << field_name << "'>" << multilang::getLabel("modify")<< "</a>" << "&nbsp;&nbsp;&nbsp;&nbsp;";
					outss <<  "<a class='pagelink' href='/cgi-bin/chgonu.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=${node_id}&flag=delete&fid=" << id_buff << "&vid=" << vlanid << "&bitmap=" << field_name << "'>" << multilang::getLabel("delete")<< "</a>" << "</td>";

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
		outadd << "<td colspan=4>" <<"<a class='pagelink' href='/cgi-bin/chgonu.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=${node_id}&flag=add&fid=${fid}'>" << multilang::getLabel("add")<< "</a>" << "</td>";
		outadd << "</tr>\n";

		addUserVariable("Add", outadd.str());
			
		m_desthtml = "onuvlan_table.html";
	}	
	else if ( flag == "port" )
	{
		port_info = (FE_PORT_STATE *) (recv_buff + sizeof (MSG_HEAD_CMD_RSP));
		sprintf (buff, "%d", portIndex->portIndex);
		addUserVariable("portno" ,  buff);
		addAdminStateVariable("fe_admin_state", port_info->admin_state);
		addUserVariable("fe_port_state" , getLink(port_info->port_state ) );

		sprintf(buff,"%d", ntohs(port_info->bw_alloc_send) * 32);
		addUserVariable("fe_bw_alloc_send", buff);
		sprintf(buff,"%d", ntohs(port_info->bw_alloc_recv) * 32 );
		addUserVariable("fe_bw_alloc_recv", buff);
		
		addUserVariable("fe_port_mode" , getModeONU(port_info->portMode ));
		addUserVariable("fe_working_mode", getWorkingModeONU(port_info->portWorkingMode ));
		
		
		addUserVariable("fe_bounded_mac", cgicc::getMACaddr(port_info->bounded_mac));

		sprintf(buff,"%d", (unsigned char )std::atoi (node_id.c_str())); //for the perf node id .
		addUserVariable("node_id", buff);
		sprintf (buff, "%d", (unsigned char )std::atoi(card_no.c_str() ));
		addUserVariable("card_no", buff);
		
		m_desthtml = "port_table.html";

	}	

	return 0;

}

void CGetOnu::output()
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
			outputError("Cann't open onu.html\n");
		}	
	} 
	else 
		outputError(multilang::getError(ret));

	free(cs);

}


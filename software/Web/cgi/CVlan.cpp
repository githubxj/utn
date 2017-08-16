#include <iostream>
#include <sstream> 
#include <vector>
#include <stdio.h>
#include <string.h>

#include "CGISession.h"
#include "../../common/const.h"
#include "../../common/msg.h"
#include "Tools.h"
#include "Lang.h"
#include "CgiRequest.h"
#include "CppSQLite3.h"
#include "Comm.h"

#include "CVlan.h"

using namespace std;

const char *CVlan::getmembers(int map)
{
	int i , mask;
	char temp[5];
	char static buff[256];
	
	memset(temp, 0 , sizeof(temp));
	memset(buff, 0, sizeof (buff) );
	mask = 0x01;
	for ( i = 0 ; i < 26; i++ )
	{
		if ( mask == ( map & mask) )
		{
			if ( (24 == i) || ( 25 == i ) )
			{
				sprintf(temp, "g%d ", i-23);
			}
			else
			{
				sprintf(temp, "p%d ", i+1);
			}
			strcat(buff, temp);
		}
		mask = mask << 1;
	}

	return buff;

}

int CVlan::Getvlan()
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;

	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));

	msg_header->msg_code = C_SW_GETVLAN;
	msg_header->card_no = CTRL_CARDNO;
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	m_desthtml = "vlan_table.html";
	return 0;


}

int CVlan::Addvlan()
{
	m_desthtml = "vlan.html";
	return 0;
}

int CVlan::Modifyvlan()
{
	
	std::string vid = getReqValue("vid");
	std::string bitmap = getReqValue("bitmap");
	int portmap = std::atoi(bitmap.c_str());
	int i , mask;
	char buff[10];

	memset(buff, 0, sizeof(buff));
	mask = 0x01;
	for (i = 0; i < 26; i++)
	{
		
		if ( mask == ( portmap & mask ))
		{
			if ( (24 == i) || (25 == i))
			{
				sprintf(buff, "ge%d", i - 23);
			}
			else
			{
				sprintf(buff, "port%d", i+1);
			}
			
			addUserVariable(buff , "checked");
		}

		mask = mask << 1;
		
	}
	
	addUserVariable("vlan_id",  vid);
	m_desthtml = "vlan.html";
	
	return 0;
}

int CVlan::Savevlan()
{

	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	char buff[128];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	std::string vid = getReqValue("vlan_id");
	std::string temp;
	int map = 0 , i;
	SW_VLAN *vlan;

	msg_header = (MSG_HEAD_CMD *)send_buff;
	vlan = (SW_VLAN *)(send_buff +  sizeof(MSG_HEAD_CMD ));
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	memset(buff, 0, sizeof(buff));

	msg_header->msg_code = C_SW_SAVEVLAN;
	msg_header->card_no = CTRL_CARDNO;
	msg_header->length = sizeof(SW_VLAN);
	
	vlan->vlan_id = std::atoi(vid.c_str());
	vlan->setFlag = 0xFF;
	//add bitmap
	for ( i = 0 ; i < 26; i++ )
	{
		if ( ( 24 == i  ) || ( 25 == i ) )
		{
			sprintf(buff, "ge%d", i-23);	
		}
		else
		{
			sprintf(buff, "port%d" , i + 1);
		}
		temp = getReqValue(buff);
		if ( temp == "on")
		{
			map |= 0x01 << i; 
		}
	}
	vlan->port_bitmap = map;
	vlan->ut_port_bitmap = map;//used for future
	
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	sprintf(buff, "sessionID=%s&cardno=7", getReqValue("sessionID").c_str());
	redirectHTMLFile("/cgi-bin/show_card.cgi", buff);
	return 0;
}

int CVlan::Delvlan()
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	char buff[128];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	std::string vid = getReqValue("vid");
	SW_VLAN *vlan;

	msg_header = (MSG_HEAD_CMD *)send_buff;
	vlan = (SW_VLAN *)(send_buff +  sizeof(MSG_HEAD_CMD ));
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	memset(buff, 0, sizeof(buff));

	msg_header->msg_code = C_SW_DELVLAN;
	msg_header->card_no = CTRL_CARDNO;
	msg_header->length = sizeof(SW_VLAN);
	
	vlan->vlan_id = std::atoi(vid.c_str());
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	sprintf(buff, "sessionID=%s&cardno=7", getReqValue("sessionID").c_str());
	redirectHTMLFile("/cgi-bin/show_card.cgi", buff);
	return 0;

}

void CVlan::getVlanitems(stringstream & outss)
{
	CppSQLite3DB db;
	char 	sql[256];
	char		buff[32];
	int rec_count = 0, pages_total=0;
	int vlanid, ulbitmap, portbitmapt , i = 0 ;

	try {
		db.open(CONF_DB);

		sprintf(sql, "select VlanID, utportbitmap, portbitmap from VLAN order by VlanID" );
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
        	{	
        		
			outss << "<tr>";
			//id
			outss << "<td>" <<(1 + i++) << "</td>";

			vlanid = q.getIntField(0);
			outss << "<td>" << vlanid << "</td>";

			//members
			portbitmapt = q.getIntField(2);
			outss << "<td>" <<  getmembers( portbitmapt ) << "</td>";

			//config
			outss << "<td>" <<  "<a class='pagelink' href='/cgi-bin/vlan.cgi?sessionID=${sessionID}&cardno=7&flag=modify&vid=" << q.getStringField(0) << "&bitmap=" << q.getStringField(2)  << "'>" << multilang::getLabel("modify")<< "</a>" << "&nbsp;&nbsp;&nbsp;&nbsp;";
			outss <<  "<a class='pagelink' href='/cgi-bin/vlan.cgi?sessionID=${sessionID}&cardno=7&flag=delete&vid=" << q.getStringField(0) << "'>" << multilang::getLabel("delete")<< "</a>" << "</td>";

			outss << "</tr>\n";
			q.nextRow();
		}
		
		
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
	
	db.close();
}

void CVlan::output()
{
	CCGISession* cs;
	stringstream outss;
	std::string sid;
	std::string flag = getReqValue("flag");
	int ret;

	// check login session
	sid = getReqValue("sessionID");
    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(sid.c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 

	if ( flag ==  "getvlan" )
	{
		ret = Getvlan();
		if ( ret == 0 )
		{
			// generate vlan items
			getVlanitems(outss);
			
			addUserVariable("vlan_items", outss.str());

			//output with the HTML template
			if( outputHTMLFile(m_desthtml, true) != 0)
			{
				outputError("Cann't open vlan_table.html\n");
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
				outputError("Cann't open vlan.html\n");
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
	else if ( flag == "add" )
	{
		ret = Addvlan();
		if ( 0 == ret )
		{
			//output with the HTML template
			if( outputHTMLFile(m_desthtml, true) != 0)
			{
				outputError("Cann't open vlan_table.html\n");
			}
		}
		else
		{
			outputError(multilang::getError(ret));

		}
		
	}
		

	free(cs);
   
}


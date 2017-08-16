#include <iostream>
#include <stdio.h>
#include <sstream> 
#include <dirent.h>
#include <string.h>

#include "CgiRequest.h"
#include "Tools.h"
#include "Lang.h"
#include "CGISession.h"
#include "CppSQLite3.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"

#include "CL3Config.h"

using namespace std;

void CL3Config::getVLANList(stringstream & outss, int select_id)
{
	CppSQLite3DB db;
	char 	sql[256];
	int vlan_id;

	try {
		db.open(CONF_DB);

		// get the interface name
		sprintf(sql, "select VlanID from VLAN;");
		CppSQLite3Query q = db.execQuery(sql);

		outss << "<select class='fixsize'  name='vlan'>";

		while (!q.eof())
        	{	
        		vlan_id = q.getIntField(0);
				
			outss << " <option value='" << vlan_id;
			if(vlan_id == select_id)
				outss << "' selected>";
			else
				outss << "'>";
			outss << vlan_id << "</option>";
			
			q.nextRow();
		}
		
		outss << "</select>";		
		
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
	
	db.close();

}


int CL3Config::getL3Interface(int intf_id)
{
	CppSQLite3DB db;
	char 	sql[256];
	char 	buff[32];
	int id, vlan, ret;
	stringstream outss;

	ret = ERROR_NOTFOUND;

	try {
		db.open(CONF_DB);

		sprintf(sql, "select id, vlan, ip, mask from L3INTF where id=%d", intf_id );
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
        	{	
        		ret = ERROR_NO;
			
			//id
			id = q.getIntField(0);
			sprintf(buff, "%d", id);
			addUserVariable("intf_id",  buff);

			//vlan
			vlan = q.getIntField(1);

			//ip
			addUserVariable("ip",  q.getStringField(2));

			//mask
			addUserVariable("mask",  q.getStringField(3));
			break;
		}		

		db.close();
		
		getVLANList(outss, vlan);
		addUserVariable("vlan", outss.str());
		
	}
	catch (CppSQLite3Exception& e)
	{
		return(-1);
	}
	
	return(ret);
	
}

void CL3Config::getL3InterfaceList(stringstream & outss)
{
	CppSQLite3DB db;
	char 	sql[256];
	int id, vlan;

	try {
		db.open(CONF_DB);

		sprintf(sql, "select id, vlan, ip, mask from L3INTF order by id" );
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
        	{	
        		
			outss << "<tr>";
			
			//id
			id = q.getIntField(0);
			outss << "<td>" << id << "</td>";

			//vlan
			vlan = q.getIntField(1);
			outss << "<td>" << vlan << "</td>";

			//ip
			outss << "<td>" <<  q.getStringField(2) << "</td>";

			//mask
			outss << "<td>" <<  q.getStringField(3) << "</td>";

			//config
			outss << "<td><input class='btn1' type='button' name='Modify' value='"<< multilang::getLabel("modify")<<"' onclick='SetL3Intf(" << id << ");'>&nbsp;&nbsp;";
			outss << "<input class='btn1' type='button' name='Delete' value='"<< multilang::getLabel("delete") <<"' onclick='DelL3Intf(" << id << ");'></td>";

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

int CL3Config::getAvailabelIntfID()
{
	CppSQLite3DB db;
	char 	sql[256];
	int id, ret;

	ret = 1;

	try {
		db.open(CONF_DB);

		sprintf(sql, "select id from L3INTF order by id");
		CppSQLite3Query q = db.execQuery(sql);

		while (!q.eof())
        	{	
			//id
			id = q.getIntField(0);
			if(ret != id)
				break;
			
			ret++;			
			q.nextRow();
		}		
		
	}
	catch (CppSQLite3Exception& e)
	{
		return(-1);
	}
	
	db.close();
	return(ret);
	
}

int CL3Config::delL3Interface()
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];	
	char buff[128];
	
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	SW_L3INTF *l3intf;
	
	std::string intf_id = getReqValue("listidx");

	msg_header = (MSG_HEAD_CMD *)send_buff;
	l3intf = (SW_L3INTF *)(send_buff +  sizeof(MSG_HEAD_CMD ));
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));

	msg_header->msg_code = C_SW_DELL3INTF;
	msg_header->card_no = CTRL_CARDNO;
	msg_header->length = sizeof(SW_L3INTF);
	
	l3intf->id = std::atoi(intf_id.c_str());
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	sprintf(buff, "sessionID=%s", getReqValue("sessionID").c_str());
	redirectHTMLFile("/cgi-bin/l3config.cgi", buff);
	return 0;

}

int CL3Config::saveL3Interface()
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];	
	char buff[128];
	
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	SW_L3INTF *l3intf;
	
	std::string intf_id = getReqValue("intf_id");
	std::string vlan = getReqValue("vlan");
	std::string ip = getReqValue("ip");
	std::string mask = getReqValue("mask");

	msg_header = (MSG_HEAD_CMD *)send_buff;
	l3intf = (SW_L3INTF *)(send_buff +  sizeof(MSG_HEAD_CMD ));
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));

	msg_header->msg_code = C_SW_SAVEL3INTF;
	msg_header->card_no = CTRL_CARDNO;
	msg_header->length = sizeof(SW_L3INTF);

	l3intf->setFlag = 0xFF;
	l3intf->id = std::atoi(intf_id.c_str());
	l3intf->vlan = std::atoi(vlan.c_str());
	strncpy(l3intf->ip, ip.c_str(), 16);
	strncpy(l3intf->mask, mask.c_str(), 16);
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	sprintf(buff, "sessionID=%s", getReqValue("sessionID").c_str());
	redirectHTMLFile("/cgi-bin/l3config.cgi", buff);
	return 0;

}

void CL3Config::output()
{
	std::string result;
	CCGISession* cs;	
	stringstream outss;
	int ret;
	
	std::string action = getReqValue("action");
    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 
	
	if (action == "new_i")
	{
		//new L3 Interface
		int new_id = getAvailabelIntfID();
		char buff[16];

		if(new_id > 0)
		{
			sprintf(buff, "%d", new_id);
			addUserVariable("intf_id",  buff);
			addUserVariable("mask",  "255.255.255.0");
			
			getVLANList(outss, 0);
			addUserVariable("vlan", outss.str());
			
			if( outputHTMLFile("l3intf.html", true) != 0)
			{
				outputError("Cann't open l3intf.html\n");
			}
		}else
			outputError(multilang::getError(ret));
	}
	else if (action == "get_i")
	{
		//get L3 Interface data for modify
		std::string intf_id = getReqValue("listidx");
		ret =  getL3Interface(std::atoi(intf_id.c_str()));
		if ( ret == ERROR_NO )
		{
			if( outputHTMLFile("l3intf.html", true) != 0)
			{
				outputError("Cann't open l3intf.html\n");
			}	
		}
		else
			outputError(multilang::getError(ret));
		
	} 
	else if (action == "del_i")
	{
		//delete a L3 Interface 
		ret =  delL3Interface();
		if ( ret != ERROR_NO )
			outputError(multilang::getError(ret));
		
	} 
	else if (action == "save_i")
	{
		//save L3 Interface 
		ret =  saveL3Interface();
		if ( ret != ERROR_NO )
			outputError(multilang::getError(ret));
	} 
	else if (action == "list_i") 
	{
		//get L3 Interface list
		getL3InterfaceList(outss);
		addUserVariable("l3intf_items", outss.str());

		//output with the HTML template
		if( outputHTMLFile("l3intf_table.html", true) != 0)
		{
			outputError("Cann't open l3intf_table.html\n");
		}			
	} 
	else
	{
		//get the main l3config page
		if(outputHTMLFile("l3config.html", true) != 0)
			outputError("Cann't open l3config.html\n");
	}

	free(cs);

}

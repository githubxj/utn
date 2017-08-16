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

#include "CMulticast.h"

using namespace std;
#define  PAGE_SIZE 20

const char *CMulticast::getmembers(int map)
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

int CMulticast::GetAllMC()
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	stringstream outss;
	unsigned char gip[4];
	char buff[256];
	int *totalnum , num, sid;
	unsigned int  bitmap, groupip, flag;

	CppSQLite3DB db;
	char 	sql[256];
	int rec_count = 0, pages_total=0;

 	int req_page = atoi(getReqValue("page").c_str());
	if(req_page <= 0)
	{
		//new request, fresh the multicast table
		
		msg_header = (MSG_HEAD_CMD *)send_buff;
		memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
		memset(buff, 0, sizeof(buff));
		memset(gip, 0 , sizeof(gip) );

		msg_header->msg_code = C_SW_GETALLMC;
		msg_header->card_no = CTRL_CARDNO;
		msg_header->length = 0;
		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
		
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		totalnum = ( int *)(recv_buff + sizeof(MSG_HEAD_CMD_RSP ));
		num = (*totalnum);	
		req_page = 1;
	}

	try {
		db.open(DYNAMIC_DB);

		//get the total alarm count;
		CppSQLite3Query q_count = db.execQuery("select count(*) from MULTICAST;");

		if(!q_count.eof())
			rec_count = q_count.getIntField(0);

		pages_total = (rec_count + PAGE_SIZE -1)/PAGE_SIZE;

		sprintf(sql, "select GrpIp, portbitmap, flag from MULTICAST order by GrpIp limit %d, %d;", (req_page-1) * PAGE_SIZE, PAGE_SIZE);
		CppSQLite3Query q = db.execQuery(sql);

		outss << "";
		sid = (req_page-1) * PAGE_SIZE;

		while (!q.eof())
        	{
        		groupip = q.getIntField(0);
        		bitmap= q.getIntField(1);
        		flag = q.getIntField(2);

			outss << "<tr>";
			sid++;
			outss << "<td>" << sid << "</td>";

			gip[0] = groupip & 0xFF;
			gip[1] = (groupip & 0xFF00) >> 8;
			gip[2] = (groupip & 0xFF0000) >> 16;
			gip[3] = (groupip & 0xFF000000) >> 24;
			sprintf(buff, "%d.%d.%d.%d", gip[3], gip[2], gip[1], gip[0] );
			outss << "<td>" << buff << "</td>";


			//members
			sprintf(buff, getmembers(bitmap));
			outss << "<td>" <<  buff << "</td>";

			//config
			if ( 1 == flag )//static
			{
				outss << "<td>" <<  "<a class='pagelink' href='/cgi-bin/multicast.cgi?sessionID=${sessionID}&cardno=7&flag=modify&gip=" << groupip << "&bitmap=" << bitmap  << "'>" << multilang::getLabel("modify")<< "</a>" << "&nbsp;&nbsp;&nbsp;&nbsp;";
				outss <<  "<a class='pagelink' href='/cgi-bin/multicast.cgi?sessionID=${sessionID}&cardno=7&flag=delete&gip=" << groupip << "'>" << multilang::getLabel("delete")<< "</a>" << "</td>";

			}
			else
			{
				outss << "<td></td>";
			}
			
			outss << "</tr>\n";
			q.nextRow();
		}
		
		//generate the navigate info
		if(rec_count > PAGE_SIZE) 
		{
			stringstream nav_outs;
				
			sprintf(buff, "%d/%d", req_page, pages_total);
			addUserVariable("page_info", buff);

			if(req_page > 1)
				nav_outs << "<a class='pagelink' href='#swtabs' onclick='multicast_page(" << req_page-1 << ")'> " << multilang::getLabel("prev") << " </a>&nbsp;";			

			if((req_page + 1) <= pages_total)
				nav_outs << "<a class='pagelink' href='#swtabs' onclick='multicast_page(" << req_page+1 << ")'> " << multilang::getLabel("next") << " </a>&nbsp;";			

			addUserVariable("navigate", nav_outs.str());
			
		}
		
	}
	catch (CppSQLite3Exception& e)
	{
		outss << "Database error!<br>";
		outss << e.errorCode() << ":" << e.errorMessage();
	}
	
	db.close();

	addUserVariable("mc_items", outss.str());

	m_desthtml = "multicast_table.html";

	return 0;


}

int CMulticast::AddMC()
{
	m_desthtml = "multicast.html";
	return 0;
}

int CMulticast::ModifyMC()
{
	
	std::string bitmap = getReqValue("bitmap");
	unsigned int portmap = std::atoi(bitmap.c_str());
	int i , mask ;
	char buff[32],   fieldname[32], ipbuff[32];
	unsigned char grpipadd[4];
	unsigned int grpip ;
	
	memset(buff, 0, sizeof(buff) );
	memset(grpipadd, 0, sizeof(grpipadd) );
	
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
	strcpy(ipbuff, getReqValue("gip").c_str());
	sscanf(ipbuff, "%10u", &grpip );//fix
	grpipadd[0] = grpip & 0xFF;
	grpipadd[1] = (grpip & 0xFF00) >> 8;	
	grpipadd[2] = (grpip & 0xFF0000) >> 16;
	grpipadd[3] = (grpip & 0xFF000000) >> 24;	
	sprintf(fieldname, "%d.%d.%d.%d", grpipadd[3],grpipadd[2],grpipadd[1] ,grpipadd[0]);
	addUserVariable("mc_ip",  fieldname);
	
	m_desthtml = "multicast.html";
	
	return 0;
}

int CMulticast::SaveMC()
{

	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	char buff[128];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	std::string gip = getReqValue("mc_ip");
	std::string temp;
	int map = 0 , i ;
	SW_MC_STATE *mcg;
	unsigned int  grpip0,grpip1 , grpip2, grpip3;

	msg_header = (MSG_HEAD_CMD *)send_buff;
	mcg = (SW_MC_STATE *)(send_buff +  sizeof(MSG_HEAD_CMD ));
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	memset(buff, 0, sizeof(buff));

	msg_header->msg_code = C_SW_SAVEMCG;
	msg_header->card_no = CTRL_CARDNO;
	msg_header->length = sizeof(SW_MC_STATE);
	
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
	
	mcg->bitmap = map ;
	mcg->flag = 1;

	//add group ip
	memset(buff, 0, sizeof(buff));
	strcpy(buff, gip.c_str() );
	sscanf(buff, "%d.%d.%d.%d", &grpip3, &grpip2, &grpip1, &grpip0);
	mcg->grpip = ((grpip3 & 0xFF) << 24) | ((grpip2 & 0xFF) << 16) |((grpip1 & 0xFF) << 8 ) |(grpip0 & 0xFF);
	
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	sprintf(buff, "sessionID=%s&cardno=7", getReqValue("sessionID").c_str());
	redirectHTMLFile("/cgi-bin/show_card.cgi", buff);
	return 0;
}

int CMulticast::DelMC()
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	char buff[128];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	std::string gip = getReqValue("gip");
	SW_MC_STATE *mcg;
	unsigned int groupip;

	msg_header = (MSG_HEAD_CMD *)send_buff;
	mcg = (SW_MC_STATE *)(send_buff +  sizeof(MSG_HEAD_CMD ));
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	memset(buff, 0, sizeof(buff));

	msg_header->msg_code = C_SW_DELMCG;
	msg_header->card_no = CTRL_CARDNO;
	msg_header->length = sizeof(SW_MC_STATE);
	
	//add group ip
	strcpy(buff, gip.c_str() );
	sscanf(buff, "%10u", &groupip);//fix
	mcg->grpip = groupip;
	mcg->bitmap = 0;
	
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	sprintf(buff, "sessionID=%s&cardno=7", getReqValue("sessionID").c_str());
	redirectHTMLFile("/cgi-bin/show_card.cgi", buff);
	return 0;

}



void CMulticast::output()
{
	CCGISession* cs;
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

	if ( flag ==  "getamc" )
	{
		ret = GetAllMC();
		if ( ret == 0 )
		{

			//output with the HTML template
			if( outputHTMLFile(m_desthtml, true) != 0)
			{
				outputError("Cann't open multicast_table.html\n");
			}
		}
		else
		{
			outputError(multilang::getError(ret));
		}
			
	}
	else if ( flag == "modify" )
	{
		ret = ModifyMC();
		if ( 0 == ret )
		{
			//output with the HTML template
			if( outputHTMLFile(m_desthtml, true) != 0)
			{
				outputError("Cann't open multicast.html\n");
			}
		}
		
		
	}
	else if ( flag == "save")
	{
		ret = SaveMC();//success redirect
		if ( 0 != ret )
		{
			outputError(multilang::getError(ret));
		}
		
	}
	else if ( flag == "delete" )
	{
		ret = DelMC();//success redirect
		if ( 0 != ret )
		{
			outputError(multilang::getError(ret));
		}
	}
	else if ( flag == "add" )
	{
		ret = AddMC();
		if ( 0 == ret )
		{
			//output with the HTML template
			if( outputHTMLFile(m_desthtml, true) != 0)
			{
				outputError("Cann't open multicast.html\n");
			}
		}
		
	}
		

	free(cs);
   
}


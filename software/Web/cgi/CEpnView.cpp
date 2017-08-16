#include <iostream>
#include <sstream> 
#include <vector>
#include <stdio.h>
#include <string.h>

#include "CGISession.h"
#include "const.h"
#include "Tools.h"
#include "Lang.h"
#include "CgiRequest.h"
#include "CppSQLite3.h"
#include "msg.h"
#include "Comm.h"

#include "CEpnView.h"

#define  PAGE_SIZE 20

using namespace std;

int CEpnView::getEponuList(int pon)
{
	char sql[256]; 
	char		buff[32];
	CppSQLite3DB db;
	int id, dist;
	int req_page, rec_count = 0, pages_total=0;

	stringstream outss, navi;

	try {
		db.open(DYNAMIC_DB);
	
		//get the total  count;
		sprintf(sql, "select count(*) from EPONUList where CardNo=%d and Pon=%d;", m_card_no, pon);
		CppSQLite3Query q_count = db.execQuery(sql);

		if(!q_count.eof())
			rec_count = q_count.getIntField(0);

		req_page = atoi(getReqValue("page").c_str());
		pages_total = (rec_count + PAGE_SIZE -1)/PAGE_SIZE;
		
		sprintf(sql, "select NodeId, MAC, ONUName, Distance, NodeState from EPONUList where CardNo=%d and Pon=%d order by NodeId limit %d, %d;", m_card_no, pon, req_page * PAGE_SIZE, PAGE_SIZE);
		CppSQLite3Query q = db.execQuery(sql);
		while (!q.eof())
        	{
			outss << "<tr>";
			
			//id
			id = q.getIntField(0);
			outss << "<td>" << id << "</td>";

			//MAC
			outss << "<td>" <<  q.getStringField(1) << "</td>";

			//Name
			outss << "<td>" <<  q.getStringField(2) << "</td>";
			
			//distance
			dist = q.getIntField(3);
			outss << "<td>" << dist/10.0 << " km</td>";
			
			//state
			if(q.getIntField(4) == 1)
				outss << "<td> Online</td>";
			else
				outss << "<td> </td>";
				

			//Link
			outss << "<td>" <<  "<a class='pagelink' href='/cgi-bin/show_eponu.cgi?sessionID=${sessionID}&cardno=" << m_card_no 
				<< "&pon=" << pon << "&onunode=" << id << "'>" << multilang::getLabel("view")<< "</a></td>";

			outss << "</tr>\n";
			q.nextRow();
		}



	}
	catch (CppSQLite3Exception& e)
	{
		return(ERROR_DATABASE);
	}
	
	db.close();
	addUserVariable("eponu_items_o", outss.str());

	//generate the navigate info
	if(rec_count > PAGE_SIZE) 
	{
		sprintf(buff, "%d/%d", req_page + 1, pages_total);
		addUserVariable("page_info", buff);

		if(req_page != 0)
			navi << "<a class='pagelink' href='/cgi-bin/epnview.cgi?sessionID="<< getReqValue("sessionID") << "&cardno=" << m_card_no << "&pon=" << pon << "&page=" << req_page - 1<< "'> " << multilang::getLabel("prev") << " </a>&nbsp;";			

		if((req_page + 1) != pages_total)
			navi << "<a class='pagelink' href='/cgi-bin/epnview.cgi?sessionID="<< getReqValue("sessionID") << "&cardno=" << m_card_no << "&pon=" << pon << "&page=" << req_page + 1 << "'> " << multilang::getLabel("next") << " </a>&nbsp;";			

		addUserVariable("navigate", navi.str());
			
	}
	return 0;

}


int CEpnView::delOfflineEponu(int pon)
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	char mac_str[32];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	EPONU_ONU_REQ	*req;

	msg_header = (MSG_HEAD_CMD *)send_buff;
	req = (EPONU_ONU_REQ *)(send_buff +  sizeof(MSG_HEAD_CMD ));
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));

	msg_header->msg_code = C_EP_DELONU_OFFLINE;
	msg_header->card_no = m_card_no;
	msg_header->length = sizeof(EPONU_ONU_REQ);
	
	req->pon = pon;
	req->node_id = 0;
	
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;

	return(msgrsp_header->rsp_code);
}

void CEpnView::output()
{
	CCGISession* cs;
	stringstream outss;	
	std::string desthtml;
	int pon_id;
	int ret=0;

    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 

	std::string card_no = getReqValue("cardno");
	m_card_no = std::atoi(card_no.c_str());
	std::string pon_str = getReqValue("pon");
	pon_id = std::atoi(pon_str.c_str());

	char buff[16];
	sprintf(buff,"%u", pon_id + 1);
	addUserVariable("pon_no", buff);
	desthtml = "epnview.html";	

	std::string type = getReqValue("type");

	if(type == "del")
	{
		ret = delOfflineEponu(pon_id);		
		outputError(multilang::getError(ret));
	} else {
		ret = getEponuList(pon_id);
		if(ret != 0)
		{
			outputError(multilang::getError(ret));
		}
		else if( outputHTMLFile(desthtml, true) != 0)
		{
			outputError("Cann't open eponuview.html\n");
		}	
	}


	free(cs);
   
}


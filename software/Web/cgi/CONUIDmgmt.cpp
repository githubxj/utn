#include <iostream>
#include <sstream> 
#include <stdio.h>
#include <string.h>

#include "const.h"
#include "msg.h"
#include "Lang.h"
#include "Comm.h"
#include "CgiRequest.h"
#include "CppSQLite3.h"
#include "CGISession.h"

#include "CONUIDmgmt.h"

using namespace std;
const char cardname_fsa[] = "FSA1250";
const char cardname_fsa_covond[] = "MAT1000";
const char cardname_dat[] = "DAT1000";

int CONUIDmgmt::getFSAList()
{
	stringstream  outss;	
	CppSQLite3DB db;
	char sql[256]; 
	int card_no, card_type;

	outss << " ";
	try {

		db.open(DYNAMIC_DB);

		sprintf(sql, "select CardNo, CardType from Card where CardType=%d or CardType=%d order by CardNo;", CARD_TYPE_FSA, CARD_TYPE_DAT);
		CppSQLite3Query q = db.execQuery(sql);
		while (!q.eof())
        	{
         		card_no = q.getIntField(0);
			card_type = q.getIntField(1);
			
       		outss << "<option value='" << card_no << " ' > ";
			if(card_type == CARD_TYPE_FSA)
			{
#ifdef COVOND
				outss <<  cardname_fsa_covond << "-" << card_no;
#else
				outss <<  cardname_fsa << "-" << card_no;				
#endif
			}
			else
				outss <<  cardname_dat << "-" << card_no;
				
			outss <<"</option>\n";

			q.nextRow();
		}
	}
	catch (CppSQLite3Exception& e)
	{
		return(-1);
	}
	
	db.close();

	addUserVariable("fsa_list", outss.str());
	
	m_desthtml = "onuid_mgmt.html";

	return 0;

}

int CONUIDmgmt::getONUList()
{
	stringstream  outss;	
	CppSQLite3DB db;
	char sql[256]; 

	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	char * base_dsn_ptr;	

	std::string card_no = getReqValue("cardno");

	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	msg_header->msg_code = C_ONU_GETDSN_LIST;

	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	base_dsn_ptr = recv_buff + sizeof(MSG_HEAD_CMD_RSP);

	outss << " ";
	try {
		unsigned char		node_state;
		int				node_sn;
		int				node_id;
		char 			buffer[32];
		
		db.open(DYNAMIC_DB);
	
		sprintf(sql, "select NodeState, NodeSN, NodeId, ONUName from ONUList where CardNo=%d order by  NodeSN;", msg_header->card_no);
		CppSQLite3Query q = db.execQuery(sql);
		while (!q.eof())
        	{
        		node_state = q.getIntField(0);
        		node_sn = q.getIntField(1);
        		node_id = q.getIntField(2);
			if( (node_id > MAX_ONU_NUM) || (node_id == 0)
				|| ((node_state != ONU_STATE_ONLINEE) && (node_state != ONU_STATE_ONLINED)) )
			{
				q.nextRow();
				continue;
			}

			outss << "<tr>";
			outss << "<td align='center'>" << node_sn <<"</td>";
			outss << "<td align='center'>" << node_id <<"</td>";		

			//Onu name
			strncpy(buffer, q.getStringField(3), MAX_DISPLAYNAME_LEN);
			buffer[MAX_DISPLAYNAME_LEN] = 0;
			outss << "<td>" << buffer <<"</td>";			

			//ONU DSN
			strncpy(buffer, base_dsn_ptr + (node_id-1)*MAX_ONUDSN_LEN, MAX_ONUDSN_LEN);
			buffer[MAX_ONUDSN_LEN] = 0;
			outss << "<td>" << buffer  <<"<input type='hidden' name='dsn" << node_id << "' id='dsn" << node_id <<  "' value='"<< buffer <<"'>" << "</td>";	

			//new id
			outss << "<td align='right'><input type='text' name='new_id" << node_id << "' id='new_id" << node_id << "' size=1 maxlength=2></td>";

			//submit
			outss << "<td><input class='btn1' type='button' name='Submit' value='"<< multilang::getLabel("submit") <<"' onclick='SetOnuID(" << node_id << ");'></td>";
			outss << "<td><input class='btn1' type='button' name='Reset' value='"<< multilang::getLabel("reset") <<"' onclick='Reset(" << node_id << ");'></td>";
			outss << "</tr>";
			q.nextRow();
		}

	}
	catch (CppSQLite3Exception& e)
	{
		return(-1);
	}
	
	db.close();
	
	addUserVariable("onu_list", outss.str());
	
	m_desthtml = "onuid_list.html";

	return 0;
	
}

int CONUIDmgmt::setID()
{
	stringstream  outss;	

	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	ONU_SETID * onu_setid;

	std::string card_no = getReqValue("cardno");
	std::string node_id = getReqValue("onunode");
	std::string new_id = getReqValue("newid");

	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	onu_setid = (ONU_SETID *) (send_buff + sizeof(MSG_HEAD_CMD));
	
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	msg_header->msg_code = C_ONU_SET_ONUID;
	msg_header->node_id = (unsigned char) std::atoi(node_id.c_str());
	msg_header->length = sizeof(ONU_SETID);

	onu_setid->new_onuid = (unsigned char) std::atoi(new_id.c_str());
	strncpy(onu_setid->onu_dsn, getReqValue("dsn").c_str(), MAX_ONUDSN_LEN);

	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

//	m_desthtml = "result_ok.html";

	return 0;
}

int CONUIDmgmt::resetOnu()
{
	stringstream  outss;	

	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;

	std::string card_no = getReqValue("cardno");
	std::string node_id = getReqValue("onunode");

	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	msg_header->msg_code = C_ONU_RESET;
	msg_header->node_id = (unsigned char) std::atoi(node_id.c_str());
	msg_header->length = 0;

	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	return 0;
}


void CONUIDmgmt::output()
{
	CCGISession* cs;
	int ret;

    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 
	
	std::string action = getReqValue("action");

	if(action == "list")
		ret = getONUList();
	else if ( action == "set" )
	{
		ret = setID();
		if(ret == 0 )
		{
			std::cout << "Content-type:text/html\r\n\r\n";
			std::cout << "0\r\n";
		}
		else 
		{
			std::cout << "Content-type:text/html\r\n\r\n";
			std::cout << multilang::getError(ret);
		}

		free(cs);
		return;
		
	}
	else if ( action == "reset" )
	{
		ret = resetOnu();
		if(ret == 0 )
		{
			std::cout << "Content-type:text/html\r\n\r\n";
			std::cout << "0\r\n";
		}
		else 
		{
			std::cout << "Content-type:text/html\r\n\r\n";
			std::cout << multilang::getError(ret);
		}

		free(cs);
		return;
		
	}
	else
		ret = getFSAList();

	if(ret == 0) {
		if(outputHTMLFile(m_desthtml, true) != 0)
		{  
			outputError("Cann't open html files\n");
		}	
	} 
	else 
		outputError(multilang::getError(ret));

	free(cs);
	
}



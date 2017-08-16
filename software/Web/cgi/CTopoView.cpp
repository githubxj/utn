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

#include "CTopoView.h"

using namespace std;

void CTopoView::genOLT()
{
	stringstream  outss;	

	outss << "<tr><td></td><td class=\"vl\">";
	if(link_end1 == 0)
		outss << "<IMG src=\"/images/redx.gif\">";
	outss << "</td></tr>";
	
	outss <<  "<tr><td>OLT</td><td align=\"center\"><IMG src=\"/images/olt.gif\"></td></tr>";

	outss << "<tr><td></td><td class=\"vl\">";
	if(link_end2 == 0)
		outss << "<IMG src=\"/images/redx.gif\">";
	outss << "</td></tr>";
		
	addUserVariable("olt_content", outss.str());
}

void CTopoView::genTopList(int number, int padding)
{
	stringstream idoutss, outss;	
	int l1_broken_flag=0;
	int ring_error1_flag =  0;

	for(int i=0; i< padding/2; i++) 
	{
		idoutss << "<td></td>" << std::endl;
		outss << "<td class=\"hl-fit\"></td>" << std::endl;
	}

	for(int i=0; i<number; i++)
	{
		idoutss << "<td></td>" << std::endl;
		idoutss << "<td>" << (int) m_onu_list[i].node_id << "</td>" << std::endl;

		if( i == 0 )
			outss << "<td class=\"hl-fit\">";
		else
			outss << "<td class=\"hl\">";
		
		if(ring_error1_flag ||(ring_error2 ==  m_onu_list[i].node_id))
		{
			outss <<"<IMG src=\"/images/redq.gif\">";
			ring_error1_flag = 0;
		}
		else
		{
			if(link_end2 ==  m_onu_list[i].node_id)
				outss <<"<IMG src=\"/images/redx.gif\">";
				
			if(l1_broken_flag && (m_onu_list[i].node_state == ONU_STATE_OFFLINE))
				outss <<"<IMG src=\"/images/redx.gif\">";
		}
		outss << "</td>" << std::endl;

		outss << "<td class=\"node-h\">";
		genNode(outss, m_onu_list[i]);
		outss << "</td>" << std::endl;

		if(link_end1 ==  m_onu_list[i].node_id)
			l1_broken_flag = 1;
		else
			l1_broken_flag = 0;
		
		if(ring_error1 ==  m_onu_list[i].node_id)
			ring_error1_flag = 1;

	}

	idoutss << "<td>&nbsp;</td>" << std::endl;
	
	if((number > 0) && ring_error1_flag)
		outss << "<td class=\"hl-fit\"><IMG src=\"/images/redq.gif\"></td>" << std::endl;
	else if((number > 0) && (link_end1 ==  m_onu_list[number-1].node_id))
		outss << "<td class=\"hl-fit\"><IMG src=\"/images/redx.gif\"></td>" << std::endl;
	else
		outss << "<td class=\"hl-fit\"></td>" << std::endl;

	for(int i=0; i< (padding+1)/2; i++) 
	{
		idoutss << "<td></td>" << std::endl;
		outss << "<td class=\"hl-fit\"></td>" << std::endl;
	}

	addUserVariable("top_nodeid_list", idoutss.str());
	addUserVariable("top_node_list", outss.str());
}

void CTopoView::genBottomList(int number, int offset, int padding)
{
	stringstream idoutss, outss;
	int l2_broken_flag=0;
	int ring_error2_flag =  0;

	for(int i=0; i< padding/2; i++) 
	{
		idoutss << "<td></td>" << std::endl;
		outss << "<td class=\"hl-fit\"></td>" << std::endl;
	}

	int start_pos = m_onu_number - offset -1;
	
	for(int i=start_pos ; i> start_pos - number; i--)
	{
		idoutss << "<td></td>" << std::endl;
		idoutss << "<td>" <<(int)m_onu_list[i].node_id << "</td>" << std::endl;

		if( i == start_pos )
			outss << "<td class=\"hl-fit\">";
		else
			outss << "<td class=\"hl\">";

		if(ring_error2_flag || (ring_error1 ==  m_onu_list[i].node_id))
		{
			outss <<"<IMG src=\"/images/redq.gif\">";
			ring_error2_flag = 0;
		}
		else
		{
			if(link_end1 ==  m_onu_list[i].node_id)
				outss <<"<IMG src=\"/images/redx.gif\">";

			if(l2_broken_flag && (m_onu_list[i].node_state == ONU_STATE_OFFLINE))
				outss <<"<IMG src=\"/images/redx.gif\">";
		}
		
		outss << "</td>" << std::endl;

		outss << "<td class=\"node-h\">";
		genNode(outss, m_onu_list[i]);
		outss << "</td>" << std::endl;

		if(link_end2 ==  m_onu_list[i].node_id)
			l2_broken_flag = 1;
		else
			l2_broken_flag = 0;
		
		if(ring_error2 ==  m_onu_list[i].node_id)
			ring_error2_flag = 1;
	}

	idoutss << "<td></td>" << std::endl;
	
	if((number > 0) && ring_error2_flag)
		outss << "<td class=\"hl-fit\"><IMG src=\"/images/redq.gif\"></td>" << std::endl;
	if((number > 0) && (link_end2 ==  m_onu_list[start_pos - number + 1].node_id))
		outss << "<td class=\"hl-fit\"><IMG src=\"/images/redx.gif\"></td>" << std::endl;
	else
		outss << "<td class=\"hl-fit\"></td>" << std::endl;

	for(int i=0; i< (padding+1)/2; i++) 
	{
		idoutss << "<td></td>" << std::endl;
		outss << "<td class=\"hl-fit\"></td>" << std::endl;
	}
	
	addUserVariable("bottom_nodeid_list", idoutss.str());
	addUserVariable("bottom_node_list", outss.str());

}

void CTopoView::genLeftList(int number, int padding)
{
	stringstream outss;	
	int l2_broken_flag=0;

	for(int i=0; i< (padding+1)/2; i++) 
	{
		outss<< "<tr><td></td>" << std::endl;
		outss<< "<td class=\"vl-fit\"></td></tr>" << std::endl;
	}

	int start_pos = m_onu_number -1;
	
	for(int i=start_pos ; i>= m_onu_number - number; i--)
	{

		if( i != start_pos )
		{
			outss<< "<tr><td></td>" << std::endl;
			outss<< "<td class=\"vl\">";

			if((ring_error1 ==  m_onu_list[i].node_id) || ( (i<MAX_ONU_NUM-1) &&(ring_error2 ==  m_onu_list[i+1].node_id)))
				outss <<"<IMG src=\"/images/redq.gif\">";
			else
			{
				if(link_end1 ==  m_onu_list[i].node_id)
					outss <<"<IMG src=\"/images/redx.gif\">";
				
				if(l2_broken_flag && (m_onu_list[i].node_state == ONU_STATE_OFFLINE))
					outss <<"<IMG src=\"/images/redx.gif\">";
			}
			
			outss << "</td></tr>" << std::endl;

		}

		outss << "<tr><td>" << (int) m_onu_list[i].node_id << "</td>" << std::endl;
		outss << "<td>";
		genNode(outss, m_onu_list[i]);
		outss << "</td></tr>" << std::endl;
		
		if(link_end2 ==  m_onu_list[i].node_id)
			l2_broken_flag = 1;
		else
			l2_broken_flag = 0;
	}
	
	outss<< "<tr><td></td><td class=\"vl\">";
	if((number > 0) &&(ring_error2 ==  m_onu_list[m_onu_number - number].node_id))
		outss <<"<IMG src=\"/images/redq.gif\">";
	if((number > 0) &&(link_end2 ==  m_onu_list[m_onu_number - number].node_id))
		outss <<"<IMG src=\"/images/redx.gif\">";
	outss<< "</td></tr>" << std::endl;

	for(int i=0; i< padding/2; i++) 
	{
		outss<< "<tr><td></td>" << std::endl;
		outss<< "<td class=\"vl-fit\"></td></tr>" << std::endl;
	}
	
	addUserVariable("left_node_list", outss.str());

}

void CTopoView::genRightList(int number, int padding)
{
	stringstream outss;	
	int l1_broken_flag=0;

	for(int i=0; i< padding/2; i++) 
	{
		outss<< "<tr><td class=\"vl-fit\"></td>" << std::endl;
		outss<< "<td></td></tr>" << std::endl;
	}

	for(int i=8 ; i< 8 + number; i++)
	{
		if( i != 8 ) 
			outss<< "<tr><td class=\"vl\">" ;
		else
			outss<< "<tr><td class=\"vl-fit\">" ;

		if((ring_error2 ==  m_onu_list[i].node_id) ||((i!=8)&& (ring_error1 ==  m_onu_list[i-1].node_id )))
			outss <<"<IMG src=\"/images/redq.gif\">";
		else
		{
			if(link_end2 ==  m_onu_list[i].node_id)
				outss <<"<IMG src=\"/images/redx.gif\">";
			
			if(l1_broken_flag && (m_onu_list[i].node_state == ONU_STATE_OFFLINE))
				outss <<"<IMG src=\"/images/redx.gif\">";
		}
		
		outss<< "</td><td></td></tr>" << std::endl;
		
		outss << "<tr><td>";
		genNode(outss, m_onu_list[i]);
		outss << "</td>" << std::endl;
		outss << "<td>" << (int) m_onu_list[i].node_id << "</td></tr>" << std::endl;
		
		if(link_end1 ==  m_onu_list[i].node_id)
			l1_broken_flag = 1;
		else
			l1_broken_flag = 0;
	}
	
	outss<< "<tr><td class=\"vl-fit\">";
	if((number > 0) &&(ring_error1 ==  m_onu_list[8 + number -1].node_id))
		outss <<"<IMG src=\"/images/redq.gif\">";
	else if((number > 0) &&(link_end1 ==  m_onu_list[8 + number -1].node_id))
		outss <<"<IMG src=\"/images/redx.gif\">";
	outss<< "</td><td></td></tr>" << std::endl;
	
	for(int i=0; i< (padding+1)/2 + 1; i++) 
	{
		outss<< "<tr><td class=\"vl-fit\"></td>" << std::endl;
		outss<< "<td></td></tr>" << std::endl;
	}

	addUserVariable("right_node_list", outss.str());

}

void CTopoView::getONUList(int card_no)
{
	char sql[256]; 
	CppSQLite3DB db;
  	ONU_LIST_ENTRY 	offline_onu_list[MAX_ONU_NUM];
	int	i, offline_onu_no=0;

	try {
		db.open(DYNAMIC_DB);

		sprintf(sql, "select CardType from Card where CardNo=%d;", card_no);
		CppSQLite3Query q_card = db.execQuery(sql);
		if (!q_card.eof())
        	{
			m_card_type = q_card.getIntField(0);
		}

		m_onu_number = 0;
		sprintf(sql, "select NodeId, NodeSN, NodeState,  NetworkState, ONUName from ONUList where CardNo=%d order by  NodeSN;", card_no);
		CppSQLite3Query q = db.execQuery(sql);
		while (!q.eof())
        	{
        		m_onu_list[m_onu_number].node_id = q.getIntField(0);
			m_onu_list[m_onu_number].node_sn = q.getIntField(1);
			m_onu_list[m_onu_number].node_state = q.getIntField(2);
			m_onu_list[m_onu_number].network_state = q.getIntField(3);
			strncpy(m_onu_list[m_onu_number].onu_name, q.getStringField(4), MAX_DISPLAYNAME_LEN);
			
			if((m_onu_list[m_onu_number].node_state == ONU_STATE_ONLINEE) || (m_onu_list[m_onu_number].node_state == ONU_STATE_ONLINED) )
				m_onu_number++;
			else if(m_onu_list[m_onu_number].node_state == ONU_STATE_OFFLINE)
			{
				if(offline_onu_no < MAX_ONU_NUM)
				{
					offline_onu_list[offline_onu_no] = m_onu_list[m_onu_number];
					offline_onu_no++;
				}
			}

			if(m_onu_number >= MAX_ONU_NUM)
				break;
			q.nextRow();
		}

		unsigned char maxSN =0;
		unsigned char minSN =0xFF;
		int broken_pos=-1;
		int ring_flag = 0;
		for(i=0; i<m_onu_number; i++)
		{
			m_debug << "ONU: id=" << (int) m_onu_list[i].node_id << " net_state=" << (int) m_onu_list[i].network_state  << "<br>" << endl;
			if(( m_onu_list[i].network_state == ONU_NET_NEAREND)
				&& (m_onu_list[i].node_sn > maxSN))
			{
				maxSN = m_onu_list[i].node_sn;
				link_end1 =  m_onu_list[i].node_id;	
				broken_pos=i;
			}	
			
			if(( m_onu_list[i].network_state == ONU_NET_FAREND)
				&& (m_onu_list[i].node_sn < minSN))
			{
				minSN = m_onu_list[i].node_sn;
				link_end2 =  m_onu_list[i].node_id;	
			}

			if(m_onu_list[i].network_state == ONU_NET_RING)
				ring_flag = 1;

		}

		if(ring_flag)
		{
			maxSN = 0;
			minSN =0xFF;
			for(i=0; i<m_onu_number; i++)
			{
				if(( m_onu_list[i].network_state == ONU_NET_RING_O1D)
					&& (m_onu_list[i].node_sn > maxSN))
				{
					maxSN = m_onu_list[i].node_sn;
					ring_error1 = m_onu_list[i].node_id;
				}	
				
				if(( m_onu_list[i].network_state == ONU_NET_RING_O2D)
					&& (m_onu_list[i].node_sn < minSN))
				{
					minSN = m_onu_list[i].node_sn;
					ring_error2 = m_onu_list[i].node_id;
				}
			}
			m_debug << "ring_error1 =" << ring_error1 << "  ring_error2=" << ring_error2  << "<br>" << endl;
		}
		
		if(m_onu_number == 0)
		{
			link_end1 = 0;
			link_end2 = 0;
		}
		
		if((link_end1 != -1) && (link_end2 == -1))
				link_end2 = 0;
		
		if((link_end2 != -1) && (link_end1 == -1))
				link_end1 = 0;

		// insert the offline nodes into the onu list
		if((offline_onu_no > 0) && (m_onu_number < MAX_ONU_NUM))
		{
			if( (offline_onu_no + m_onu_number) > MAX_ONU_NUM)
				offline_onu_no = MAX_ONU_NUM - m_onu_number;

			for(i = m_onu_number-1; i>broken_pos; i--)
			{
				m_debug << "move" << i << "to" << offline_onu_no + i << "<br>";
				m_onu_list[offline_onu_no + i ] = m_onu_list[i];
			}

			for(i=0; i<offline_onu_no; i++)
			{
				m_debug << "move offline id:" << (int) offline_onu_list[i].node_id << "to" << broken_pos+1+ i<< "<br>";				
				m_onu_list[broken_pos+1+ i] = offline_onu_list[i];
			}
			m_onu_number += offline_onu_no;
		}
		m_debug << "link_end1=" << link_end1 << " link_end2=" << link_end2  << "broken=" << broken_pos << "<br>" << endl;

	}
	catch (CppSQLite3Exception& e)
	{
		return;
	}
	
	db.close();


	/*char send_buff[MAX_CMDBUFF_SIZE];//this code for getonulist  real time,
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	ONU_LIST_ENTRY *ponu;
	int onu_index;
	
	
	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset( msg_header, 0, sizeof( MSG_HEAD_CMD));

	msg_header->card_no = card_no;
	msg_header->msg_code = C_ONU_GETLIST;
	msg_header->length = 0;

	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	if(msgrsp_header->length != sizeof(ONU_LIST_ENTRY))
		return(ERROR_COMM_PARSE);

	ponu = (ONU_LIST_ENTRY *)(sizeof(MSG_HEAD_CMD_RSP)+recv_buff);

	for ( onu_index=0; onu_index < MAX_ONU_NUM; onu_index++)
	{
		
		memcpy(m_onu_list + onu_index, ponu + onu_index, sizeof(ONU_LIST_ENTRY));
		m_onu_number++;
		if(m_onu_number >= MAX_ONU_NUM)
				break;
		
	}*/
	

	

}
	
void CTopoView::output()
{
	CCGISession* cs;
	stringstream outss;	

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

	getONUList(m_card_no);

	int tNo,bNo,lNo,rNo;
	int tPadding=0, bPadding=0, lPadding=0, rPadding=0;

	if(m_onu_number<=16) 
	{
		tNo = (m_onu_number + 1)/2;
		bNo = m_onu_number/2;

		if(tNo < 3)
		{
			tPadding = 2 * (3-tNo);
			bPadding = tPadding;
		}
		
		if(tNo != bNo)
			bPadding += 3;
		
		lNo = 0;
		rNo =0;
		rPadding = 7;
	} else {
		tNo = 8;
		bNo = 8;
		switch(m_onu_number) {
			case 17:
				rPadding = 4;
				lNo = 0;
				rNo = 1;
				break;
			case 18:
				lNo = 0;
				rNo = 2;
				break;
			case 19:
				lPadding = 4;
				lNo = 0;
				rNo = 3;
				break;
			case 20:
				lPadding = 2;
				lNo = 1;
				rNo = 3;
				break;
			case 21:
				rPadding = 2;
				lNo = 2;
				rNo = 3;
				break;
			case 22:
				lPadding = 2;
				lNo = 2;
				rNo = 4;
				break;
			case 23:
				rPadding = 2;
				lNo = 3;
				rNo = 4;
				break;
			case 24:
			default:
				lPadding = 2;
				lNo = 3;
				rNo = 5;
				break;
		}
	}

	genOLT();
	genTopList(tNo, tPadding);
  	genBottomList(bNo, lNo, bPadding);
  	genLeftList(lNo, lPadding);
  	genRightList(rNo, rPadding);
	
	addUserVariable("debug_info", m_debug.str());

	if(m_card_type == CARD_TYPE_DAT)
		addUserVariable("fsa_name", "DAT1000");
	else	
#ifdef COVOND
		addUserVariable("fsa_name", "MAT1000");
#else
		addUserVariable("fsa_name", "FSA1250");
#endif

	if( outputHTMLFile("topo_view.html", true) != 0)
	{
		outputError("Cann't open topo_view.html\n");
	}	

	free(cs);
   
}


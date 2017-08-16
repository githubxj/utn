#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <netinet/in.h>

#include "CgiRequest.h"
#include "CGISession.h"
#include "Lang.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"
#include "CCount.h"

using namespace std;

int CGetCount::getCardCount()//for all  onu and sw control card 
{

		std::string card_no = getReqValue("cardno");
		std::string action = getReqValue("action");
			
		char send_buff[MAX_CMDBUFF_SIZE];
		char recv_buff[MAX_CMDBUFF_SIZE];
		char buff[256];
		MSG_HEAD_CMD *msg_header;
		MSG_HEAD_CMD_RSP *msgrsp_header;
		PORT_COUNT *portcount;
		SW_PORT_COUNT *swcount;
		int i;
		char field[32];
		
		msg_header = (MSG_HEAD_CMD *)send_buff;
		memset(msg_header, 0, sizeof(MSG_HEAD_CMD));

		addUserVariable("card_no", card_no);
		if ( action == "reset")
		{
			msg_header->msg_code = C_FSA_RESETCOUNT;
		}
		else if ( action == "swget" )
		{
			msg_header->msg_code = C_SW_GETCOUNT;
		}
		else if ( action == "swreset" )
		{
			msg_header->msg_code = C_SW_RESETCOUNT;
		}
		else
		{ 
			msg_header->msg_code = C_FSA_GETCOUNT;   
		}
			
		
		msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());	
		msg_header->node_id = 0;
		msg_header->length = 0;

		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);
		
		if ( action == "reset")
		{
			sprintf(buff, "sessionID=%s&cardno=%s", getReqValue("sessionID").c_str(), getReqValue("cardno").c_str());
			redirectHTMLFile("/cgi-bin/countview.cgi", buff);
			return (-1);
		}
		else if ( action == "swreset" )
		{
			sprintf(buff, "sessionID=%s&cardno=%s&action=swget", getReqValue("sessionID").c_str(), getReqValue("cardno").c_str());
			redirectHTMLFile("/cgi-bin/countview.cgi", buff);
			return (-1);
		}
		else if ( action == "swget" )
		{
			if (msgrsp_header->length != sizeof(SW_PORT_COUNT))
				return (ERROR_COMM_PARSE);

			swcount = (SW_PORT_COUNT *)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));

			for ( i = 0; i < 26; i++ )
			{
				if ( (i == 24) || (i == 25) )
				{
					sprintf(buff,"%u",swcount->recvbytecount[i] );
					sprintf(field, "recvbyte_ge%d", i-23 );
					addUserVariable(field, buff);
					
					sprintf(buff,"%u",swcount->recvpktcount[i] );
					sprintf(field, "recvpkt_ge%d", i-23 );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->recvMCpktcount[i] );
					sprintf(field, "recvmcpkt_ge%d", i-23 );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->recvBCpktcount[i] );
					sprintf(field, "recvbcpkt_ge%d", i-23 );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->tranbytecount[i] );
					sprintf(field, "tranbyte_ge%d", i-23 );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->tranpktcount[i] );
					sprintf(field, "tranpkt_ge%d", i-23 );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->tranMCpktcount[i] );
					sprintf(field, "tranmcpkt_ge%d", i-23 );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->tranBCpktcount[i] );
					sprintf(field, "tranbcpkt_ge%d", i-23 );
					addUserVariable(field, buff);

				}
				else
				{
					sprintf(buff,"%u",swcount->recvbytecount[i] );
					sprintf(field, "recvbyte_fe%d", i + 1 );
					addUserVariable(field, buff);
					
					sprintf(buff,"%u",swcount->recvpktcount[i] );
					sprintf(field, "recvpkt_fe%d", i + 1  );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->recvMCpktcount[i] );
					sprintf(field, "recvmcpkt_fe%d", i + 1  );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->recvBCpktcount[i] );
					sprintf(field, "recvbcpkt_fe%d", i + 1  );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->tranbytecount[i] );
					sprintf(field, "tranbyte_fe%d", i + 1  );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->tranpktcount[i] );
					sprintf(field, "tranpkt_fe%d", i + 1  );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->tranMCpktcount[i] );
					sprintf(field, "tranmcpkt_fe%d", i + 1  );
					addUserVariable(field, buff);

					sprintf(buff,"%u",swcount->tranBCpktcount[i] );
					sprintf(field, "tranbcpkt_fe%d", i + 1  );
					addUserVariable(field, buff);

				}
			}

			m_desthtml = "swcount.html";
		}
		else 
		{
			if(msgrsp_header->length != sizeof(PORT_COUNT))
				return(ERROR_COMM_PARSE);

			portcount = (PORT_COUNT *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));
			addUserVariable("count1_name", multilang::getstatisticname(portcount->countType, 1));
			addUserVariable("count2_name", multilang::getstatisticname(portcount->countType, 2));
			for ( i = 0; i < 26; i++ )//fix?
			{
				if ( (i == 24) || (i == 25) )
				{
					sprintf(buff,"%u", ntohl(portcount->portCount1[i]));
					sprintf(field, "ge%d_count1", i-23 );
					addUserVariable(field, buff);
					
					sprintf(buff,"%u", ntohl(portcount->portCount2[i]));
					sprintf(field, "ge%d_count2", i-23 );
					addUserVariable(field , buff);
				}
				else
				{
					sprintf(buff,"%u", ntohl(portcount->portCount1[i]));
					sprintf(field, "onu%d_count1", i+1 );
					addUserVariable(field, buff);
					
					sprintf(buff,"%u", ntohl(portcount->portCount2[i]));
					sprintf(field, "onu%d_count2", i+1 );
					addUserVariable(field , buff);
				}
				
			}

			m_desthtml = "fsacount.html";
	}
	
	return 0;

}

void CGetCount::output()
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

	int ret = getCardCount();
	
	if(ret == 0) {
		if(outputHTMLFile(m_desthtml, true) != 0)
		{
			outputError("Cann't open fsacount.html\n");
		}	
	} 
	else if (-1 != ret )
		outputError(multilang::getError(ret));

	free(cs);

}

#include <iostream>
#include <sstream> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "const.h"
#include "Lang.h"
#include "CgiRequest.h"
#include "CppSQLite3.h"
#include "CGISession.h"
#include "Comm.h"
#include "msg.h"
#include "CFrameView.h"
#include "Tools.h"
using namespace std;

/*
char CFrameView::GetConfig()
{
	FILE *fp;
	char src[MAX_LEN][MAX_LEN];
	char *p=NULL;
	char des;
	int  i=0,line=0;

	if(NULL==(fp=fopen(BACK_CONFIG,"r+w")))
		return ERROR_SYSTEM_GENERAL;
	while(fgets(src[i],sizeof(src[i]),fp))
	{
		if(p=strstr(src[i],FAN_STATE))
		des=(char)atoi(p+strlen(FAN_STATE));
		line=i++;
	}
	fclose(fp);

	return des;
}


int CFrameView::ChgConfig()
{
	FILE *fp;
	char src[MAX_LEN][MAX_LEN];
	char des[MAX_LEN][MAX_LEN];
	char tmp[MAX_LEN][MAX_LEN];
	int i=0,line=0;
	CTools t;

	fanstate=getReqValue("fanstate").c_str();

	if(NULL==(fp=fopen(BACK_CONFIG,"r+w")))
		return  ERROR_SYSTEM_GENERAL;

	while(fgets(src[i],sizeof(src[i]),fp))
	{
		strncpy(tmp[i],src[i],sizeof(src[i]));
		if(strstr(src[i],FAN_STATE))
		{
			memset(des[i],0,sizeof(des[i]));
			t.str_replace(src[i],MAX_LEN,"\n","");
			t.str_replace(src[i],MAX_LEN," ","");
			sprintf(des[i],"%s%s\n",FAN_STATE,fanstate);
		}
		else{
			memset(des[i],0,sizeof(des[i]));
			memcpy(des[i],tmp[i],sizeof(tmp[i]));
		}
		line=i++;
	}
	rewind(fp);
	for(i=0;i<=line;i++)
	fputs(des[i],fp);
	fclose(fp);

	return ERROR_NO;
}
*/
int CFrameView::setShelf()
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];

	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	SHELF_INFO *shelf;

	std::string fan = getReqValue("fanstate"); 
	std::string fan1_t_low = getReqValue("fan1_t_low"); 
	std::string fan1_t_high = getReqValue("fan1_t_high"); 
	std::string fan2_t_low = getReqValue("fan2_t_low"); 
	std::string fan2_t_high = getReqValue("fan2_t_high"); 
	std::string fan3_t_low = getReqValue("fan3_t_low"); 
	std::string fan3_t_high = getReqValue("fan3_t_high"); 

	msg_header=(MSG_HEAD_CMD*)send_buff;
	memset(msg_header,0,sizeof(MSG_HEAD_CMD)+sizeof(SHELF_INFO));
	shelf = (SHELF_INFO*)(send_buff+sizeof(MSG_HEAD_CMD));
	shelf->fan_config = (unsigned char)std::atoi(fan.c_str());
	shelf->t_threshod_low[0] = (unsigned char)std::atoi(fan1_t_low.c_str());
	shelf->t_threshod_high[0] = (unsigned char)std::atoi(fan1_t_high.c_str());
	shelf->t_threshod_low[1] = (unsigned char)std::atoi(fan2_t_low.c_str());
	shelf->t_threshod_high[1] = (unsigned char)std::atoi(fan2_t_high.c_str());
	shelf->t_threshod_low[2] = (unsigned char)std::atoi(fan3_t_low.c_str());
	shelf->t_threshod_high[2] = (unsigned char)std::atoi(fan3_t_high.c_str());

	msg_header->length=sizeof(SHELF_INFO);
	msg_header->msg_code=C_SHELF_SET;
	msg_header->card_no=CTRL_CARDNO;
	if(comm::send_cmd(send_buff,recv_buff)!=0)
		return ERROR_COMM_LOCAL;
	msgrsp_header=(MSG_HEAD_CMD_RSP*)recv_buff;
	if(msgrsp_header->rsp_code!=0)
		return msgrsp_header->rsp_code;

	return 0;
}

int CFrameView::getShelfInfo(SHELF_INFO *shelf)
{
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];

	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;

	msg_header=(MSG_HEAD_CMD*)send_buff;
	memset(msg_header,0,sizeof(MSG_HEAD_CMD));
	msg_header->length=0;
	msg_header->msg_code=C_SHELF_GET;
	msg_header->card_no= CTRL_CARDNO;
	if(comm::send_cmd(send_buff,recv_buff)!=0)
		return ERROR_COMM_LOCAL;
	msgrsp_header=(MSG_HEAD_CMD_RSP*)recv_buff;
	if(msgrsp_header->rsp_code!=0)
		return msgrsp_header->rsp_code;
	if(msgrsp_header->length!=sizeof(SHELF_INFO))
		return ERROR_COMM_PARSE;
	
	memcpy(shelf, recv_buff+sizeof(MSG_HEAD_CMD_RSP), sizeof(SHELF_INFO));
	return 0;
}


void CFrameView::output()
{
	stringstream outss;	
	CCGISession* cs;
	CppSQLite3DB db;
	std::string action = getReqValue("act");
	char buff[256];
	int i, ret;

    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 

	if( action == "set")
	{
		ret=setShelf();
		if(ret != 0)
			outputError(multilang::getError(ret));
		else
		{
			sprintf(buff, "sessionID=%s&act=get", getReqValue("sessionID").c_str());
			redirectHTMLFile("/cgi-bin/frameview.cgi", buff);
		}
		free(cs);
		return;
	}

	int state=0;
	SHELF_INFO shelf;
	
	ret=getShelfInfo(&shelf);	

	if( 0 != ret )
	{
		outputError(multilang::getError(ret));
		free(cs);
		return;
	}

	if( action == "get")
	{
		if((shelf.fan_config&0x08) != 0) 
			addUserVariable("fanauto_checked","checked");
		else 
		{
			if((shelf.fan_config&0x01) != 0)
				addUserVariable("fan1_checked","checked");
			if((shelf.fan_config&0x02) != 0)
				addUserVariable("fan2_checked","checked");
			if((shelf.fan_config&0x04) != 0)
				addUserVariable("fan3_checked","checked");
		}
		sprintf(buff, "%d", shelf.t_threshod_low[0]);
		addUserVariable("fan1_t_low", buff);
		sprintf(buff, "%d", shelf.t_threshod_high[0]);
		addUserVariable("fan1_t_high", buff);
		sprintf(buff, "%d", shelf.t_threshod_low[1]);
		addUserVariable("fan2_t_low", buff);
		sprintf(buff, "%d", shelf.t_threshod_high[1]);
		addUserVariable("fan2_t_high", buff);
		sprintf(buff, "%d", shelf.t_threshod_low[2]);
		addUserVariable("fan3_t_low", buff);
		sprintf(buff, "%d", shelf.t_threshod_high[2]);
		addUserVariable("fan3_t_high", buff);
		
		if( outputHTMLFile("fanctl.html", true) != 0)
		{
			outputError("Cann't open fanctl.html\n");
		}
		
	} else {

		std::string var_empty = "<td width='30' ><IMG src='/images/slot_empty.jpg' border=0></td>\n";

	    try
	    {
		int card_no, card_type;
		int ge_sw=0;

		db.open(DYNAMIC_DB);

		CppSQLite3Query q1 = db.execQuery("select CardNo, CardType from Card where CardType=8 and CardNo=17;");
		if(!q1.eof())
			ge_sw=1;

		CppSQLite3Query q = db.execQuery("select CardNo, CardType from Card order by CardNo;");

		for(i=1; i<=13; i++) {

			if (!q.eof())
	        	{
	        		card_no = q.getIntField(0);
					
				if(card_no >0 && card_no > i) {
					outss << var_empty;
					continue;
				} else  if(card_no == i) {
	        			card_type = q.getIntField(1);

					switch(card_type) {
						case CARD_TYPE_CTRL:
							outss << "<td width='60' ><a href='/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/sctl_card1.jpg' border=0></a></td>\n";
							if(ge_sw)
							{
								outss << "<td width='60' ><a href='/cgi-bin/show_ge8.cgi?sessionID="<< getReqValue("sessionID") <<"&flag=base&cardno="<<GW_CARDNO;
								outss << "'><IMG src='/images/swge_card.jpg' border=0></a></td>\n";
							}
							else
								outss << "<td width='60' ><IMG src='/images/slot_emptyd.jpg' border=0></td>\n";
							break;
						case CARD_TYPE_FSA:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/fsa_card.jpg' border=0></a></td>\n";
							break;
						case CARD_TYPE_DAT:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/dat_card.jpg' border=0></a></td>\n";
							break;
						case CARD_TYPE_FE8:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/fe8_card.jpg' border=0></a></td>\n";
							break;
						case CARD_TYPE_MS4V1H:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/ms4_card.jpg' border=0></a></td>\n";
							break;
						case CARD_TYPE_M4H:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/m4h_card.jpg' border=0></a></td>\n";
							break;
						case CARD_TYPE_M4S:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/m4s_card.jpg' border=0></a></td>\n";
							break;
						case CARD_TYPE_EPN:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/epn_card.jpg' border=0></a></td>\n";
							break;
						case CARD_TYPE_ENCODE:
						case CARD_TYPE_SDE:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<<getReqValue("sessionID") <<"&cardno="<<card_no;
							outss <<"'><IMG src='/images/h264-enc.jpg' border=0></a></td>\n ";
							break;
						case CARD_TYPE_DECODE:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<<getReqValue("sessionID") <<"&cardno="<<card_no;
							outss <<"'><IMG src='/images/h264-dec.jpg' border=0></a></td>\n ";
							break;
						case CARD_TYPE_FSA600:
							outss << "<td width='30'><a href='/cgi-bin/show_card.cgi?sessionID="<<getReqValue("sessionID") <<"&cardno=" <<card_no;
							outss <<"'><IMG src='/images/fsa_600.jpg' border=0></a></td>\n";
							break;
						case CARD_TYPE_GE8:
							outss << "<td width='30' ><a href='/cgi-bin/show_ge8.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "&flag=base'><IMG src='/images/ge8_card.jpg' border=0></a></td>\n";
							break;
						case CARD_TYPE_NTT:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<<getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/ntt2000.jpg' border=0></a></td>\n";
							i++; /*NTT occupied 2 slot */
							break;
                                                case CARD_TYPE_NTT_S: // 20150115 add sing slot
                                                        outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<<getReqValue("sessionID") <<"&cardno="<<card_no;
                                                        outss << "'><IMG src='/images/ntt2000s.jpg' border=0></a></td>\n";
                                                        break;
						case CARD_TYPE_HDC:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/hdec_card.jpg' border=0></a></td>\n";
							break;
						case CARD_TYPE_MSD:
							outss << "<td width='30' ><a href='/cgi-bin/show_card.cgi?sessionID="<< getReqValue("sessionID") <<"&cardno="<<card_no;
							outss << "'><IMG src='/images/msdec_card.jpg' border=0></a></td>\n";
							break;
						default:
							outss << var_empty;
							break;
					}
				} 			
				q.nextRow();
			}
			else
				outss << var_empty;
		}   

		db.close();

	    }
	    catch (CppSQLite3Exception& e)
	    {
	    	std::string error;

		error =  "Database error!<br>";
		error += e.errorCode();
		error += ":";
		error += e.errorMessage();
		
		outputError(error);
		db.close();
		free(cs);
		return;
	    }
		
		//add power slot
		if(shelf.power & 0x01)
			outss << "<td width='60' ><IMG src='/images/power.jpg' border=0></td>\n";
		else
			outss << "<td width='60' ><IMG src='/images/slot_emptyd.jpg' border=0></td>\n";

		if(shelf.power & 0x02)
			outss << "<td width='60' ><IMG src='/images/power.jpg' border=0></td>\n";
		else
			outss << "<td width='60' ><IMG src='/images/slot_emptyd.jpg' border=0></td>\n";
		
		addUserVariable("card_list", outss.str());
		
		sprintf(buff,"%d",shelf.temperature);
		addUserVariable("back_temp",buff);

		for(i=0;i<3;i++)
		{
			sprintf(buff,"fan%d",i+1);
			switch(shelf.fan_state[i])
			{
			case  FAN_ON:
				addUserVariable(buff, "<IMG src='/images/fan_on.gif' border=0>");
				break;
			case  FAN_FAIL:
				addUserVariable(buff,  "<IMG src='/images/fan_fail.gif' border=0>");
				break;
			case  FAN_OFF:
				addUserVariable(buff, "<IMG src='/images/fan_off.gif' border=0>");
				break;
			}
		}
		
		if( outputHTMLFile("frame_view.html", true) != 0)
		{
			outputError("Cann't open frame_view.html\n");
		}	
	}
 
	free(cs);
}


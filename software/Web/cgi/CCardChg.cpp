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

#include "CCardChg.h"
#include "logger.h"

using namespace std;

const char *CCardChg::getCompress ( unsigned char type )
{
	switch ( type )
	{

		case 1:
			return ("<select class='fixsize' name='compress_type'  id='compress_type'>\
				      <option value='1' selected >MPEG2</option>\
				      <option value='2'>MPEG4</option>\
				      <option value='3'>H264</option>\
				      <option value='4' >AVS</option>\
				</selected>");
		case 2:
			return ("<select class='fixsize' name='compress_type'  id='compress_type'>\
				      <option value='1'  >MPEG2</option>\
				      <option value='2' selected>MPEG4</option>\
				      <option value='3'>H264</option>\
				      <option value='4' >AVS</option>\
				</selected>");
		case 3:
			return ("<select class='fixsize' name='compress_type'  id='compress_type'>\
				      <option value='1'  >MPEG2</option>\
				      <option value='2'>MPEG4</option>\
				      <option value='3' selected >H264</option>\
				      <option value='4' >AVS</option>\
				</selected>");
		case 4:
			return ("<select class='fixsize' name='compress_type'  id='compress_type'>\
				      <option value='1'  >MPEG2</option>\
				      <option value='2'>MPEG4</option>\
				      <option value='3'>H264</option>\
				      <option value='4' selected>AVS</option>\
				</selected>");
		default:
			return ("<select class='fixsize' name='compress_type'  id='compress_type'>\
				      <option value='1'  >MPEG2</option>\
				      <option value='2'>MPEG4</option>\
				      <option value='3'>H264</option>\
				      <option value='4' >AVS</option>\
				      <option value='5' selected>Not-Support</option>\
				</selected>");
				
				
				

	}



}

const char *CCardChg::getStream ( unsigned char type )
{
	switch ( type )
	{

		case 1:
			return ("<select class='fixsize' name='stream_type'  id='stream_type'>\
				      <option value='1' selected >TS</option>\
				      <option value='2'>ES</option>\
				</selected>");
		case 2:
			return ("<select class='fixsize' name='stream_type' id='stream_type' >\
				      <option value='1'  >TS</option>\
				      <option value='2' selected>ES</option>\
				</selected>");

		default:
			return ("<select class='fixsize' name='stream_type' id='stream_type' >\
				      <option value='1'  >TS</option>\
				      <option value='2' selected>ES</option>\
				      <option value='3' selected>Not-Support</option>\
				</selected>");
				
				
				

	}
}	


const char *CCardChg::getChannelNO(unsigned char channelno)
{
	switch (channelno)
	{
		case 0:
			return ("<select class='fixsize'  name='channel_no' id='channel_no'> \
				     <option value='0' selected>channel 1</option>\
				     <option value='1' >channel 2</option>\
				</select>");
		case 1:
			return ("<select class='fixsize'  name='channel_no'  id='channel_no'>\
				     <option value='0' >channel 1</option>\
				     <option value='1' selected>channel 2</option>\
				</select>");
		default:
		   	return ("<select class='fixsize'  name='channel_no'  id='channel_no'>\
				     <option value='0' >channel 1</option>\
				     <option value='1' >channel 2</option>\
				     <option value='2' selected>Unkown channel</option>\
				</select>");
	}

}

int mac_covert(unsigned char *s_mac,char *t_mac)
{
	int i=0;
	unsigned char t,t1,t2;
	for(i=0;i<6;i++)
	{
		t1=t_mac[2*i];
		//printf("%c    ",t1);
		if(t1>=97)
		{
			t1=t1-97+10;			
			t1=t1<<4;
			t1=t1&0xf0;
			//t2=t2&0x0f;
		}else if(t>=65)
		{
			t1=t1-65+10;			
			t1=t1<<4;
			t1=t1&0xf0;
		}
		else
		{
			t1=t1-48;
			t1=t1<<4;
			t1=t1&0xf0;
			
		}
		
		t2=t_mac[2*i+1];
		
		if(t2>=97)
		{
			t2=t2-97+10;			
			
			//t2=t2&0x0f;
		}else if(t>=65)
		{
			t2=t2-65+10;			
			
			//t2=t2&0x0f;
		}
		else
		{
			t2=t2-48;
			
			//t2=t2&0x0f;
		}
		//printf("%x",t2);
		t=t1+t2;
		s_mac[i]=t;
		
	}
}

int hex_convert(const char *in_p, char *out_p)
{
	int len;
	char first;
	char *ptr;

	ptr = (char *)in_p;
	for(len=0; len<16; len++)
	{
		while((*ptr == ' ') || (*ptr == ':')) ptr++;

		if(*ptr == 0)
			break;
		
		first =*ptr;		
		ptr++;
		
		if((*ptr == 0) || (*ptr == ' ') || (*ptr == ':'))
		{
			*(out_p + len) = cgicc::hexToChar(0, first);
			continue;
		}
	
		*(out_p + len) = cgicc::hexToChar(first, *ptr);
		ptr++;		
	}
	
	return(len);
}

int CCardChg::updataCardInfo()
{
	CppSQLite3DB db;
	char sql_str[256];
	std::string card_no = getReqValue("cardno");
	std::string action =  getReqValue("action");
	std::string portno = getReqValue ("portno");
	std::string onu100_no = getReqValue ("node_id");
	std::string fe_admin_state = getReqValue ("fe_admin_state");
	std::string fe_port_mode = getReqValue ("fe_port_mode");
	std::string fe_working_mode = getReqValue ("fe_working_mode");
	std::string igmp_enable=getReqValue ("igmp_enable");
	FILE *fd_yp;
	char igmpdat[100];
	GE8_IGMP *igmp_yp;
	int card_type;
	int bw_send, bw_recv;
	std::string mac ;
	char mac1[12];
	GE_PORT_STATE *porstate;
	db.open(DYNAMIC_DB);
	sprintf(sql_str, "select CardType  from Card where CardNo=%d;", std::atoi(card_no.c_str()));
	CppSQLite3Query q = db.execQuery(sql_str);



	if (!q.eof()){
	card_type = q.getIntField(0);
	
		
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	CARD_SET *	card_set;
	BROADCASTSTORM_CTRL_FE *fe8broadcastStorm_set;
	CARD_SET_485FE  *fe8card_set;
	ENCODE_PROTOCOL_STATE *protocol_set;
	ENCODE_VIDEO_STATE *param_set;
	ENCODE_OSD_STATE *osd_set;
	ENCODE_PICTURE_STATE *picture_set;
	ENCODE_MODE_STATE *mode_set;
	ENCODE_NET_STATE *net_set;
	GE8_NET_STATE* ge8_net_set;
	ENCODE_NTP *ntp_set;
	ENCODE_CTRL *encode_ctrl;
	ENCODE_SYSTIME *time_set;
	ENCODE_MODE_STATE *patten;
	FSA600_SET_485 *fsa600_set;
	FSA600_SETRMT *fsa600_setrmt;
	ONU_SET  *onu100_set;
	SW_CONFIG *swconfig;
	CARD_SET_485FE  *nttcard_set;
	int downstream,upstream;
	
	
	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));

	switch (card_type)
	{
		case CARD_TYPE_CTRL:
			if ( action == "setparam")
			{
				unsigned int portmap;
				
				msg_header->msg_code = C_SW_SETPARAM;
				msg_header->length = sizeof(SW_CONFIG);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				swconfig = (SW_CONFIG*)(send_buff+ sizeof(MSG_HEAD_CMD));
				swconfig->setFlag = 0xFF;
				swconfig->qry_time = std::atoi(getReqValue("igmp_qrytime").c_str());
				swconfig->igmp_timeout = std::atoi(getReqValue("igmp_timeout").c_str());
				portmap = 0;
				if( !getReqValue("qp1").empty())
					portmap |= 0x1000000;
				if( !getReqValue("qp2").empty())
					portmap |= 0x2000000;
				swconfig->igmp_qportmap = portmap;
				portmap = 0;
				if( !getReqValue("rp1").empty())
					portmap |= 0x1000000;
				if( !getReqValue("rp2").empty())
					portmap |= 0x2000000;
				swconfig->igmp_rportmap = portmap;
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			}
			else
			{
				msg_header->msg_code = C_CARD_SET;
				msg_header->length = sizeof(CARD_SET);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				card_set = (CARD_SET*)(send_buff+ sizeof(MSG_HEAD_CMD));
				card_set->setFlag = 0xFF;
				strncpy(card_set->card_name, getReqValue("card_name").c_str(), MAX_DISPLAYNAME_LEN);
	//			card_set->admin_state = (unsigned char) std::atoi(getReqValue("admin_state").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			}
			break;
			
		case CARD_TYPE_FSA:
		case CARD_TYPE_DAT:
		case CARD_TYPE_EPN:
		case CARD_TYPE_HDC:
		case CARD_TYPE_SDE:
		case CARD_TYPE_MSD:
			if ( action == "reset")
			{
				msg_header->msg_code = C_CARD_RESET;
				msg_header->length = 0;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
			}
			else if ( action == "upath")
			{
				EPONU_UPPATH *up_path;
				msg_header->msg_code = C_EP_SET_OLT_UPPATH;
				msg_header->length = sizeof(EPONU_UPPATH);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				up_path = (EPONU_UPPATH *)(send_buff + sizeof(MSG_HEAD_CMD));

				memset(up_path, 0, sizeof(EPONU_UPPATH));
				up_path->pon = 0;
				up_path->node_id = 0;
				strncpy(up_path->path, getReqValue("path").c_str(), MAX_NAME2_LEN);
			}
			else if ( action == "vlan")
			{
				EPN_OLT_VLAN *epn_vlan;
				msg_header->msg_code = C_EP_SET_OLTVLAN;
				msg_header->length = sizeof(EPN_OLT_VLAN);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				epn_vlan = (EPN_OLT_VLAN *)(send_buff + sizeof(MSG_HEAD_CMD));
				memset(epn_vlan, 0, sizeof(EPN_OLT_VLAN));

				char field_name[16];
				for(int i=0; i<5; i++)
				{
					for(int j=0; j<5; j++)
					{
						sprintf(field_name, "port%d%d", i+1, j+1);
						if("on" == getReqValue(field_name))
							epn_vlan->vlan_bitmap[i] |= (1<<j);
					}
				}

			}
			else
			{
				msg_header->msg_code = C_CARD_SET;
				msg_header->length = sizeof(CARD_SET);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				card_set = (CARD_SET *)(send_buff+sizeof(MSG_HEAD_CMD));
				memcpy(card_set->card_name, getReqValue("card_name").c_str(), MAX_DISPLAYNAME_LEN);
				card_set->admin_state = (unsigned char)std::atoi(getReqValue("admin_state").c_str());
				card_set->setFlag = 0xFF;	
			}

			if(comm::send_cmd(send_buff, recv_buff) != 0) 
				return ERROR_COMM_LOCAL;
	
			msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
			if(msgrsp_header->rsp_code != 0) 
				return(msgrsp_header->rsp_code);
			
			return ERROR_NO;
			
			break;
			
			
		case CARD_TYPE_NTT:		
		case CARD_TYPE_NTT_S: // 20150115
			if ( action == "base" )
			{
				msg_header->msg_code = C_CARD_SET;
				msg_header->length = sizeof(CARD_SET);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				card_set = (CARD_SET*)(send_buff+ sizeof(MSG_HEAD_CMD));
				card_set->setFlag = 0xFF;
				card_set->admin_state =  (unsigned char)std::atoi(getReqValue("admin_state").c_str());
				strncpy(card_set->card_name, getReqValue("card_name").c_str(), MAX_DISPLAYNAME_LEN);

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				return ERROR_NO;
			}

			if ( action == "set" )
			{
				
				msg_header->msg_code = C_FE8_BROADCASTSTORM_SET;
				msg_header->length = sizeof(BROADCASTSTORM_CTRL_FE);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				BROADCASTSTORM_CTRL_FE *fe8broadcastStorm_set;
				fe8broadcastStorm_set = (BROADCASTSTORM_CTRL_FE*)(send_buff+ sizeof(MSG_HEAD_CMD));
				fe8broadcastStorm_set->setFlag = 0xFF;
				fe8broadcastStorm_set->broadcaststorm_ctrl = (unsigned char)std::atoi(getReqValue("storm_ctrl").c_str());
				fe8broadcastStorm_set->storm_threshold = (unsigned short)std::atoi(getReqValue("storm_threshold").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
		
				memset(send_buff,0,sizeof(send_buff));
				
				msg_header = (MSG_HEAD_CMD *)send_buff;
			
				msg_header->msg_code = C_NTT_SET;
				msg_header->length = sizeof(CARD_SET_485FE);
				
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				nttcard_set = (CARD_SET_485FE*)(send_buff+ sizeof(MSG_HEAD_CMD));

				char buff[256];
			
				for(int j=1;j<=8;j++)
				{
				
					nttcard_set->Index = j;
					nttcard_set->port.setFlag = 0xFF;
					memset(buff,0,256);
					sprintf(buff,"ntt_admin_state%d",j);
					nttcard_set->port.admin_state = (unsigned char)std::atoi(getReqValue(buff).c_str());

					memset(buff,0,256);
					sprintf(buff,"ntt_bw_alloc_recv%d",j);
					bw_recv = std::atoi(getReqValue(buff).c_str());
					nttcard_set->port.bw_alloc_recv = bw_recv/32;
						
					memset(buff,0,256);
					sprintf(buff,"ntt_bw_alloc_send%d",j);
					bw_send = std::atoi(getReqValue(buff).c_str() ) ;
					nttcard_set->port.bw_alloc_send =bw_send /32;
				

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
				}
										
				return ERROR_NO;
			}
			break;
		case CARD_TYPE_FE8:
			if ( action == "base" )
			{
				msg_header->msg_code = C_CARD_SET;
				msg_header->length = sizeof(CARD_SET);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				card_set = (CARD_SET*)(send_buff+ sizeof(MSG_HEAD_CMD));
				card_set->setFlag = 0xFF;
				card_set->admin_state =  (unsigned char)std::atoi(getReqValue("admin_state").c_str());
				strncpy(card_set->card_name, getReqValue("card_name").c_str(), MAX_DISPLAYNAME_LEN);

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
				return ERROR_NO;
			}
			if ( action == "setBroadcastStorm" )
			{
				msg_header->msg_code = C_FE8_BROADCASTSTORM_SET;
				msg_header->length = sizeof(BROADCASTSTORM_CTRL_FE);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				BROADCASTSTORM_CTRL_FE *fe8broadcastStorm_set;
				fe8broadcastStorm_set = (BROADCASTSTORM_CTRL_FE*)(send_buff+ sizeof(MSG_HEAD_CMD));
				fe8broadcastStorm_set->setFlag = 0xFF;
				fe8broadcastStorm_set->broadcaststorm_ctrl = (unsigned char)std::atoi(getReqValue("storm_ctrl").c_str());
				fe8broadcastStorm_set->storm_threshold = (unsigned short)std::atoi(getReqValue("storm_threshold").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
				return ERROR_NO;
			}
			if ( action == "port" )
			{
				msg_header = (MSG_HEAD_CMD *)send_buff;
				memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
				msg_header->msg_code = C_FE8_SET;
				msg_header->length = sizeof(CARD_SET_485FE);
				
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				fe8card_set = (CARD_SET_485FE*)(send_buff+ sizeof(MSG_HEAD_CMD));

				char field_name[32];
				
					
				fe8card_set->Index = (unsigned char )std::atoi(portno.c_str());
				fe8card_set->admin_state = (unsigned char)std::atoi(getReqValue("admin_state").c_str());	
				
				fe8card_set->port.setFlag = 0xFF;
				fe8card_set->port.admin_state = (unsigned char)std::atoi(getReqValue("fe_admin_state").c_str());
				fe8card_set->port.portMode = (unsigned char)std::atoi(getReqValue( "fe_port_mode").c_str());
				fe8card_set->port.portWorkingMode = (unsigned char)std::atoi(getReqValue( "fe_working_mode").c_str());

				bw_recv = std::atoi(getReqValue("fe_bw_alloc_recv").c_str());
				fe8card_set->port.bw_alloc_recv = bw_recv/32;
						

				bw_send = std::atoi(getReqValue("fe_bw_alloc_send").c_str() ) ;
				fe8card_set->port.bw_alloc_send =bw_send /32;
						

				mac = getReqValue("fe_bounded_mac");
				cgicc::getMac (mac, fe8card_set->port.bounded_mac);

				/*test mac address trans*/
				//sprintf (field_name, "%d-%d-%d-%d-%d-%d", fe8card_set->port.bounded_mac[0], fe8card_set->port.bounded_mac[1], fe8card_set->port.bounded_mac[2], fe8card_set->port.bounded_mac[3], fe8card_set->port.bounded_mac[4], fe8card_set->port.bounded_mac[5]);
				//addUserVariable ("femac", field_name);

				//sprintf (field_name, "%d-%d-%d-%d-%d-%d",* p, *(p+1) ,*(p+2) ,*(p+2));
				//addUserVariable ("p", field_name);
				addUserVariable ("getmac", mac);
				

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
						
						
				
				return ERROR_NO;
			}
			break;

		case CARD_TYPE_FSA600:
			msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
			msg_header->node_id = (unsigned char) std::atoi(onu100_no.c_str());
			/*if ( action == "base")
			{
				msg_header->msg_code = C_CARD_SET;
				
			}*/
			/*if ( action == "set_loc")
			{
				msg_header->msg_code = C_FSA600_SET;
			}*/
			/*if( action == "set_rmt" )
			{
				msg_header->msg_code = C_FSA600_SETRMT;
			}*/
			if ( action == "reset_loc")
			{
				msg_header->msg_code = C_FSA600_RESET;
			}
			if ( action == "setdefault_loc")
			{
				msg_header->msg_code = C_FSA600_SETDFT;
			}
			if ( action == "reset_rmt")
			{
				msg_header->msg_code = C_FSA600_RESETRMT;
			}
			if ( action =="setdefault_rmt" )
			{
				msg_header->msg_code = C_FSA600_SETDFTRMT;
			}

			if ( action == "base" )
			{
				msg_header->msg_code = C_CARD_SET;
				msg_header->length = sizeof(CARD_SET);
				card_set = (CARD_SET*)(send_buff+ sizeof(MSG_HEAD_CMD));
				card_set->setFlag = 0xFF;
				card_set->admin_state =  (unsigned char)std::atoi(getReqValue("admin_state").c_str());
				strncpy(card_set->card_name, getReqValue("card_name").c_str(), MAX_DISPLAYNAME_LEN);

				
			}
			if ( action == "set_loc")
			{
				msg_header->msg_code = C_FSA600_SET;
				msg_header->length = sizeof (FSA600_SET_485);
 				fsa600_set = (FSA600_SET_485 *) (send_buff + sizeof (MSG_HEAD_CMD) );
				fsa600_set->setFlag = 0xFF;
				fsa600_set->TransmitMode = (unsigned char)std::atoi(getReqValue("transmit_mode").c_str() );
				fsa600_set->PortState = (unsigned char )std::atoi(getReqValue("port_lock").c_str() );
				
				upstream = std::atoi (getReqValue("up_stream").c_str() );
				downstream = std::atoi(getReqValue("down_stream").c_str() );
				if ( upstream > 8192 )
				{
					fsa600_set->UpStreamFlag = 1;
					fsa600_set->UpStream = upstream/512;
				}
				else{
					fsa600_set->UpStreamFlag = 0;
					fsa600_set->UpStream = upstream/32;

				}

				if ( downstream > 8192 )
				{
					fsa600_set->DownStreamFlag = 1;
					fsa600_set->DownStream = downstream/512;
				}
				else{

					fsa600_set->DownStreamFlag = 0;
					fsa600_set->DownStream = downstream/32;
				}
				
			}
			if ( action == "set_rmt")
			{	/*msg_header->msg_code = C_ONU_SET;
				msg_header->length = sizeof (ONU_SET);
				onu100_set = (ONU_SET *)(send_buff  + sizeof (MSG_HEAD_CMD));
				onu100_set->setFlag = 0xFF;
				strncpy ( onu100_set->onu_name , getReqValue("onu_name").c_str(), MAX_DISPLAYNAME_LEN);

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);*/

				msg_header->msg_code = C_FSA600_SETRMT;
				msg_header->length = sizeof(FSA600_SETRMT);
				fsa600_setrmt = (FSA600_SETRMT *) (send_buff + sizeof (MSG_HEAD_CMD ));
				fsa600_setrmt->setFlag = 0xFF;
				fsa600_setrmt->OnuId = (unsigned char )std::atoi( onu100_no.c_str()  );
				fsa600_setrmt->TransmitMode = (unsigned char )std::atoi(getReqValue("transmit_mode").c_str());
				fsa600_setrmt->CfgWorkMode1 = (unsigned char )std::atoi(getReqValue("conf_workmode_onu1").c_str());
				fsa600_setrmt->CfgWorkMode2 = (unsigned char )std::atoi (getReqValue("conf_workmode_onu2").c_str());
				strncpy ( fsa600_setrmt->onu_name , getReqValue("onu_name").c_str(), MAX_DISPLAYNAME_LEN);

				
			}

			if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
			if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
				return ERROR_NO;

			break;

		case CARD_TYPE_ENCODE:
		case CARD_TYPE_DECODE:
			if ( action == "protocol_set" )
			{
				msg_header->msg_code = C_ENCODE_PROTOCOL_SET;
				msg_header->length = sizeof (ENCODE_PROTOCOL_STATE);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

				protocol_set = (ENCODE_PROTOCOL_STATE *) (send_buff + sizeof (MSG_HEAD_CMD));
				protocol_set->channelno = (unsigned char) std::atoi(getReqValue("ch_no").c_str());
				memcpy (protocol_set->protocol,getReqValue("protocol").c_str(), 6 );
				memcpy (protocol_set->ip ,getReqValue("ip").c_str(), 16 );
				memcpy (protocol_set->port,getReqValue("port").c_str(), 8 );
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			
			}

			if ( action == "param_set" )
			{
				msg_header->msg_code = C_ENCODE_VIDEO_SET;
				msg_header->length = sizeof (ENCODE_VIDEO_STATE);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

				param_set = (ENCODE_VIDEO_STATE *) (send_buff + sizeof (MSG_HEAD_CMD));
				param_set->channelno = (unsigned char) std::atoi(getReqValue("ch_no").c_str());
				char vbr[4];
				const char *vid_bit_cbr = getReqValue("vid_vbr").c_str();
				const char *vid_bit_type = getReqValue("vid_bit_type").c_str();
				if (!strcmp(vid_bit_type, "0")) {
					sprintf(vbr, "%s", "0");
				}else {
					sprintf(vbr, "%s", vid_bit_cbr);
				}
				memcpy (param_set->vidBitType,vbr, 4 );
				memcpy (param_set->vidBitRate,getReqValue("vid_bit_rate").c_str(), 8 );
				memcpy (param_set->resolving,getReqValue("resolving").c_str(), 8 );
				memcpy (param_set->frameRate,getReqValue("frm_rate").c_str(), 3 );
				memcpy (param_set->gop,getReqValue("gop").c_str(), 4 );
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			
			}
			
			if ( action == "osd_set" )
			{
				msg_header->msg_code = C_ENCODE_OSD_SET;
				msg_header->length = sizeof (ENCODE_OSD_STATE);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

				osd_set = (ENCODE_OSD_STATE *) (send_buff + sizeof (MSG_HEAD_CMD));
				osd_set->channelno = (unsigned char) std::atoi(getReqValue("ch_no").c_str());
				memcpy (osd_set->location ,getReqValue("location").c_str(), 4 );
				osd_set->show_time = (unsigned char) std::atoi(getReqValue("show_time").c_str());
				memcpy (osd_set->channel_name ,getReqValue("channel_name").c_str(), MAX_DISPLAYNAME_LEN );
				memcpy (osd_set->color ,getReqValue("osd_color").c_str(), 4 );
				//osd_set->color = (unsigned char) std::atoi(getReqValue("osd_color").c_str());
				//osd_set->location = (unsigned char) std::atoi(getReqValue("location").c_str());
				//osd_set->show_vlost = (unsigned char) std::atoi(getReqValue("vlost").c_str());
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			
			}


			if ( action == "picture_set" )
			{
				msg_header->msg_code = C_ENCODE_PICTURE_SET;
				msg_header->length = sizeof (ENCODE_PICTURE_STATE);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

				picture_set = (ENCODE_PICTURE_STATE *) ( send_buff + sizeof (MSG_HEAD_CMD));
				picture_set->channelno = (unsigned char) std::atoi(getReqValue("ch_no").c_str());
				memcpy (picture_set->bright,getReqValue("bright").c_str(), 4 );
				memcpy (picture_set->contrast,getReqValue("contrast").c_str(), 4 );
				memcpy (picture_set->chroma,getReqValue("chroma").c_str(), 4 );
				memcpy (picture_set->saturation,getReqValue("saturation").c_str(), 4 );
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			}

			
			if ( action == "mode_set" )
			{
				msg_header->msg_code = C_ENCODE_MODE_SET;
				msg_header->length = sizeof (ENCODE_MODE_STATE);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

				mode_set = (ENCODE_MODE_STATE *) ( send_buff + sizeof (MSG_HEAD_CMD));
				mode_set->channelno = (unsigned char) std::atoi(getReqValue("ch_no").c_str());
				
				//mode_set->compresstype = (unsigned char) std::atoi(getReqValue("compress_type").c_str());
				mode_set->streamtype =  (unsigned char) std::atoi(getReqValue("stream_type").c_str());
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			
			}

		#if 0
			  if ( action  == "mode_get")
			{
				msg_header->msg_code = C_ENCODE_MODE_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("channel_no").c_str());
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				msg_header->length = 0;
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_MODE_STATE))
					return(ERROR_COMM_PARSE);
				patten = (ENCODE_MODE_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );
				
				//addUserVariable("channel_no", getChannelNO( patten->channelno));
				
				//addUserVariable("compress_type", getCompress( patten->compresstype ));

				addUserVariable("stream_type" , getStream( patten->streamtype ));

				return ERROR_NO;


			}
			  #endif

			
			/*if ( action == "start" )
			{
				msg_header->msg_code = C_ENCODE_CTRL;
				msg_header->length = sizeof (ENCODE_CTRL);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

				encode_ctrl = (ENCODE_CTRL *) ( send_buff + sizeof (MSG_HEAD_CMD));
				encode_ctrl->flag = 1;
				
				encode_ctrl->channelno = (unsigned char) std::atoi(getReqValue("channel_no").c_str());
				memcpy (encode_ctrl->ipaddr, getReqValue("ipaddr").c_str() , 17);
				encode_ctrl->portno = (unsigned short ) std::atoi(getReqValue("portno").c_str());
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			
			}

			
			if ( action == "stop" )
			{
				msg_header->msg_code = C_ENCODE_CTRL;
				msg_header->length = sizeof (ENCODE_CTRL);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

				encode_ctrl = (ENCODE_CTRL *) ( send_buff + sizeof (MSG_HEAD_CMD));
				encode_ctrl->flag = 2;
				
				encode_ctrl->channelno = (unsigned char) std::atoi(getReqValue("channel_no").c_str());
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			
			}*/

			if ( action == "net_set" )
			{
				msg_header->msg_code = C_ENCODE_NET_SET;
				msg_header->length = sizeof (ENCODE_NET_STATE);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

				net_set = (ENCODE_NET_STATE *) ( send_buff + sizeof (MSG_HEAD_CMD));
				net_set->channelno = (unsigned char) std::atoi(getReqValue("ch_no").c_str());
				memcpy (net_set->gateway, getReqValue("encode_gateway").c_str(), 17);
				memcpy (net_set->ipaddr , getReqValue("encode_ip").c_str(), 17);
				memcpy (net_set->mask, getReqValue("encode_mask").c_str(), 17);
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			
			}

			if ( action == "ntp_set" )
			{
				msg_header->msg_code = C_ENCODE_NTP_SET;
				msg_header->length = sizeof (ENCODE_NTP);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

				ntp_set = (ENCODE_NTP *) ( send_buff + sizeof (MSG_HEAD_CMD));
				
				memcpy (ntp_set->server_addr , getReqValue("ntp_ip").c_str(), 17);
				//ntp_set->interval =  (unsigned short) std::atoi(getReqValue("interval").c_str());
				memcpy (ntp_set->interval , getReqValue("interval").c_str(), 8);
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			
			}

			if ( action == "settime" )
			{
				msg_header->msg_code = C_ENCODE_SYSTIME_SET;
				msg_header->length = sizeof (ENCODE_SYSTIME);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

				time_set = (ENCODE_SYSTIME *) ( send_buff + sizeof (MSG_HEAD_CMD));
				
				memcpy (time_set->time , getReqValue("encode_settime").c_str(), 20);
				
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			
			}
			char buff[32];
			if ( action == "gettime" )
			{
				msg_header->msg_code = C_ENCODE_SYSTIME_GET;
				msg_header->node_id = 0;
				msg_header->length = 0;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_SYSTIME))
					return(ERROR_COMM_PARSE);
				time_set = (ENCODE_SYSTIME *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );

				sprintf (buff, "%s", time_set->time);
				addUserVariable("encode_time", buff);
				
				return ERROR_NO;
				
			}
			
			if ( action == "restart" )
			{
				msg_header->msg_code = C_ENCODE_RESTART;
				msg_header->length = 0;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			
			}
			
			break;

		case CARD_TYPE_GE8:
			if ( action == "net_set" )
			{
				std::string tmp;
				msg_header->msg_code = C_GE8_NET_SET;
				msg_header->length = sizeof (GE8_NET_STATE);
                msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				
				
				ge8_net_set = (GE8_NET_STATE *) ( send_buff + sizeof (MSG_HEAD_CMD));
				memcpy (ge8_net_set->gateway, getReqValue("encode_gateway").c_str(), 17);
				memcpy (ge8_net_set->ipaddr , getReqValue("encode_ip").c_str(), 17);
				memcpy (ge8_net_set->mask, getReqValue("encode_mask").c_str(), 17);
				memcpy (mac1, getReqValue("encode_mac").c_str(), 12);
				mac_covert(ge8_net_set->mac,mac1);
				logger(LOG_ERR,"t_MAC%s,s_mac %x%x%x%x%x%x%x\n",mac1,ge8_net_set->mac[0],ge8_net_set->mac[1],ge8_net_set->mac[2],ge8_net_set->mac[3],ge8_net_set->mac[4],ge8_net_set->mac[5]);
				tmp=getReqValue("ip_enable");
				if(tmp=="on")
					ge8_net_set->enable=1;
				else
					ge8_net_set->enable=0;
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
					
				return ERROR_NO;
			}

			if(action == "port_set")
			{
				
				msg_header->msg_code = C_GE8_SETPORT;
				msg_header->length = sizeof(GE_PORT_STATE);
				porstate = (GE_PORT_STATE *) (send_buff + sizeof (MSG_HEAD_CMD ));
				porstate->portIndex = (unsigned char )std::atoi(portno.c_str() );
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				porstate->admin_state=(unsigned char )std::atoi(fe_admin_state.c_str() );
				porstate->portMode=(unsigned char )std::atoi(fe_port_mode.c_str() );
				porstate->portWorkingMode=(unsigned char )std::atoi(fe_working_mode.c_str() );
				logger(LOG_ERR,"port portindex %d, adminstate %d",porstate->portIndex,porstate->admin_state);
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
				return ERROR_NO;
			}
			if(action == "port_rset")
			{
				msg_header->msg_code = C_GE8_STATISTICS_CLR;
				msg_header->length = 0;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
				return ERROR_NO;
			}
			if(action == "setigmp")
			{
				int igmp_i=0,num;
				char igmp_buf[100];
				unsigned char igmpmask=0x00;
				std::string tmp;
				msg_header->msg_code = C_GE8_SETIGMP;
				msg_header->length = sizeof(GE8_IGMP);
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				igmp_yp = (GE8_IGMP *)(send_buff+ sizeof(MSG_HEAD_CMD));
				tmp=getReqValue("igmp_enable");
				if(tmp=="on")
					igmp_yp->State=1;
				else
					igmp_yp->State=0;
				//logger(LOG_ERR,"igmp_flood%s\n",getReqValue("igmp_flood").c_str());
				tmp=getReqValue("igmp_flood");
				if(tmp=="on")
					igmp_yp->Ipmc_flood=1;
				else
					igmp_yp->Ipmc_flood=0;

				for(igmp_i=0;igmp_i<8;igmp_i++)
				{
					sprintf(igmp_buf,"igmp_p%d",igmp_i);
					tmp=getReqValue(igmp_buf);
					if(tmp=="on")
					{
						igmpmask=(0x01<<igmp_i)|igmpmask;
					}
					
				}
				
				
				igmp_yp->Route_port=igmpmask;
				num=std::atoi(getReqValue("igmp_vlannum").c_str());
				
				for(igmp_i=0;igmp_i<num;igmp_i++)
				{
					sprintf(igmp_buf,"igmp_vlanid%d",igmp_i);
					igmp_yp->Vid[igmp_i]=std::atoi(getReqValue(igmp_buf).c_str());
					
					
					//logger(LOG_ERR,"igmp_vid%f\n",igmp_yp->Vid[igmp_i]);
					sprintf(igmp_buf,"Snoop_state%d",igmp_i);
					tmp=getReqValue(igmp_buf);
					if(tmp=="on")
						igmp_yp->Snoop_state[igmp_i]=1;
					else
						igmp_yp->Snoop_state[igmp_i]=0;

					sprintf(igmp_buf,"Query_state%d",igmp_i);
					tmp=getReqValue(igmp_buf);
					if(tmp=="on")
						igmp_yp->Query_state[igmp_i]=1;
					else
						igmp_yp->Query_state[igmp_i]=0;
				
					
				}
				igmp_yp->Vid[num] = 0xff;
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);
				return ERROR_NO;
				}
			break;
			
		case CARD_TYPE_MS4V1H:
		case CARD_TYPE_M4S:
		case CARD_TYPE_M4H:
		
			if ( action == "switch" )
			{
				msg_header->msg_code = C_MS4_SWITCH;
				msg_header->length = 1;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				*(send_buff+ sizeof(MSG_HEAD_CMD)) =  (char)std::atoi(getReqValue("input_video").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
				else
					return ERROR_NO;
			} else if ( action == "debug" ) {
				char * param;
				int input_len;
				
				msg_header->msg_code = C_MS4_DEBUG;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				param = send_buff+ sizeof(MSG_HEAD_CMD);

				memset(param, 0, 16);
				input_len = hex_convert(getReqValue("debug_input").c_str(), param);
				if(input_len < 1)
					return(ERROR_WRONG_ARGS);
				msg_header->length = input_len;

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;

				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				param = recv_buff+ sizeof(MSG_HEAD_CMD_RSP);
				addUserVariable("dbg_output", cgicc::dumpBuff(param, msgrsp_header->length));
				param = send_buff+ sizeof(MSG_HEAD_CMD);
				addUserVariable("dbg_input", cgicc::dumpBuff(param, input_len).substr(2));

				return ERROR_NO;
			}
			break;		
		default:
			break;
		
		}


	}
	else
		return ERROR_SYSTEM_GENERAL;
	
	db.close();
}



void CCardChg::output()
{
	std::string result;
	CCGISession* cs;	
	std::string action =  getReqValue("action");
	
    	cs = new CCGISession();      
	if(cs->IsExistIDForSession(getReqValue("sessionID").c_str(), getRemoteAddr().c_str()))
	{
		addUserVariable("error", multilang::getError(ERROR_LOGIN_TIMEOUT));
		outputHTMLFile("/relogin.html", true);	
		free(cs);
		return;
	} 
	
	 int ret = updataCardInfo();

	if( (ret == 0) && (action =="gettime") )
	{
		if(outputHTMLFile("encode_time.html", true) != 0)
		{
			outputError("Cann't open encode_time.html\n");
		}	
	}
	else if ( (ret == 0) && (action =="mode_get"))
	{
		if(outputHTMLFile("encode_mode.html", true) != 0)
		{
			outputError("Cann't open encode_mode.html\n");
		}	
	}
	else if ( (ret == 0) && (action =="debug"))
	{
		if(outputHTMLFile("ms4_debug.html", true) != 0)
		{
			outputError("Cann't open ms4_debug.html\n");
		}	
	}	
	else 
	{
			/*if(outputHTMLFile("test.html", true) != 0)// for test mac.
			{
			outputError("Cann't open test.html\n");
			}*/
		
			result = multilang::getError(ret);
			 outputError(result);
	}
	
	free(cs);

}

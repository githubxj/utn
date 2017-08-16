#include <iostream>
#include <sstream> 
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sstream> 
#include <vector>
#include <syslog.h>
#include <netinet/in.h>

#include "CgiRequest.h"
#include "CGISession.h"
#include "Lang.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"
#include "logger.h"
#include "CHDCard.h"
#include "CppSQLite3.h"

using namespace std;

const char *CHDCard::getSourceType(unsigned char type)
{
	switch(type) {
		case 1:
			return ("<select class='fixsize' id='source_type' name='source_type'> \
					<option value='1' selected>TS</option> \
					<option value='2' >RTP</option> \
					<option value='3' >ES</option> \
					</select>");
		case 2:
			return ("<select class='fixsize' id='source_type' name='source_type'> \
					<option value='1' >TS</option> \
					<option value='2' selected>RTP</option> \
					<option value='3' >ES</option> \
					</select>");
		default:
			return ("<select class='fixsize' id='source_type' name='source_type'> \
					<option value='1' >TS</option> \
					<option value='2' >RTP</option> \
					<option value='3' selected>ES</option> \
					</select>");
			
	}

}

const char *CHDCard::getProtocol(unsigned char type)
{
	switch(type) {
		case 1:
			return ("<select class='fixsize' id='protocol' name='protocol'> \
					<option value='0' >UDP</option> \
					<option value='1' selected>TCP</option> \
					</select>");
		default:
			return ("<select class='fixsize' id='protocol' name='protocol'> \
					<option value='0' selected>UDP</option> \
					<option value='1' >TCP</option> \
					</select>");			
	}

}


const char *CHDCard::getOutputType(unsigned char type)
{
	switch(type) {
		case 2:
			return (" <option value='1' >DVI</option> \
					<option value='2' selected>HDMI</option>");
		default:
			return (" <option value='1' selected>DVI</option> \
					<option value='2' >HDMI</option>");
	}

}

const char *CHDCard::getFrameRate(unsigned char rate)
{
	switch(rate) {
		case 25:
			return ("<select class='fixsize'  id='frame_rate' name='frame_rate'>\
					<option value='25' selected>25</option>\
					<option value='30' >30</option>\
					</select>");
		default:
			return ("<select class='fixsize' id='frame_rate' name='frame_rate'>\
					<option value='25' >25</option>\
					<option value='30' selected>30</option>\
					</select>");
			
	}

}

const char *CHDCard::getDecMode(unsigned char var)
{
	switch(var) {
		case 0:
			return ("<select class='fixsize' id='dec_mode' name='dec_mode'>\
					<option value='0' selected>Stream</option>\
					<option value='1' >Frame</option>\
					</select>");
		default:
			return ("<select class='fixsize' id='dec_mode' name='dec_mode'>\
					<option value='0' >Stream</option>\	
					<option value='1' selected>Frame</option>\
					</select>");
			
	}

}

const char *CHDCard::getCVBSMode(unsigned char var)
{
	switch(var) {
		case 0:
			return ("<select class='fixsize'  name='cvbs_mode'  id='cvbs_mode'>\
					<option value='0' selected>PAL</option>\
					<option value='1' >NTSC</option>\
					</select>");
		default:
			return ("<select class='fixsize'  name='cvbs_mode' id='cvbs_mode'>\
					<option value='0' >PAL</option>\
					<option value='1' selected>NTSC</option>\
					</select>");
			
	}

}

const char *CHDCard::getOutputMode1(unsigned char var)
{
	switch(var) {
		case 3:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' selected>480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		case 4:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' selected>576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		case 5:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' selected>720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		case 6:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' selected>720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		case 7:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' selected>1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		case 8:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' selected>1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		case 9:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' selected>1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		case 10:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' selected>1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		case 11:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' selected>1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		case 12:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' selected>1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		case 13:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' selected>800x600</option>\
					<option value='14' >1024x768</option>\
					</select>");
		default:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode1'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					<option value='13' >800x600</option>\
					<option value='14' selected>1024x768</option>\
					</select>");
			
	}

}

const char *CHDCard::getOutputMode2(unsigned char var)
{
	switch(var) {
		case 3:
			return ("<select class='fixsize'  name='output_mode2' id='output_mode2'>\
					<option value='3' selected>480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					</select>");
		case 4:
			return ("<select class='fixsize'  name='output_mode2' id='output_mode2'>\
					<option value='3' >480P60</option>\
					<option value='4' selected>576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					</select>");
		case 5:
			return ("<select class='fixsize'  name='output_mode2' id='output_mode2'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' selected>720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					</select>");
		case 6:
			return ("<select class='fixsize'  name='output_mode2' id='output_mode2'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' selected>720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					</select>");
		case 7:
			return ("<select class='fixsize'  name='output_mode2' id='output_mode2'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' selected>1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					</select>");
		case 8:
			return ("<select class='fixsize'  name='output_mode12' id='output_mode2'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' selected>1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					</select>");
		case 9:
			return ("<select class='fixsize'  name='output_mode2' id='output_mode2'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' selected>1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					</select>");
		case 10:
			return ("<select class='fixsize'  name='output_mode2' id='output_mode2'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' selected>1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' >1080P30</option>\
					</select>");
		case 11:
			return ("<select class='fixsize'  name='output_mode2' id='output_mode2'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' selected>1080P25</option>\
					<option value='12' >1080P30</option>\
					</select>");
		default:
			return ("<select class='fixsize'  name='output_mode1' id='output_mode2'>\
					<option value='3' >480P60</option>\
					<option value='4' >576P50</option>\
					<option value='5' >720P60</option>\
					<option value='6' >720P50</option>\
					<option value='7' >1080I60</option>\
					<option value='8' >1080I50</option>\
					<option value='9' >1080P60</option>\
					<option value='10' >1080P50</option>\
					<option value='11' >1080P25</option>\
					<option value='12' selected>1080P30</option>\
					</select>");
	}

}


int CHDCard::get_display_mode_by_string(const char *str)
{
  if (NULL == str) return 0;

  if (!strcmp("mode_pic_1_ch_0", str)) 
  	return 0;
  else if (!strcmp("mode_pic_1_ch_1", str))
  	return 1;
  else if (!strcmp("mode_pic_1_ch_2", str))
  	return 2;
  else if (!strcmp("mode_pic_1_ch_3", str)) 
  	return 3;
  else if (!strcmp("mode_hd", str)) 
  	return 0;
  else if (!strcmp("mode_pic_4", str)) 
  	return 16;
  else if (!strcmp("mode_pic_9", str))
  	return 17;
  else if (!strcmp("mode_pic_16", str))
  	return 18;
  else if (!strcmp("mode_group", str))
  	return 19;
  else if (!strcmp("mode_cvbs_in", str)) 
  	return 20;
  else 
  	return 0;

}

int CHDCard::genSwitchCmd(char *out, int ch, int display_mode)
{
	switch (display_mode)
	{
		case 0: 
		case 1: 
		case 2: 
		case 3: 
			sprintf(out, "SetDecMode %d mode=D1 ch=%d", ch, display_mode); 
			break;
		case 16:	
			sprintf(out, "SetDecMode %d mode=4D1", ch); 
			break;
		case 21:	
			sprintf(out, "SetDecMode %d mode=PJ4", ch); 
			break;
		case 22:	
			sprintf(out, "SetDecMode %d mode=PJ9", ch); 
			break;
		default:
			return -1;      
	}

	return 0;
}

int CHDCard::procHDReq()
{
	std::string card_no = getReqValue("cardno");
	std::string flag = getReqValue("flag");
	
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	HDEC_CFG * hdec_cfg;
	
	char buff[256];

	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());

	if ( flag == "cfg")
	{
		// get config
		msg_header->msg_code = C_HDC_GETCMD;
		sprintf(send_buff+sizeof(MSG_HEAD_CMD), "CFG");
		msg_header->length = 3;
		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		if(msgrsp_header->length != sizeof(HDEC_CFG))
			return(ERROR_COMM_PARSE);

		hdec_cfg = (HDEC_CFG*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
					
		addUserVariable( "source_type" , getSourceType(hdec_cfg->source_type));
		addUserVariable( "frame_rate" , getFrameRate(hdec_cfg->frame_rate));
		addUserVariable( "cvbs_mode" , getCVBSMode(hdec_cfg->cvbs_mode));
		addUserVariable( "dec_mode" , getDecMode(hdec_cfg->dec_mode));
		addUserVariable( "protocol" , getProtocol(hdec_cfg->transport_type));
		sprintf(buff, "%d", ntohs(hdec_cfg->filter));
		addUserVariable("filter", buff);
			
		addUserVariable("display_mode1", multilang::getDisplayMode(hdec_cfg->display_mode1) );
		addUserVariable("output_mode1" , getOutputMode1(hdec_cfg->output_mode1));
		addUserVariable("output_type1" , getOutputType(hdec_cfg->output_type[0]));
		sprintf(buff, "%d", ntohs(hdec_cfg->pj_no[0]));
		addUserVariable("pj_no1", buff);

		addUserVariable("display_mode2", multilang::getDisplayMode(hdec_cfg->display_mode2) );
		addUserVariable("output_mode2" , getOutputMode2(hdec_cfg->output_mode2));
		addUserVariable("output_type2" , getOutputType(hdec_cfg->output_type[1]));
		sprintf(buff, "%d", ntohs(hdec_cfg->pj_no[1]));
		addUserVariable("pj_no2", buff);

		m_desthtml = "hdcfg_table.html";
	
	} else if (flag == "setcfg") {
		msg_header->msg_code = C_HDC_SETCMD;
		sprintf(send_buff+sizeof(MSG_HEAD_CMD), "CFG");
		msg_header->length = sizeof(HDEC_CFG) + 3;
		
		hdec_cfg = (HDEC_CFG *)(send_buff+sizeof(MSG_HEAD_CMD) + 3);
		memset(hdec_cfg, 0, sizeof(HDEC_CFG));
		hdec_cfg->source_type = (unsigned char)std::atoi(getReqValue("source_type").c_str());
		hdec_cfg->transport_type = (unsigned char)std::atoi(getReqValue("protocol").c_str());
		hdec_cfg->frame_rate = (unsigned char)std::atoi(getReqValue("frame_rate").c_str());
		hdec_cfg->dec_mode = (unsigned char)std::atoi(getReqValue("dec_mode").c_str());
		hdec_cfg->cvbs_mode = (unsigned char)std::atoi(getReqValue("cvbs_mode").c_str());
		hdec_cfg->filter = htons((unsigned short)std::atoi(getReqValue("filter").c_str()));
		
		hdec_cfg->display_mode1 = (unsigned char)std::atoi(getReqValue("display_mode1").c_str());
		hdec_cfg->output_mode1 = (unsigned char)std::atoi(getReqValue("output_mode1").c_str());
		hdec_cfg->output_type[0] = (unsigned char)std::atoi(getReqValue("output_type1").c_str());
		hdec_cfg->pj_no[0] =(unsigned char)std::atoi(getReqValue("pj_no1").c_str());
			
		hdec_cfg->display_mode2 = (unsigned char)std::atoi(getReqValue("display_mode2").c_str());
		hdec_cfg->output_mode2 = (unsigned char)std::atoi(getReqValue("output_mode2").c_str());
		hdec_cfg->output_type[1] = (unsigned char)std::atoi(getReqValue("output_type2").c_str());
		hdec_cfg->pj_no[1] =(unsigned char)std::atoi(getReqValue("pj_no2").c_str());

		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		m_desthtml = "";			
	}else if ( flag == "mode") {
		msg_header->msg_code = C_HDC_GETCMD;
		sprintf(send_buff+sizeof(MSG_HEAD_CMD), "CFG");
		msg_header->length = 3;
		if(comm::send_cmd(send_buff, recv_buff) != 0) 
			return ERROR_COMM_LOCAL;
	
		msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
		if(msgrsp_header->rsp_code != 0) 
			return(msgrsp_header->rsp_code);

		if(msgrsp_header->length != sizeof(HDEC_CFG))
			return(ERROR_COMM_PARSE);

		hdec_cfg = (HDEC_CFG*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
				
		addUserVariable("display_mode1", multilang::getDisplayMode(hdec_cfg->display_mode1));
		addUserVariable("display_mode2", multilang::getDisplayMode(hdec_cfg->display_mode2));
		addUserVariable("output_list", multilang::getVOutputList(hdec_cfg->display_mode1, hdec_cfg->display_mode2));
		
		m_desthtml = "hdmode_table.html";
		
	}else if (flag == "switch") {
		char *cmdptr;
		int disp_mode;

		msg_header->msg_code = C_HDC_SETCMD;
		cmdptr = send_buff+sizeof(MSG_HEAD_CMD);
		
  		std::string type = getReqValue("type");

		if(type == "d1")
		{
			disp_mode = std::atoi(getReqValue("display_mode1").c_str());
			if(genSwitchCmd(cmdptr, 0, disp_mode))
				return(ERROR_COMM_PARSE);
		
	  		msg_header->length = strlen(cmdptr);

			if(comm::send_cmd(send_buff, recv_buff) != 0) 
				return ERROR_COMM_LOCAL;
		
			msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
			if(msgrsp_header->rsp_code != 0) 
				return(msgrsp_header->rsp_code);
		} 
		else if(type == "d2")
		{
			disp_mode = std::atoi(getReqValue("display_mode2").c_str());
			if(genSwitchCmd(cmdptr, 1, disp_mode))
				return(ERROR_COMM_PARSE);
		
	  		msg_header->length = strlen(cmdptr);

			if(comm::send_cmd(send_buff, recv_buff) != 0) 
				return ERROR_COMM_LOCAL;
		
			msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
			if(msgrsp_header->rsp_code != 0) 
				return(msgrsp_header->rsp_code);
		}
		else  if(type == "v")
		{
			sprintf(cmdptr, "switch input=%s output=%s ", getReqValue("input_ch").c_str(), getReqValue("output_ch").c_str());
		
	  		msg_header->length = strlen(cmdptr);

			if(comm::send_cmd(send_buff, recv_buff) != 0) 
				return ERROR_COMM_LOCAL;
		
			msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
			if(msgrsp_header->rsp_code != 0) 
				return(msgrsp_header->rsp_code);
		}
		else 
			return ERROR_COMM_PARSE;

		m_desthtml = "";	
		
	}
	
	return 0;

}

void CHDCard::output()
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

	int ret = procHDReq();
	
	if(ret == 0) {
		if(m_desthtml == "")
			outputError(multilang::getError(0));
		else  if(outputHTMLFile(m_desthtml, true) != 0)
		{  
			outputError("Cann't open hd.html\n");
		}
	} 
	else 
		outputError(multilang::getError(ret));

	free(cs);

}



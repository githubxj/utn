#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#include "CgiRequest.h"
#include "CGISession.h"
#include "Lang.h"
#include "msg.h"
#include "Comm.h"
#include "const.h"
#include "CGetCard.h"

//**************add for VBR ********
#define PERFECT		17
#define VERY_GOOD 	20
#define GOOD 	  	25
#define MEDIUM	 	27
#define STANDARD	30
#define NOT_BAD		33
#define POOR		38

using namespace std;

const char * CGetCard::makeVersion(unsigned char high, unsigned char low)
{
	char static version[32];

	sprintf(version, "%c.%c", high, low);
	return(version);
}

const char *CGetCard::getCardName(unsigned char type)
{
	switch(type) {
		case CARD_TYPE_CTRL:
			return("SW&CTL-c");			
		case CARD_TYPE_FSA:
#ifdef COVOND
			return ("MAT1000-c");
#else
			return("FSA12500-c");
#endif
		case CARD_TYPE_DAT:
			return ("DAT1000-c");
		case CARD_TYPE_FE8:
			return("FE8-c");
		case CARD_TYPE_ENCODE:
			return ("H4EC-c");
		case CARD_TYPE_FSA600:
#ifdef COVOND
			return ("DAT600-c");
#else
			return ("FSA600-c");
#endif
		case CARD_TYPE_DECODE:
			return ("H4DC-c");
#ifdef __MTT__	// 201611
        case CARD_TYPE_MTT:
            return ("MTT");
#else
		case CARD_TYPE_NTT:
			return ("NTT2000-c");
#endif

#if 1 // 201708
        case CARD_TYPE_MTT_411:
            return ("MTT411");
            
        case CARD_TYPE_MTT_441:
            return ("MTT441");
#endif

        case CARD_TYPE_NTT_S:   // 20150115 add
                return ("NTT2000-s");
                        
		case CARD_TYPE_MS4V1H:
			return ("MS4V1H");
		case CARD_TYPE_M4S:
			return ("M4S");
		case CARD_TYPE_M4H:
			return ("M4H");
		case CARD_TYPE_EPN:
			return ("EPN2000");
		case CARD_TYPE_HDC:
			return ("HD-DEC");
		case CARD_TYPE_SDE:
			return ("SDI-ENC");
		case CARD_TYPE_MSD:
			return ("MS-DEC");
		default:
			return("Unknow");
	}
}


const char *CGetCard::getLink(unsigned char type)
{	switch(type)
	{		case 1:			return("Up");
			case 2:			return("Down");
			case 0:			return ("Down");
			default:			return("Unknown");
			}
}

const char *CGetCard::getSFPstate ( unsigned char state )
{	/* 1:inserted 0:removed*/
	switch(state)
	{		
			case 0:			return ("removed");
			case 1:			return("inserted&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  <a href='/cgi-bin/sfpinfo.cgi?sessionID=${sessionID}&cardno=${cardno}&onunode=${node_id}&sfpno=${node_id}'><IMG src='/images/sfp_info.gif'></a>");
			
			default:			return("Unknown");
	}

}

const char *CGetCard::getWorkingModefe8(unsigned char type)
{

		switch(type)
				{

					case 1:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' selected> Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 2:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' selected> 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 3:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' selected> 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 4:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' selected> 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");			
						case 5:
							return("<select class='fixsize'   name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' selected> 10M-Half</option>\
								      </select>");			
		
						default:
							return("<select class='fixsize'  name='fe_working_mode' id='fe_working_mode'> \
								      <option value='1' > Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      <option value='6' selected> not-supported</option></select>");	
		}
		
}

const char *CGetCard::getModeFE8(unsigned char type )
{
			switch(type)
			{
				case 1:
					return ("<select class='fixsize'  id='fe_port_mode' name='fe_port_mode' onchange='onWorkingModeChg(\"fe_port_mode\");'>\
						<option value='1' selected> learning mode</option>\	
						<option value='2' > bounded MAC</option>\
						</select>");
				case 2:
					return ("<select class='fixsize' id='fe_port_mode'  name='fe_port_mode' onchange='onWorkingModeChg(\"fe_port_mode\");'>\
						<option value='1' > learning mode</option>\	
						<option value='2' selected> bounded MAC</option>\
						</select>");
				default:
					return ("<select class='fixsize' id='fe_port_mode'  name='fe_port_mode' onchange='onWorkingModeChg(\"fe_port_mode\");'>\
							<option value='1' > learning mode</option>\	
							<option value='2' > bounded MAC</option>\
							<option value='3' selected>not-support</option>\
							</select>");
			
				}
	
					
	
}


const char *CGetCard::getProtocol(const char *protocol)
{
	if (!strcmp(protocol, "MULTI")) {
			return ("<select  id='protocol' name='protocol' class='fixsize' >\
						<option value='MULTI' selected>MULTI</option>\
						<option value='UDP'>UDP</option>\
						</select>");
	} 

	if (!strcmp(protocol, "UDP")) {
		return ("<select  id='protocol' name='protocol' class='fixsize' >\
					<option value='MULTI'>MULTI</option>\
					<option value='UDP' selected>UDP</option>\
					</select>");
	}  
	return("");
}	



const char *CGetCard::getChannelNO(unsigned char channelno)
{
	switch (channelno)
	{
		case 0:
			return ("<select class='fixsize'  name='channel_no' id='channel_no'> \
				     <option value='0' selected>channel 1</option>\
				     <option value='1' >channel 2</option>\
				</select>");
		case 1:
			return ("<select class='fixsize'  name='channel_no' id='channel_no'>\
				     <option value='0' >channel 1</option>\
				     <option value='1' selected>channel 2</option>\
				</select>");
		default:
		   	return ("<select class='fixsize'  name='channel_no' id='channel_no'>\
				     <option value='0' >channel 1</option>\
				     <option value='1' >channel 2</option>\
				     <option value='2' selected>Unkown channel</option>\
				</select>");
	}

}

const char *CGetCard::getChannel(unsigned char channelno)
{
	switch (channelno)
	{
		case 0:
			return "Channel 1";
		case 1:
			return "Channel 2";
		default :
			return "Unkown Channel";

	}


}
const char *CGetCard::getOSDLocation (unsigned char location)
{
	switch (location)
	{
		case 0:
			return ("<select class='fixsize'  name='location' id='location'>\
				     <option value='0' selected>Top-L:Top-R</option>\
				     <option value='1'>Top-L:Bottom-R</option>\
				     <option value='2'>Bottom-L:Top-R</option>\
				     <option value='3'>Bottom-L:Bottom-R</option>\
				     <option value='4'>Top-R:Top-L</option>\
				     <option value='5'>Bottom-R:Top-L</option>\
				     <option value='6'>Top-R:Bottom-L</option>\
				     <option value='7'>Bottom-R:Bottom-L</option>\
				</select>");

		case 1:
			return ("<select class='fixsize'  name='location' id='location'>\
				     <option value='0'>Top-L:Top-R</option>\
				     <option value='1' selected>Top-L:Bottom-R</option>\
				     <option value='2'>Bottom-L:Top-R</option>\
				     <option value='3'>Bottom-L:Bottom-R</option>\
				     <option value='4'>Top-R:Top-L</option>\
				     <option value='5'>Bottom-R:Top-L</option>\
				     <option value='6'>Top-R:Bottom-L</option>\
				     <option value='7'>Bottom-R:Bottom-L</option>\
				</select>");	


		case 2:
			return ("<select class='fixsize'  name='location' id='location'>\
				     <option value='0'>Top-L:Top-R</option>\
				     <option value='1'>Top-L:Bottom-R</option>\
				     <option value='2' selected>Bottom-L:Top-R</option>\
				     <option value='3'>Bottom-L:Bottom-R</option>\
				     <option value='4'>Top-R:Top-L</option>\
				     <option value='5'>Bottom-R:Top-L</option>\
				     <option value='6'>Top-R:Bottom-L</option>\
				     <option value='7'>Bottom-R:Bottom-L</option>\
				</select>");
		case 3:
			return ("<select class='fixsize'  name='location' id='location'>\
				     <option value='0'>Top-L:Top-R</option>\
				     <option value='1'>Top-L:Bottom-R</option>\
				     <option value='2'>Bottom-L:Top-R</option>\
				     <option value='3' selected>Bottom-L:Bottom-R</option>\
				     <option value='4'>Top-R:Top-L</option>\
				     <option value='5'>Bottom-R:Top-L</option>\
				     <option value='6'>Top-R:Bottom-L</option>\
				     <option value='7'>Bottom-R:Bottom-L</option>\
				</select>");

		case 4:
			return ("<select class='fixsize'  name='location' id='location'>\
				     <option value='0'>Top-L:Top-R</option>\
				     <option value='1'>Top-L:Bottom-R</option>\
				     <option value='2'>Bottom-L:Top-R</option>\
				     <option value='3'>Bottom-L:Bottom-R</option>\
				     <option value='4' selected>Top-R:Top-L</option>\
				     <option value='5'>Bottom-R:Top-L</option>\
				     <option value='6'>Top-R:Bottom-L</option>\
				     <option value='7'>Bottom-R:Bottom-L</option>\
				</select>");
		case 5:
			return ("<select class='fixsize'  name='location' id='location'>\
				     <option value='0'>Top-L:Top-R</option>\
				     <option value='1'>Top-L:Bottom-R</option>\
				     <option value='2'>Bottom-L:Top-R</option>\
				     <option value='3'>Bottom-L:Bottom-R</option>\
				     <option value='4'>Top-R:Top-L</option>\
				     <option value='5' selected>Bottom-R:Top-L</option>\
				     <option value='6'>Top-R:Bottom-L</option>\
				     <option value='7'>Bottom-R:Bottom-L</option>\
				</select>");

		case 6:
			return ("<select  class='fixsize' name='location' id='location'>\
				     <option value='0'>Top-L:Top-R</option>\
				     <option value='1'>Top-L:Bottom-R</option>\
				     <option value='2'>Bottom-L:Top-R</option>\
				     <option value='3'>Bottom-L:Bottom-R</option>\
				     <option value='4'>Top-R:Top-L</option>\
				     <option value='5'>Bottom-R:Top-L</option>\
				     <option value='6' selected>Top-R:Bottom-L</option>\
				     <option value='7'>Bottom-R:Bottom-L</option>\
				</select>");

		case 7:
			return ("<select class='fixsize' name='location' id='location'>\
				     <option value='0'>Top-L:Top-R</option>\
				     <option value='1'>Top-L:Bottom-R</option>\
				     <option value='2'>Bottom-L:Top-R</option>\
				     <option value='3'>Bottom-L:Bottom-R</option>\
				     <option value='4'>Top-R:Top-L</option>\
				     <option value='5'>Bottom-R:Top-L</option>\
				     <option value='6'>Top-R:Bottom-L</option>\
				     <option value='7' selected>Bottom-R:Bottom-L</option>\
				</select>");
		default:
			return ("<select class='fixsize'  name='location' id='location'>\
				     <option value='0'>Top-L:Top-R</option>\
				     <option value='1'>Top-L:Bottom-R</option>\
				     <option value='2'>Bottom-L:Top-R</option>\
				     <option value='3'>Bottom-L:Bottom-R</option>\
				     <option value='4'>Top-R:Top-L</option>\
				     <option value='5'>Bottom-R:Top-L</option>\
				     <option value='6'>Top-R:Bottom-L</option>\
				     <option value='7'>Bottom-R:Bottom-L</option>\
				     <option value='8' selected>Unkown Location</option>\
				</select>");
			
	}
}
const char *CGetCard::getOSDColor(unsigned char color)
{
	switch ( color)
	{
		case 0:
			return ("<select class='fixsize' name='osd_color'  id='osd_color' >\
					<option value='0' selected>	White	</option>\
					<option value='1'>	Blue	</option>\
					<option value='2'>	Yellow	</option>\
					<option value='3'>	Green	</option>\
					<option value='4'>  Magenta	</option>\
					<option value='5'>	Cyan	</option>\
					<option value='6'>	Red	</option>\
					<option value='7'>	Black	</option>\
					</select>	");
		case 1:
			return ("<select class='fixsize' name='osd_color' id='osd_color' >\
					<option value='0'>	White	</option>\
					<option value='1' selected>	Blue	</option>\
					<option value='2'>	Yellow	</option>\
					<option value='3'>	Green	</option>\
					<option value='4'>  Magenta	</option>\
					<option value='5'>	Cyan	</option>\
					<option value='6'>	Red	</option>\
					<option value='7'>	Black	</option>\
					</select>	");
		case 2:
			return ("<select class='fixsize' name='osd_color' id='osd_color' >\
					<option value='0'>	White	</option>\
					<option value='1'>	Blue	</option>\
					<option value='2' selected>	Yellow	</option>\
					<option value='3'>	Green	</option>\
					<option value='4'>  Magenta	</option>\
					<option value='5'>	Cyan	</option>\
					<option value='6'>	Red	</option>\
					<option value='7'>	Black	</option>\
					</select>	");
		case 3:
			return ("<select class='fixsize' name='osd_color' id='osd_color' >\
					<option value='0'>	White	</option>\
					<option value='1'>	Blue	</option>\
					<option value='2'>	Yellow	</option>\
					<option value='3' selected>	Green	</option>\
					<option value='4'>  Magenta	</option>\
					<option value='5'>	Cyan	</option>\
					<option value='6'>	Red	</option>\
					<option value='7'>	Black	</option>\
					</select>	");
		case 4:
			return ("<select class='fixsize' name='osd_color' id='osd_color' >\
					<option value='0'>	White	</option>\
					<option value='1'>	Blue	</option>\
					<option value='2'>	Yellow	</option>\
					<option value='3'>	Green	</option>\
					<option value='4' selected>  Magenta	</option>\
					<option value='5'>	Cyan	</option>\
					<option value='6'>	Red	</option>\
					<option value='7'>	Black	</option>\
					</select>	");
		case 5:
			return ("<select class='fixsize' name='osd_color' id='osd_color' >\
					<option value='0'>	White	</option>\
					<option value='1'>	Blue	</option>\
					<option value='2'>	Yellow	</option>\
					<option value='3'>	Green	</option>\
					<option value='4'>  Magenta	</option>\
					<option value='5' selected>	Cyan	</option>\
					<option value='6'>	Red	</option>\
					<option value='7'>	Black	</option>\
					</select>	");
		case 6:
			return ("<select class='fixsize' name='osd_color' id='osd_color' >\
					<option value='0'>	White	</option>\
					<option value='1'>	Blue	</option>\
					<option value='2'>	Yellow	</option>\
					<option value='3'>	Green	</option>\
					<option value='4'>  Magenta	</option>\
					<option value='5'>	Cyan	</option>\
					<option value='6' selected>	Red	</option>\
					<option value='7'>	Black	</option>\
					</select>	");
		case 7:
			return ("<select class='fixsize' name='osd_color' id='osd_color' >\
					<option value='0'>	White	</option>\
					<option value='1'>	Blue	</option>\
					<option value='2'>	Yellow	</option>\
					<option value='3'>	Green	</option>\
					<option value='4'>  Magenta	</option>\
					<option value='5'>	Cyan	</option>\
					<option value='6'>	Red	</option>\
					<option value='7' selected>	Black	</option>\
					</select>	");
		default :
			return ("<select class='fixsize' name='osd_color' id='osd_color' >\
					<option value='0'>	White	</option>\
					<option value='1'>	Blue	</option>\
					<option value='2'>	Yellow	</option>\
					<option value='3'>	Green	</option>\
					<option value='4'>	Magenta	</option>\
					<option value='5'>	Cyan	</option>\
					<option value='6'>	Red	</option>\
					<option value='7'>	Black	</option>\
					<option value='8' selected>Not-Support</option>\
					</select>");
		
	}

}
const char *CGetCard::showTime(unsigned char state)
{
	if ( 1 == state )
	{
		return ("<select class='fixsize'  name='show_time' id='show_time'>\
				<option value='1' selected>ON</option>\
				<option value='2'>OFF</option>\
		</select>");
	}
	else 
	{
		return ("<select class='fixsize'  name='show_time' id='show_time'>\
				<option value='1' >ON</option>\
				<option value='2' selected>OFF</option>\
		</select>");
	}

}

const char *CGetCard::showVlost(unsigned char vlost)
{
	if ( 1 == vlost)
	{
		return ("<select class='fixsize'  name='vlost'  id='vlost'>\
				<option value='1' selected>ON</option>\
				<option value='2'>OFF</option>\
		</select>");
	}
	else 
	{
		return ("<select class='fixsize'  name='vlost'  id='vlost'>\
				<option value='1' >ON</option>\
				<option value='2' selected>OFF</option>\
		</select>");
	}

}

const char *CGetCard::getCompresstype ( unsigned char type )
{
	switch ( type )
	{

		case 1:
			return ("<select class='fixsize' name='compress_type' id='compress_type'  >\
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
			return ("<select class='fixsize' name='compress_type' id='compress_type' >\
				      <option value='1'  >MPEG2</option>\
				      <option value='2'>MPEG4</option>\
				      <option value='3' selected >H264</option>\
				      <option value='4' >AVS</option>\
				</selected>");
		case 4:
			return ("<select class='fixsize' name='compress_type' id='compress_type' >\
				      <option value='1'  >MPEG2</option>\
				      <option value='2'>MPEG4</option>\
				      <option value='3'>H264</option>\
				      <option value='4' selected>AVS</option>\
				</selected>");
		default:
			return ("<select class='fixsize' name='compress_type' id='compress_type' >\
				      <option value='1'  >MPEG2</option>\
				      <option value='2'>MPEG4</option>\
				      <option value='3'>H264</option>\
				      <option value='4' >AVS</option>\
				      <option value='5' selected>Not-Support</option>\
				</selected>");
				
				
				

	}



}

const char *CGetCard::getStreamtype ( unsigned char type )
{
	switch ( type )
	{

		case 1:
			return ("<select class='fixsize' name='stream_type' id='stream_type' >\
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


const char *CGetCard::get_gop(unsigned char gop)
	{
		switch(gop) {
			case 5:
					return ("<select class='fixsize' name='gop' id='gop'>\
						<option value='5' selected> 5	</option>\
						<option value='10' >	10	</option>\
						<option value='15' >	15	</option>\
						<option value='30' >	30	</option>\
						<option value='60' >	60	</option>\
						<option value='120' >	120 </option>\
						</select>	");
			case 10:
					return ("<select class='fixsize' name='gop'  id='gop'>\
						<option value='5' > 5	</option>\
						<option value='10' selected>	10	</option>\
						<option value='15' >	15	</option>\
						<option value='30' >	30	</option>\
						<option value='60' >	60	</option>\
						<option value='120' >	120 </option>\
						</select>	");
			case 15:
					return ("<select class='fixsize' name='gop'  id='gop'>\
						<option value='5' > 5	</option>\
						<option value='10' >	10	</option>\
						<option value='15' selected>	15	</option>\
						<option value='30' >	30	</option>\
						<option value='60' >	60	</option>\
						<option value='120' >	120 </option>\
						</select>	");
			case 30:
					return ("<select class='fixsize' name='gop'  id='gop'>\
						<option value='5' > 5	</option>\
						<option value='10' >	10	</option>\
						<option value='15' >	15	</option>\
						<option value='30' selected>	30	</option>\
						<option value='60' >	60	</option>\
						<option value='120' >	120 </option>\
						</select>	");
			case 60:
					return ("<select class='fixsize' name='gop'  id='gop'>\
						<option value='5' > 5	</option>\
						<option value='10' >	10	</option>\
						<option value='15' >	15	</option>\
						<option value='30' >	30	</option>\
						<option value='60' selected>	60	</option>\
						<option value='120' >	120 </option>\
						</select>	");
			case 120:
					return ("<select class='fixsize' name='gop'  id='gop'>\
						<option value='5' > 5	</option>\
						<option value='10' >	10	</option>\
						<option value='15' >	15	</option>\
						<option value='30' >	30	</option>\
						<option value='60' >	60	</option>\
						<option value='120' selected>	120 </option>\
						</select>	");
			default:
					return ("<select class='fixsize' name='gop'  id='gop'>\
						<option value='5' > 5	</option>\
						<option value='10' >	10	</option>\
						<option value='15' >	15	</option>\
						<option value='30' >	30	</option>\
						<option value='60' >	60	</option>\
						<option value='120' >	120 </option>\
						<option value='240' selected>Not-Support</option>\
						</select>	");
		}
	}


const char *CGetCard::get_resolving(const char *resolving)
{
	if(!strcmp(resolving, "720/576")) {
		
			return ("<select class='fixsize' name='resolving' id='resolving'>\
					<option value='720/576' selected>	720/576[D1]	</option>\
					<option value='704/576' >	704/576[ 4CIF ] 	</option>\
					<option value='352/288' >	352/288[ CIF ] 	</option>\
					<option value='176/144' >	176/144[ QCIF ] 	</option>\
					<option value='720/480' >	720/480[ D1(NTSC) ] 	</option>\
					<option value='352/240' >	352/240[ CIF(NTSC) ] 	</option>\
					</select>	");
	}
	if(!strcmp(resolving, "704/576")) {
		
			return ("<select class='fixsize' name='resolving'  id='resolving'>\
					<option value='720/576' >	720/576[D1]	</option>\
					<option value='704/576' selected>	704/576[ 4CIF ] 	</option>\
					<option value='352/288' >	352/288[ CIF ] 	</option>\
					<option value='176/144' >	176/144[ QCIF ] 	</option>\
					<option value='720/480' >	720/480[ D1(NTSC) ] 	</option>\
					<option value='352/240' >	352/240[ CIF(NTSC) ] 	</option>\
					</select>	");
	}
	if(!strcmp(resolving, "352/288")) {
		
			return ("<select class='fixsize' name='resolving'  id='resolving'>\
					<option value='720/576' >	720/576[D1]	</option>\
					<option value='704/576' >	704/576[ 4CIF ] 	</option>\
					<option value='352/288' selected>	352/288[ CIF ] 	</option>\
					<option value='176/144' >	176/144[ QCIF ] 	</option>\
					<option value='720/480' >	720/480[ D1(NTSC) ] 	</option>\
					<option value='352/240' >	352/240[ CIF(NTSC) ] 	</option>\
					</select>	");
	}
	if(!strcmp(resolving, "176/144")) {
		
			return ("<select class='fixsize' name='resolving'  id='resolving'>\
					<option value='720/576' >	720/576[D1]	</option>\
					<option value='704/576' >	704/576[ 4CIF ] 	</option>\
					<option value='352/288' >	352/288[ CIF ] 	</option>\
					<option value='176/144' selected>	176/144[ QCIF ] 	</option>\
					<option value='720/480' >	720/480[ D1(NTSC) ] 	</option>\
					<option value='352/240' >	352/240[ CIF(NTSC) ] 	</option>\
					</select>	");
	}
	if(!strcmp(resolving, "720/480")) {
		
			return ("<select class='fixsize' name='resolving'  id='resolving'>\
					<option value='720/576' >	720/576[D1]	</option>\
					<option value='704/576' >	704/576[ 4CIF ] 	</option>\
					<option value='352/288' >	352/288[ CIF ] 	</option>\
					<option value='176/144' >	176/144[ QCIF ] 	</option>\
					<option value='720/480' selected>	720/480[ D1(NTSC) ] 	</option>\
					<option value='352/240' >	352/240[ CIF(NTSC) ] 	</option>\
					</select>	");
	}

	if(!strcmp(resolving, "352/240")) {
		
			return ("<select class='fixsize' name='resolving'  id='resolving'>\
					<option value='720/576' >	720/576[D1]	</option>\
					<option value='704/576' >	704/576[ 4CIF ] 	</option>\
					<option value='352/288' >	352/288[ CIF ] 	</option>\
					<option value='176/144' >	176/144[ QCIF ] 	</option>\
					<option value='720/480' >	720/480[ D1(NTSC) ] 	</option>\
					<option value='352/240' selected>	352/240[ CIF(NTSC) ] 	</option>\
					</select>	");
	}
	return("");
}


const char *CGetCard::get_vid_bit_type(unsigned char vid_bit_type)
{		
	if (!vid_bit_type) {
		return("<select class='fixsize' id='vid_bit_type'  name='vid_bit_type' onchange='onloadcbr();'> \
						<option value='0' selected>	CBR	</option>\
						<option value='1' >	VBR 	</option>\
				</select>");			
	}else {
		return("<select class='fixsize' id='vid_bit_type'  name='vid_bit_type' onchange='onloadcbr();'> \
						<option value='0' >	CBR	</option>\
						<option value='1' selected>	VBR 	</option>\
				</select>");	
	}
}

const char *CGetCard::show_cbr(unsigned char vid_bit_type)
{		
	if (!vid_bit_type) {
		return("hidden");			
	}else {
		return("visible");	
	}
}

const char *CGetCard::get_vbr(unsigned char vbr)
{	
	
		if (!vbr) {
			return ("<select class='fixsize'  name='vid_vbr' id='vid_vbr'> \
						<option value='1' selected>	Perfect!	</option>\
						<option value='2' >	Very GOOD! 	</option>\
						<option value='3' >	GOOD! 	</option>\
						<option value='4' >	Medium! 	</option>\
						<option value='5' >	Standard! 	</option>\
						<option value='6' >	Not Bad! 	</option>\
						<option value='7' >	Poor! 	</option>\
					</select>");		
		}else {
			switch(vbr) {
				case 1:
						return ("<select class='fixsize'  name='vid_vbr' id='vid_vbr'> \
									<option value='1' selected>	Perfect!	</option>\
									<option value='2' >	Very GOOD! 	</option>\
									<option value='3' >	GOOD! 	</option>\
									<option value='4' >	Medium! 	</option>\
									<option value='5' >	Standard! 	</option>\
									<option value='6' >	Not Bad! 	</option>\
									<option value='7' >	Poor! 	</option>\
								</select>");		
				
					case 2:
						return ("<select class='fixsize'  name='vid_vbr'  id='vid_vbr'> \
									<option value='1' >	Perfect!	</option>\
									<option value='2' selected>	Very GOOD! 	</option>\
									<option value='3' >	GOOD! 	</option>\
									<option value='4' >	Medium! 	</option>\
									<option value='5' >	Standard! 	</option>\
									<option value='6' >	Not Bad! 	</option>\
									<option value='7' >	Poor! 	</option>\
								</select>");
				case 3:
						return ("<select class='fixsize'  name='vid_vbr'  id='vid_vbr'> \
									<option value='1' >	Perfect!	</option>\
									<option value='2' >	Very GOOD! 	</option>\
									<option value='3' selected>	GOOD! 	</option>\
									<option value='4' >	Medium! 	</option>\
									<option value='5' >	Standard! 	</option>\
									<option value='6' >	Not Bad! 	</option>\
									<option value='7' >	Poor! 	</option>\
								</select>");
				case 4:
						return ("<select class='fixsize'  name='vid_vbr'  id='vid_vbr'> \
									<option value='1' >	Perfect!	</option>\
									<option value='2' >	Very GOOD! 	</option>\
									<option value='3' >	GOOD! 	</option>\
									<option value='4' selected>	Medium! 	</option>\
									<option value='5' >	Standard! 	</option>\
									<option value='6' >	Not Bad! 	</option>\
									<option value='7' >	Poor! 	</option>\
								</select>");
				case 5:
						return ("<select class='fixsize'  name='vid_vbr'  id='vid_vbr'> \
									<option value='1' >	Perfect!	</option>\
									<option value='2' >	Very GOOD! 	</option>\
									<option value='3' >	GOOD! 	</option>\
									<option value='4' >	Medium! 	</option>\
									<option value='5' selected>	Standard! 	</option>\
									<option value='6' >	Not Bad! 	</option>\
									<option value='7' >	Poor! 	</option>\
								</select>");
				case 6:
						return ("<select class='fixsize'  name='vid_vbr'  id='vid_vbr'> \
									<option value='1' >	Perfect!	</option>\
									<option value='2' >	Very GOOD! 	</option>\
									<option value='3' >	GOOD! 	</option>\
									<option value='4' >	Medium! 	</option>\
									<option value='5' >	Standard! 	</option>\
									<option value='6' selected>	Not Bad! 	</option>\
									<option value='7' >	Poor! 	</option>\
								</select>");
				case 7:
						return ("<select class='fixsize'  name='vid_vbr'  id='vid_vbr'> \
									<option value='1' >	Perfect!	</option>\
									<option value='2' >	Very GOOD! 	</option>\
									<option value='3' >	GOOD! 	</option>\
									<option value='4' >	Medium! 	</option>\
									<option value='5' >	Standard! 	</option>\
									<option value='6' >	Not Bad! 	</option>\
									<option value='7' selected>	Poor! 	</option>\
								</select>");		
						
			}
		}
		return("");
}



const char *CGetCard::getLockstate(unsigned char state)
{
	switch (state)
	{
		case 0:
			return ("<select class='fixsize' name='port_lock'  id='port_lock'>\
				   <option value='0' selected >Unlock</option>\
				   <option value='1' >Lock</option>\
				   </select>");
		case 1:
			return ("<select class='fixsize' name='port_lock' id='port_lock'>\
				   <option value='0' >Unlock</option>\
				   <option value='1' selected  >Lock</option>\
				   </select>");
		default:
			return ("<select class='fixsize' name='port_lock' id='port_lock'>\
				   <option value='0' >Unlock</option>\
				   <option value='1'  >Lock</option>\
				   <option value='2' selected >Not-Support</option>\
				   </select>");

	}


}

const char *CGetCard::getTransimitmode(unsigned char mode , unsigned char type)//type: 1,fsa600, else onu100.
{
	if ( 1 == type )
	{
		switch (mode)
		{
			case 0:
				return ("<select class='fixsize' name='transmit_mode' id='transmit_mode'>\
					   <option value='0' selected >store-forward</option>\
					   <option value='1' >cut-through</option>\
					   </select>");
			case 1:
				return ("<select class='fixsize' name='transmit_mode' id='transmit_mode'>\
					   <option value='0'  >store-forward</option>\
					   <option value='1' selected >cut-through</option>\
					   </select>");
			default:
				return ("<select class='fixsize' name='transmit_mode' id='transmit_mode'>\
					   <option value='0' >store-forward</option>\
					   <option value='1'  >cut-through</option>\
					   <option value='2' selected >Not-Support</option>\
					   </select>");

		}
	}
	else{

		switch (mode)
		{
			case 1:
				return ("<select class='fixsize' name='transmit_mode' id='transmit_mode'>\
					   <option value='1' selected >cut-through</option>\
					   <option value='2' >store-forward</option>\
					   </select>");
			case 2:
				return ("<select class='fixsize' name='transmit_mode' id='transmit_mode'>\
					   <option value='1'  >cut-through</option>\
					   <option value='2' selected>store-forward</option>\
					   </select>");
			default:
				return ("<select class='fixsize' name='transmit_mode' id='transmit_mode'>\
					   <option value='1'  >cut-through</option>\
					   <option value='2' >store-forward</option>\
					   <option value='3' selected >Not-Support</option>\
					   </select>");

		}

	}
}

const char *CGetCard::getWorkingModefsa600rmt(unsigned char type, int portno, int readonly)
{

	if (readonly)
	{
		switch (portno)
		{
			case 1:
				switch (type)
				{
					case 1:
						return "Auto";
					case 2:
						return "100M-Full";
					case 3:
						return "100M-Half";
					case 4:
						return "10M-Full";
					case 5:
						return "10M-Half";
					default :
						return "Not-Support";
				}

			case 2:
				switch (type)
				{
					case 1:
						return "Auto";
					case 2:
						return "100M-Full";
					case 3:
						return "100M-Half";
					case 4:
						return "10M-Full";
					case 5:
						return "10M-Half";
					default :
						return "Not-Support";
				}	
		}

	}else {

		switch ( portno)
		{
			case 1:
				switch ( type )
				{
					case 1:
						return("<select  class='fixsize'  name='conf_workmode_onu1'  id='conf_workmode_onu1'> \
								      <option value='1' selected> Auto</option>\
								      <option value='2' > 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");
					case 2:
						return("<select  class='fixsize'  name='conf_workmode_onu1' id='conf_workmode_onu1'> \
								      <option value='1' > Auto</option>\
								      <option value='2' selected> 100M-Full</option>\
								      <option value='3' > 100M-Half</option>\
								      <option value='4' > 10M-Full</option>\
								      <option value='5' > 10M-Half</option>\
								      </select>");
					case 3:
							return("<select  class='fixsize'  name='conf_workmode_onu1' id='conf_workmode_onu1'> \
									      <option value='1' > Auto</option>\
									      <option value='2' > 100M-Full</option>\
									      <option value='3' selected> 100M-Half</option>\
									      <option value='4' > 10M-Full</option>\
									      <option value='5' > 10M-Half</option>\
									      </select>");
					case 4:
							return("<select  class='fixsize'  name='conf_workmode_onu1' id='conf_workmode_onu1'> \
									      <option value='1' > Auto</option>\
									      <option value='2' > 100M-Full</option>\
									      <option value='3' > 100M-Half</option>\
									      <option value='4' selected> 10M-Full</option>\
									      <option value='5' > 10M-Half</option>\
									      </select>");
					case 5:
							return("<select  class='fixsize'  name='conf_workmode_onu1' id='conf_workmode_onu1'> \
									      <option value='1' > Auto</option>\
									      <option value='2' > 100M-Full</option>\
									      <option value='3' > 100M-Half</option>\
									      <option value='4' > 10M-Full</option>\
									      <option value='5' selected> 10M-Half</option>\
									      </select>");
					default:
							return("<select  class='fixsize'  name='conf_workmode_onu1' id='conf_workmode_onu1'> \
									      <option value='1' > Auto</option>\
									      <option value='2' > 100M-Full</option>\
									      <option value='3' > 100M-Half</option>\
									      <option value='4' > 10M-Full</option>\
									      <option value='5' > 10M-Half</option>\
									      <option value='6' selected>Not-Support</option>\
									      </select>");
				}

			case 2:
					switch ( type )
					{
						case 1:
							return("<select  class='fixsize'  name='conf_workmode_onu2' id='conf_workmode_onu2'> \
									      <option value='1' selected> Auto</option>\
									      <option value='2' > 100M-Full</option>\
									      <option value='3' > 100M-Half</option>\
									      <option value='4' > 10M-Full</option>\
									      <option value='5' > 10M-Half</option>\
									      </select>");
						case 2:
							return("<select  class='fixsize'  name='conf_workmode_onu2' id='conf_workmode_onu2'> \
									      <option value='1' > Auto</option>\
									      <option value='2' selected> 100M-Full</option>\
									      <option value='3' > 100M-Half</option>\
									      <option value='4' > 10M-Full</option>\
									      <option value='5' > 10M-Half</option>\
									      </select>");
						case 3:
								return("<select  class='fixsize'  name='conf_workmode_onu2' id='conf_workmode_onu2'> \
										      <option value='1' > Auto</option>\
										      <option value='2' > 100M-Full</option>\
										      <option value='3' selected> 100M-Half</option>\
										      <option value='4' > 10M-Full</option>\
										      <option value='5' > 10M-Half</option>\
										      </select>");
						case 4:
								return("<select  class='fixsize'  name='conf_workmode_onu2' id='conf_workmode_onu2'> \
										      <option value='1' > Auto</option>\
										      <option value='2' > 100M-Full</option>\
										      <option value='3' > 100M-Half</option>\
										      <option value='4' selected> 10M-Full</option>\
										      <option value='5' > 10M-Half</option>\
										      </select>");
						case 5:
								return("<select  class='fixsize'  name='conf_workmode_onu2' id='conf_workmode_onu2'> \
										      <option value='1' > Auto</option>\
										      <option value='2' > 100M-Full</option>\
										      <option value='3' > 100M-Half</option>\
										      <option value='4' > 10M-Full</option>\
										      <option value='5' selected> 10M-Half</option>\
										      </select>");
						default:
								return("<select  class='fixsize'  name='conf_workmode_onu2' id='conf_workmode_onu2'> \
										      <option value='1' > Auto</option>\
										      <option value='2' > 100M-Full</option>\
										      <option value='3' > 100M-Half</option>\
										      <option value='4' > 10M-Full</option>\
										      <option value='5' > 10M-Half</option>\
										      <option value='6' selected>Not-Support</option>\
										      </select>");
					}
			default :
				return "Unkown Port";
				
		
		}

	}
	return ("");
}

int CGetCard::getCardData()
{
	std::string card_no = getReqValue("cardno");
	std::string flag = getReqValue("flag");
	std::string onuno = getReqValue ("onuid");
	std::string sfpno = getReqValue ( "sfpIndex");
	std::string portno = getReqValue ("portno");
	std::string ntuno = getReqValue("ntuno");
	
	char send_buff[MAX_CMDBUFF_SIZE];
	char recv_buff[MAX_CMDBUFF_SIZE];
	MSG_HEAD_CMD *msg_header;
	MSG_HEAD_CMD_RSP *msgrsp_header;
	CARD_INFO *	card_info;
	BROADCASTSTORM_CTRL_FE *broadcastStormInfo;
	FE_PORT_STATE *fe8portstate;
	FE_PORT_STATE *fe8portstatesend;
	ENCODE_VIDEO_STATE *videostate;
	ENCODE_NET_STATE *netstate;
	ENCODE_PROTOCOL_STATE *protocolstate;
	ENCODE_OSD_STATE *osdstate;
	ENCODE_PICTURE_STATE *picturestate;
	ENCODE_SYSTIME *sys_time;
	ENCODE_NTP *sys_ntp;
	ENCODE_MODE_STATE *patten;
	FSA600_PORT_485 *fsa600state;
	FSA600_GETRMT *remotestate;
	ONU_INFO *onu100Info;
	SW_PORT_STATE *swport;
	SW_CONFIG *swconfig;
	NTT_STATE *nttstate;

    MTT_STATE *mttstate;  // 201611	

#if 1 // 201708    
    MTT_411_STATE *mtt411state;
    MTT_441_STATE *mtt441state;
#endif

	FE_PORT_STATE *nttinfo;
	int Index;
	stringstream outss;
	char field_name[32];
	
	msg_header = (MSG_HEAD_CMD *)send_buff;
	memset(msg_header, 0, sizeof(MSG_HEAD_CMD));

	msg_header->msg_code = C_CARD_GET;
	msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
	
	if(comm::send_cmd(send_buff, recv_buff) != 0) 
		return ERROR_COMM_LOCAL;
	
	msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
	if(msgrsp_header->rsp_code != 0) 
		return(msgrsp_header->rsp_code);

	if(msgrsp_header->length != sizeof(CARD_INFO))
		return(ERROR_COMM_PARSE);

	card_info = (CARD_INFO*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
	
	addUserVariable("card_type", getCardName(card_info->card_type));
	char buff[256];
	sprintf(buff,"%d", msgrsp_header->card_no);
	addUserVariable("card_no", buff);
	memcpy (buff, card_info->card_name , MAX_DISPLAYNAME_LEN);
	buff[MAX_DISPLAYNAME_LEN] = '\0';
	addUserVariable("card_name", buff);
	
	addAdminStateVariable("admin_state", card_info->admin_state);
	addUserVariable("version_hw", makeVersion(card_info->version_HW[0], card_info->version_HW[1]));
	addUserVariable("version_fw", makeVersion(card_info->version_FW[0], card_info->version_FW[1]));
	addUserVariable("version_sw", makeVersion(card_info->version_SW[0], card_info->version_SW[1]));
	card_info->version_OS[MAX_DISPLAYNAME_LEN -1] = '\0';
	addUserVariable("version_os", card_info->version_OS);
	if (card_info->card_type == CARD_TYPE_FE8 )
	{
		addUserVariable("node_id", "0");
	}
	
	m_desthtml = "card.html";
	switch(card_info->card_type) {
		case CARD_TYPE_CTRL:
			if ( flag == "getport" )
			{
				msg_header->msg_code = C_SW_GETPORTSTATE;
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(SW_PORT_STATE))
					return(ERROR_COMM_PARSE);

				swport = (SW_PORT_STATE *)(recv_buff + sizeof( MSG_HEAD_CMD_RSP ));
				for ( Index = 0; Index < 26; Index++ )
				{
					if ( ( 24 == Index) || ( 25 == Index ))
					{
						sprintf(buff, "ge%d_state", Index - 23);
						sprintf(field_name, "ge%d_mode", Index - 23);
					}
					else
					{
						sprintf(buff, "port%d_state", Index + 1);
						sprintf(field_name, "port%d_mode", Index + 1);
					}

					addUserVariable(buff, getLink(swport->port_state[Index].port_state));
					if ( 1 == swport->port_state[Index].port_state )//up
					{
						switch( swport->port_state[Index].portWorkingMode )
						{
							case ETH_AUTO:
								addUserVariable(field_name , "AUTO");
								break;
							case ETH_100MFULL:
								addUserVariable(field_name , "100M Full");
								break;
							case ETH_100MHALF:
								addUserVariable(field_name , "100M Half");
								break;
							case ETH_10MFULL:
								addUserVariable(field_name , "10M Full");
								break;
							case ETH_10MHALF:
								addUserVariable(field_name , "10M Half");
								break;
							case ETH_1000M:
								addUserVariable(field_name , "1000M");
								break;
							default:
								addUserVariable(field_name , "NA");
								break;
									
						}
					}
					else
					{
						addUserVariable(field_name , "-");

					}		

				}
				
				m_desthtml = "swportstate_table.html";
			}
			else if ( flag == "getparam" )
			{
				msg_header->msg_code = C_SW_GETPARAM ;
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(SW_CONFIG))
					return(ERROR_COMM_PARSE);

				swconfig = (SW_CONFIG *)(recv_buff + sizeof( MSG_HEAD_CMD_RSP ));
				if(swconfig->qry_time == 0)
				{
					addUserVariable("igmp_qrytime", "");
				} else {
					addUserVariable("query_checked", "checked");
					sprintf(buff, "%u", swconfig->qry_time);
					addUserVariable("igmp_qrytime", buff);
				}
				if(swconfig->igmp_timeout== 0)
				{
					addUserVariable("igmp_timeout", "");
				} else {
					addUserVariable("timeout_checked", "checked");
					sprintf(buff, "%u", swconfig->igmp_timeout);
					addUserVariable("igmp_timeout", buff);
				}

				/*port G1*/
				if(swconfig->igmp_qportmap & 0x1000000)
					addUserVariable("qp1_checked", "checked");
				if(swconfig->igmp_rportmap & 0x1000000)
					addUserVariable("rp1_checked", "checked");
				
				/*port G2*/
				if(swconfig->igmp_qportmap & 0x2000000)
					addUserVariable("qp2_checked", "checked");
				if(swconfig->igmp_rportmap & 0x2000000)
					addUserVariable("rp2_checked", "checked");
				
				m_desthtml = "param_table.html";

			}
			break;
		case CARD_TYPE_FSA:
				m_desthtml = "cardfsa.html";
			break;
		case CARD_TYPE_DAT:
				m_desthtml = "carddat.html";
			break;
		case CARD_TYPE_HDC:
		case CARD_TYPE_SDE:
		case CARD_TYPE_MSD:
				m_desthtml = "cardhdc.html";
			break;
		case CARD_TYPE_EPN:
			if ( flag == "stat" )
			{
				EPN_CARD_STATE  *epn_stat;
				msg_header->msg_code = C_EP_GET_OLTSTAT;
				msg_header->length = 0;
			
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
			
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(EPN_CARD_STATE))
					return(ERROR_COMM_PARSE);
				
				epn_stat = (EPN_CARD_STATE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
				addUserVariable ("pon1_state", multilang::getSfpState(epn_stat->fiber1_state)); 
				addUserVariable ("pon2_state", multilang::getSfpState(epn_stat->fiber2_state)); 

				m_desthtml = "epnbase_table.html";
			}else if ( flag == "vlan" )
			{
				EPN_OLT_VLAN  *epn_vlan;
				msg_header->msg_code = C_EP_GET_OLTVLAN;
				msg_header->length = 0;
			
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
			
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(EPN_OLT_VLAN))
					return(ERROR_COMM_PARSE);
				
				epn_vlan= (EPN_OLT_VLAN*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
				for(int i=0; i<5; i++)
				{
					for(int j=0; j<5; j++)
					{
						sprintf(field_name, "port%d%d", i+1, j+1);
						if(epn_vlan->vlan_bitmap[i] & (1<<j))
							addUserVariable(field_name,"checked");
						else
							addUserVariable(field_name,"");
					}
				}

				m_desthtml = "epnvlan_table.html";
			}
			else 			
				m_desthtml = "cardepn.html";
			
			break;
		case CARD_TYPE_MS4V1H:
		case CARD_TYPE_M4H:
		case CARD_TYPE_M4S:
			if ( flag == "switch" )
			{
				if(card_info->card_type == CARD_TYPE_MS4V1H)
					m_desthtml = "ms4_switch.html";
				else
					m_desthtml = "m4s_h_switch.html";
			}
			else if ( flag == "debug" )
				m_desthtml = "ms4_debug.html";
			else
				m_desthtml = "cardms4.html";
			break;
		case CARD_TYPE_FE8:
			
			if ( flag == "fe8port" )
			{
					msg_header->msg_code = C_FE8_PORT_GET;
					msg_header->length = sizeof(FE_PORT_STATE);
					fe8portstatesend = (FE_PORT_STATE*)(send_buff + sizeof(MSG_HEAD_CMD));
					
					
					Index = (unsigned char)std::atoi( portno.c_str() );
					fe8portstatesend->portIndex = Index;
					
					if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
			
					msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
					if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);

					if(msgrsp_header->length != sizeof(FE_PORT_STATE))
						return(ERROR_COMM_PARSE);
					fe8portstate = (FE_PORT_STATE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));

					sprintf (buff, "%d", Index);
					addUserVariable("portno", buff);
					addAdminStateVariable("fe_admin_state", fe8portstate->admin_state);
					addUserVariable("fe_port_state", getLink(fe8portstate->port_state));
					
					sprintf(buff, "%d", fe8portstate->bw_alloc_send*32 );
					addUserVariable("fe_bw_alloc_send" , buff);
					
					sprintf(buff, "%d", fe8portstate->bw_alloc_recv*32 );
					addUserVariable("fe_bw_alloc_recv" , buff );
					
					addUserVariable("fe_port_mode" , getModeFE8(fe8portstate->portMode));
					addUserVariable("fe_working_mode", getWorkingModefe8(fe8portstate->portWorkingMode ) );
					addUserVariable("fe_bounded_mac", cgicc::getMACaddr(fe8portstate->bounded_mac));
					
					m_desthtml = "port_table.html";

				}
			else if( flag == "base" )
			{
				msg_header->msg_code = C_CARD_GET;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
				
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(CARD_INFO))
					return(ERROR_COMM_PARSE);

				card_info = (CARD_INFO*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
				
				addUserVariable("card_type", getCardName(card_info->card_type));
				char buff[256];
				sprintf(buff,"%d", msgrsp_header->card_no);
				addUserVariable("card_no", buff);
				memcpy (buff, card_info->card_name , MAX_DISPLAYNAME_LEN);
				buff[MAX_DISPLAYNAME_LEN] = '\0';
				addUserVariable("card_name", buff);
				
				addAdminStateVariable("admin_state", card_info->admin_state);
				addUserVariable("version_hw", makeVersion(card_info->version_HW[0], card_info->version_HW[1]));
				addUserVariable("version_fw", makeVersion(card_info->version_FW[0], card_info->version_FW[1]));
				addUserVariable("version_sw", makeVersion(card_info->version_SW[0], card_info->version_SW[1]));
				card_info->version_OS[MAX_DISPLAYNAME_LEN -1] = '\0';
				addUserVariable("version_os", card_info->version_OS);

				m_desthtml = "fe8base_table.html";
			}
			else if( flag == "broadcastStorm" )
			{
				msg_header->msg_code = C_FE8_BROADCASTSTORM_GET;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
				
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(BROADCASTSTORM_CTRL_FE))
					return(ERROR_COMM_PARSE);

				broadcastStormInfo = (BROADCASTSTORM_CTRL_FE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
				addUserVariable("storm_ctrl", multilang::getstormstate(broadcastStormInfo->broadcaststorm_ctrl));
				sprintf(buff, "%d",  broadcastStormInfo->storm_threshold);
				addUserVariable("storm_threshold",buff);
				m_desthtml = "fe8broadcastStorm_table.html";
			}
			else {
					
					m_desthtml = "cardfe8.html";
			}
			
			break;
		case CARD_TYPE_FSA600:
			if ( flag == "remote" )/*for onu100*/
			{
				msg_header->msg_code = C_FSA600_GETRMT;
				msg_header->node_id = (unsigned char ) std::atoi (onuno.c_str() );
				msgrsp_header->length = 0;	

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
						
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(FSA600_GETRMT))
						return(ERROR_COMM_PARSE);
					
				remotestate = ( FSA600_GETRMT *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );				
				addUserVariable("transmit_mode" , getTransimitmode (remotestate->TransmitMode, 2 ));
		
				addUserVariable( "cur_workmode_onu1" , getWorkingModefsa600rmt( remotestate->CurWorkMode1, 1 , 1));
				addUserVariable( "conf_workmode_onu1" , getWorkingModefsa600rmt( remotestate->CfgWorkMode1, 1 , 0));
				addUserVariable( "tx1_link",  getLink( remotestate->Txlink1) );

				addUserVariable( "cur_workmode_onu2" , getWorkingModefsa600rmt( remotestate->CurWorkMode2, 2 , 1));
				addUserVariable( "conf_workmode_onu2" , getWorkingModefsa600rmt( remotestate->CfgWorkMode2, 2 , 0));
				addUserVariable( "tx2_link",  getLink( remotestate->Txlink2) );

				sprintf (buff, "%d", msg_header->node_id);
				addUserVariable ("node_id", buff);

				memcpy(buff, remotestate->onu_name, MAX_DISPLAYNAME_LEN );
				buff[MAX_DISPLAYNAME_LEN] = '\0';
				addUserVariable ("onu_name", buff );


				m_desthtml = "onu100.html";
				
			}
			else if ( flag == "local" )/*for local modules*/
			{
				msg_header->msg_code = C_FSA600_INFO;
				msg_header->node_id = (unsigned char ) std::atoi (sfpno.c_str() );/*node_id is used as sfpno.*/

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(FSA600_PORT_485))
					return(ERROR_COMM_PARSE);
				fsa600state = (FSA600_PORT_485 *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );
				
				sprintf (buff, "%d", msg_header->node_id );
				addUserVariable ("node_id", buff);
				addUserVariable("fx_link", getLink( fsa600state->Fxlink));
				
				addUserVariable("port_lock", getLockstate(fsa600state->PortState ));
				addUserVariable("sfp_state", getSFPstate(fsa600state->SfpState));
				
				addUserVariable("transmit_mode" , getTransimitmode( fsa600state->TransmitMode, 1));
				

				if ( 1 == fsa600state->UpStreamFlag )
				{
					sprintf (buff, "%d", fsa600state->UpStream*512);
					addUserVariable( "up_stream", buff  );
				}else{
					
					sprintf (buff, "%d", fsa600state->UpStream*32);
					addUserVariable( "up_stream", buff  );
				}

				if ( 1 == fsa600state->DownStreamFlag )
				{
					sprintf (buff, "%d", fsa600state->DownStream*512);
					addUserVariable( "down_stream", buff );
				}else{
					sprintf (buff, "%d", fsa600state->DownStream*32);
					addUserVariable( "down_stream", buff );
				}
				

				m_desthtml = "fsa600_table.html";


			}
			else {

				m_desthtml = "cardfsa600.html";
			}
			
			break;

		case CARD_TYPE_ENCODE:
			if ( flag == "channel")
			{
				//const char* channel_no = getReqValue("ch_no").c_str();
				//addUserVariable("ch_no",channel_no);
				m_desthtml = "encode_channel.html";
			}
			else if ( flag == "protocol")
			{
				msg_header->msg_code = C_ENCODE_PROTOCOL_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("ch_no").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_PROTOCOL_STATE))
					return(ERROR_COMM_PARSE);
				protocolstate = (ENCODE_PROTOCOL_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );

				addUserVariable("protocol", getProtocol(protocolstate->protocol));
				sprintf (buff, "%s", protocolstate->ip);
				addUserVariable("ip",buff);
				sprintf (buff, "%s", protocolstate->port);
				addUserVariable("port",buff);

				m_desthtml = "encode_protocol.html";
			}
			else if ( flag == "osd")
			{
				msg_header->msg_code = C_ENCODE_OSD_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("ch_no").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_OSD_STATE))
					return(ERROR_COMM_PARSE);
				osdstate = (ENCODE_OSD_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );
			
				addUserVariable("channel_no", getChannelNO( msgrsp_header->node_id));
				
				//addUserVariable("location", getOSDLocation(osdstate->location));
				addUserVariable("location", getOSDLocation((unsigned char)(atoi((const char *)(osdstate->location)))));

				addUserVariable("show_time" , showTime( osdstate->show_time));
				
				osdstate->channel_name[MAX_DISPLAYNAME_LEN -1] = '\0';
				sprintf (buff, "%s", osdstate->channel_name);
				addUserVariable("channel_name", buff);
				
				//addUserVariable("osd_color",getOSDColor( osdstate->color));
				addUserVariable("osd_color",getOSDColor((unsigned char)(atoi((const char *)(osdstate->color)))));

				//m_desthtml = "encode_osd.html";
				m_desthtml = "encode_video_osd.html";
			}
			else if ( flag == "picture")
			{
				msg_header->msg_code = C_ENCODE_PICTURE_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("ch_no").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_PICTURE_STATE))
					return(ERROR_COMM_PARSE);
				picturestate = (ENCODE_PICTURE_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );

				sprintf (buff, "%s", picturestate->bright);
				addUserVariable("bright", buff);
				sprintf (buff, "%s", picturestate->contrast);
				addUserVariable("contrast",buff);
				sprintf (buff, "%s", picturestate->chroma);
				addUserVariable("chroma",buff);
				sprintf (buff, "%s", picturestate->saturation);
				addUserVariable("saturation",buff);

				m_desthtml = "encode_video_image.html";
			}
			else if ( flag == "bitType")
			{
				msg_header->msg_code = C_ENCODE_MODE_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("ch_no").c_str());;

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_MODE_STATE))
					return(ERROR_COMM_PARSE);
				patten = (ENCODE_MODE_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );

				addUserVariable("stream_type" , getStreamtype( patten->streamtype ));

				m_desthtml = "encode_ts.html";
			}
			/*else  if ( flag == "patten")
			{

				m_desthtml = "encode_ajaxmode.html";
			}

			else if ( flag == "test")
			{
				m_desthtml = "demo.htm";
				
			}
			
			else  if ( flag == "ajaxpatten")
			{
				msg_header->msg_code = C_ENCODE_MODE_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("ch_no").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_MODE_STATE))
					return(ERROR_COMM_PARSE);
				patten = (ENCODE_MODE_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );
				
				/*addUserVariable("channel_no", getChannel( patten->channelno) );
				
				addUserVariable("compress_type", getCompresstype( patten->compresstype ));*/

				/*addUserVariable("stream_type" , getStreamtype( patten->streamtype ));


				m_desthtml = "encode_mode.html";
			}*/
			
			else if ( flag == "reboot" )
			{
				m_desthtml = "encode_restart.html";
			}
			else if (flag == "network")
			{
				msg_header->msg_code = C_ENCODE_NET_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("ch_no").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_NET_STATE))
					return(ERROR_COMM_PARSE);
				netstate = (ENCODE_NET_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );
				sprintf (buff, "%s", netstate->ipaddr);
				addUserVariable("encode_ip", buff);
				sprintf (buff, "%s", netstate->mask);
				addUserVariable("encode_mask", buff);
				sprintf (buff, "%s", netstate->gateway);
				addUserVariable("encode_gateway", buff);

				m_desthtml = "encode_ip.html";
				

			}
			else if ( flag == "systime" )
			{
				msg_header->msg_code = C_ENCODE_SYSTIME_GET;
				msg_header->node_id = 0;

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_SYSTIME))
					return(ERROR_COMM_PARSE);
				sys_time = (ENCODE_SYSTIME *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );

				sprintf (buff, "%s", sys_time->time);
				addUserVariable("encode_time", buff);
				

				m_desthtml = "encode_time.html";
			}
			else if ( flag == "ntp" )
			{
				msg_header->msg_code = C_ENCODE_NTP_GET;
				msg_header->node_id = 0;

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_NTP))
					return(ERROR_COMM_PARSE);
				sys_ntp = (ENCODE_NTP *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );

				sprintf (buff, "%s", sys_ntp->server_addr);
				addUserVariable("ntp_ip", buff);
				sprintf ( buff, "%s", sys_ntp->interval );
				addUserVariable("interval", buff);

				m_desthtml = "encode_ntp.html";
				
			}
			/*else if ( flag == "ctrl" )
			{
				addUserVariable("channel_no", getChannelNO( 0 ));
				m_desthtml = "encode_control.html";
			}*/
			else if ( flag == "encode_video_param" )
			{
				msg_header->msg_code = C_ENCODE_VIDEO_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("ch_no").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
		
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_VIDEO_STATE))
					return(ERROR_COMM_PARSE);

				/*remain to process recv message*/
				
				videostate = (ENCODE_VIDEO_STATE *)(recv_buff + sizeof (MSG_HEAD_CMD_RSP));

				sprintf( buff, "%d", videostate->channelno);
				addUserVariable( "channel_no" , buff);

				sprintf(buff, "%s", videostate->vidBitType);
				addUserVariable("vid_bit_type", get_vid_bit_type((unsigned char)(atoi((const char *)(buff)))));
				addUserVariable("vbr_cbr", show_cbr((unsigned char)(atoi((const char *)(buff)))));
				addUserVariable("vid_bit_vbr", get_vbr((unsigned char)(atoi((const char *)(buff)))));
				
				sprintf(buff, "%s", videostate->vidBitRate);
				addUserVariable( "vid_bit_rate" , buff);
				
				sprintf(buff, "%s", videostate->resolving);
				addUserVariable("resolving", get_resolving(buff));
				//addUserVariable("resolving", buff);
				
				sprintf(buff, "%s", videostate->frameRate);
				addUserVariable( "frm_rate" , buff);

				//addUserVariable("gop",get_gop(videostate->gop));
				addUserVariable("gop",get_gop((unsigned char)(atoi((const char *)(videostate->gop)))));

				m_desthtml = "encode_video_param.html";

			}
			else if( flag == "base" )
			{
				msg_header->msg_code = C_CARD_GET;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
				
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(CARD_INFO))
					return(ERROR_COMM_PARSE);

				card_info = (CARD_INFO*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
				
				addUserVariable("card_type", getCardName(card_info->card_type));
				char buff[256];
				sprintf(buff,"%d", msgrsp_header->card_no);
				addUserVariable("card_no", buff);
				memcpy (buff, card_info->card_name , MAX_DISPLAYNAME_LEN);
				buff[MAX_DISPLAYNAME_LEN] = '\0';
				addUserVariable("card_name", buff);
				
				addAdminStateVariable("admin_state", card_info->admin_state);
				addUserVariable("version_hw", makeVersion(card_info->version_HW[0], card_info->version_HW[1]));
				addUserVariable("version_fw", makeVersion(card_info->version_FW[0], card_info->version_FW[1]));
				addUserVariable("version_sw", makeVersion(card_info->version_SW[0], card_info->version_SW[1]));
				card_info->version_OS[MAX_DISPLAYNAME_LEN -1] = '\0';
				addUserVariable("version_os", card_info->version_OS);

				m_desthtml = "encodebase_table.html";
			}
			else
			{
				m_desthtml = "cardencode.html";
			}
			break;

		case CARD_TYPE_DECODE:
			if ( flag == "channel")
			{
				m_desthtml = "decode_channel.html";
			}
			else if ( flag == "protocol")
			{
				msg_header->msg_code = C_ENCODE_PROTOCOL_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("ch_no").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_PROTOCOL_STATE))
					return(ERROR_COMM_PARSE);
				protocolstate = (ENCODE_PROTOCOL_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );

				addUserVariable("protocol", getProtocol(protocolstate->protocol));
				sprintf (buff, "%s", protocolstate->ip);
				addUserVariable("ip",buff);
				sprintf (buff, "%s", protocolstate->port);
				addUserVariable("port",buff);

				m_desthtml = "encode_protocol.html";
			}
			else if ( flag == "bitType")
			{
				msg_header->msg_code = C_ENCODE_MODE_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("ch_no").c_str());;

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_MODE_STATE))
					return(ERROR_COMM_PARSE);
				patten = (ENCODE_MODE_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );

				addUserVariable("stream_type" , getStreamtype( patten->streamtype ));

				m_desthtml = "encode_ts.html";
			}
			else if ( flag == "reboot" )
			{
				m_desthtml = "encode_restart.html";
			}
			else if (flag == "network")
			{
				msg_header->msg_code = C_ENCODE_NET_GET;
				msg_header->node_id = (unsigned char) std::atoi(getReqValue("ch_no").c_str());

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_NET_STATE))
					return(ERROR_COMM_PARSE);
				netstate = (ENCODE_NET_STATE *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );
				sprintf (buff, "%s", netstate->ipaddr);
				addUserVariable("encode_ip", buff);
				sprintf (buff, "%s", netstate->mask);
				addUserVariable("encode_mask", buff);
				sprintf (buff, "%s", netstate->gateway);
				addUserVariable("encode_gateway", buff);

				m_desthtml = "encode_ip.html";
				

			}
			else if ( flag == "systime" )
			{
				msg_header->msg_code = C_ENCODE_SYSTIME_GET;
				msg_header->node_id = 0;

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_SYSTIME))
					return(ERROR_COMM_PARSE);
				sys_time = (ENCODE_SYSTIME *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );

				sprintf (buff, "%s", sys_time->time);
				addUserVariable("encode_time", buff);
				

				m_desthtml = "encode_time.html";
			}
			else if ( flag == "ntp" )
			{
				msg_header->msg_code = C_ENCODE_NTP_GET;
				msg_header->node_id = 0;

				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
	
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(ENCODE_NTP))
					return(ERROR_COMM_PARSE);
				sys_ntp = (ENCODE_NTP *)( recv_buff + sizeof(MSG_HEAD_CMD_RSP) );

				sprintf (buff, "%s", sys_ntp->server_addr);
				addUserVariable("ntp_ip", buff);
				sprintf ( buff, "%s", sys_ntp->interval );
				addUserVariable("interval", buff);

				m_desthtml = "encode_ntp.html";
				
			}
			else if( flag == "base" )
			{
				msg_header->msg_code = C_CARD_GET;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
				
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);

				if(msgrsp_header->length != sizeof(CARD_INFO))
					return(ERROR_COMM_PARSE);

				card_info = (CARD_INFO*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
				
				addUserVariable("card_type", getCardName(card_info->card_type));
				char buff[256];
				sprintf(buff,"%d", msgrsp_header->card_no);
				addUserVariable("card_no", buff);
				memcpy (buff, card_info->card_name , MAX_DISPLAYNAME_LEN);
				buff[MAX_DISPLAYNAME_LEN] = '\0';
				addUserVariable("card_name", buff);
				
				addAdminStateVariable("admin_state", card_info->admin_state);
				addUserVariable("version_hw", makeVersion(card_info->version_HW[0], card_info->version_HW[1]));
				addUserVariable("version_fw", makeVersion(card_info->version_FW[0], card_info->version_FW[1]));
				addUserVariable("version_sw", makeVersion(card_info->version_SW[0], card_info->version_SW[1]));
				card_info->version_OS[MAX_DISPLAYNAME_LEN -1] = '\0';
				addUserVariable("version_os", card_info->version_OS);

				m_desthtml = "encodebase_table.html";
			}
			else
			{
				m_desthtml = "cardencode.html";
			
			}
			break;
			
/*
    查询状态C_NTT_GETSTATE（0xA1）
    原来仅有2个字节的返回信息。这里把它进行扩展。共11个字节：
    【1】	FD1State [6:0] =【a_dled, a_rpf, a_link, a_sfp_present】。
            Rpf = 远端电源故障
            Link = 光口状态
            Sfp_present = SFP光模块在位
            Dled为4个比特，从高到低对应：tx3，rx3，tx1，rx1
            光口：1标识在线，0为下线；rpf：1为故障，0为正常；sfp_present：1为正常，0为不正常

    【2】	FD2State [6:0] = 【b_dled, b_rpf, b_link, b_sfp_present】
            Dled为4个比特，从高到对应：tx4，rx4，tx2，rx2
    【3】Loc_EthState，每2个比特代表一个状态：
            00：down，01：10M，10：100M，11：1G
            Loc_EthState总体是一个字节，即8个比特，每2个比特代表一个端口的状态：
            bit[1:0]代表本地以太网端口1，bit[3:2]代表端口2，
            bit[5:4]代表端口3，bit[7:6]代表端口4.

    【4】Rmt1_card_type, 【1C表示前端为4E3D大缓存设备。】1C代表4E+2D带16Mb缓存的设备，
            我们自己的名称为：HVFE402M。 HVFE 代表高清视频快速以太网的意思，M代表内存。
            还有一个类型值1D（十进制29），表示1E+2D的设备，名称为HVFE102T。T代表发送端
    【5】Rmt1_ethstate
    【6】Rmt1_otherState [ RMT_VER ]. // data status using local only.
    【7】Rmt1_temp
    【8】Rmt2_card_type,
    【9】Rmt2_ethstate
    【10】Rmt2_otherState
    【11】Rmt2_temp (update every second)
        RMT_VER HIGH 6 bits: [7:6] SW, [5:4] FW, [3:2] HW
        Rmt1_temp为有符号单字节，取值范围-128°C~+127°C。
        温度过高告警在BYTE3 local port up的比特[4]和[5]，分别表示远端1和远端2发生温度过高告警。
        （过高指温度>=65°C ，固定门限）。
        温度和PERFORMANCE数据待下一版本。
*/
#ifdef __MTT__  // 201611
        case CARD_TYPE_MTT:
			if ( flag == "getstate" )
			{
			    char tmp[24];
				msg_header->msg_code = C_MTT_GETSTATE;
				msg_header->card_no  = (unsigned char)std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff)!=0)
					return  ERROR_COMM_LOCAL;
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0)
					return  (msgrsp_header->rsp_code);
				if(msgrsp_header->length!=sizeof(MTT_STATE))
					return  ERROR_COMM_PARSE;

				mttstate = (MTT_STATE *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));
				
				// 解析接收端状态，然后输出
				for (int i = 1; i <= 2; i++)
				{
                    int state = mttstate->FD1State;
                    char tmp_img[24];
                    if (i == 2) 
                        state = mttstate->FD2State;
                    sprintf(tmp_img, "spf%d_img", i);
                    
                    sprintf(tmp, "spf%d_state", i);
                    if ((state & 0x01) == 1)
                    {
                        addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                        addUserVariable(tmp,  multilang::getLabel("ON"));
                    }
                    else
                    {
                        addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                        addUserVariable(tmp,  multilang::getLabel("OFF"));
                    }

                    sprintf(tmp_img, "fiber%d_img", i);
                    sprintf(tmp, "fiber%d_state", i);
                    if ((state & 0x02) == 2)
                    {
                        addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                        addUserVariable(tmp,  multilang::getLabel("ON"));
                    }
                    else
                    {
                        addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                        addUserVariable(tmp,  multilang::getLabel("OFF"));
                    }
                    
                    sprintf(tmp_img, "rpf%d_img", i);
                    sprintf(tmp, "rpf%d_state", i);
                    if ((state & 0x04) == 4)
                    {
                        addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                        addUserVariable(tmp,  multilang::getLabel("OFF"));
                    }
                    else
                    {
                        addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                        addUserVariable(tmp,  multilang::getLabel("ON"));
                    }
                }
                    
                    
                switch(mttstate->FD1State & 0x60 )
                {
                    case  0x60:  // data 3
                        addUserVariable("RT3_state",multilang::getLabel("R/T"));
                    break;
                    case 0x40:
                        addUserVariable("RT3_state",multilang::getLabel("T"));
                    break;
                    case 0x20:
                        addUserVariable("RT3_state",multilang::getLabel("R"));
                    break;
                    default:
                        addUserVariable("RT3_state",multilang::getLabel("NONE"));
                    break;
                }
                    
                switch(mttstate->FD1State & 0x18)
                {
                    case  0x18:  // data 3
                        addUserVariable("RT1_state",multilang::getLabel("R/T"));
                    break;
                    case 0x10:
                        addUserVariable("RT1_state",multilang::getLabel("T"));
                    break;
                    case 0x08:
                        addUserVariable("RT1_state",multilang::getLabel("R"));
                    break;
                    default:
                        addUserVariable("RT1_state",multilang::getLabel("NONE"));
                    break;
                }
                
                if (mttstate->FD2State & 0x01 == 1) 
                    addUserVariable("spf2_state",multilang::getLabel("spf_on"));
                else
                    addUserVariable("spf2_state",multilang::getLabel("spf_off"));
				
                if (mttstate->FD2State & 0x02 == 2) 
                    addUserVariable("fiber2_state",multilang::getLabel("fiber_on"));
                else
                    addUserVariable("fiber2_state",multilang::getLabel("fiber_off"));
                    
                if (mttstate->FD2State & 0x04 == 4)  // RPF
                    addUserVariable("rpf2_state",multilang::getLabel("power_on"));
                else
                    addUserVariable("rpf2_state",multilang::getLabel("power_off"));
                    
                switch(mttstate->FD2State & 0x60 )
                {
                    case  0x60:  // data 3
                        addUserVariable("RT4_state",multilang::getLabel("data_RT"));
                    break;
                    case 0x40:
                        addUserVariable("RT4_state",multilang::getLabel("data_T"));
                    break;
                    case 0x20:
                        addUserVariable("RT4_state",multilang::getLabel("data_R"));
                    break;
                    default:
                        addUserVariable("RT4_state",multilang::getLabel("data_NONE"));
                    break;
                }
                    
                switch(mttstate->FD2State & 0x18)
                {
                    case  0x18:  // data 3
                        addUserVariable("RT2_state",multilang::getLabel("data_RT"));
                    break;
                    case 0x10:
                        addUserVariable("RT2_state",multilang::getLabel("data_T"));
                    break;
                    case 0x08:
                        addUserVariable("RT2_state",multilang::getLabel("data_R"));
                    break;
                    default:
                        addUserVariable("RT2_state",multilang::getLabel("data_NONE"));
                    break;
                }


                // 4个本地以太网口    00：down，01：10M，10：100M，11：1G
                for (int i = 0; i < 4; i++)
                {
                    int state = (mttstate->Loc_EthState >> (i * 2)) & 0x03;
                    sprintf(tmp, "Loc_Eth%dState", i+1);
                    char tmp_img[24];
                    sprintf(tmp_img, "eth%d_img%d", j, i+1);
                    switch(state)
                    {
                        case 0:
                        default:
                            addUserVariable(tmp_img, "<img  title='DOWN'  src='/images/red.gif' >");
                            addUserVariable(tmp,  multilang::getLabel("Down"));
                        break;
                        case 1:
                            addUserVariable(tmp_img, "<img  title='10M' src='/images/green.gif' >");
                            addUserVariable(tmp,  multilang::getLabel("10M"));
                        break;
                        case 2:
                            addUserVariable(tmp_img, "<img  title='100M' src='/images/green.gif' >");
                            addUserVariable(tmp,  multilang::getLabel("100M"));
                        break;
                        case 3:
                            addUserVariable(tmp_img, "<img  title='1G' src='/images/green.gif' >");
                            addUserVariable(tmp,  multilang::getLabel("1G"));
                        break;
                    }
                }

                //mtu1
                if (mttstate->Rmt1_card_type == 0x1C)
                    addUserVariable("mtu1_type", "HVFE402M");
                else(mttstate->Rmt1_card_type == 0x1D)
                    addUserVariable("mtu1_type", "HVFE102T");
                else
                    addUserVariable("mtu1_type", "Unknown");
                
                if (0 == mttstate->Rmt1_temp)
                    strcpy(tmp, multilang::getLabel("Unknown"));
                else
                    sprintf(tmp, "%d", mttstate->Rmt1_temp);
                addUserVariable("mtu1_temp", tmp);

                
                if (mttstate->Rmt2_card_type == 0x1C)
                    addUserVariable("mtu2_type", "HVFE402M");
                else(mttstate->Rmt1_card_type == 0x1D)
                    addUserVariable("mtu2_type", "HVFE102T");
                else
                    addUserVariable("mtu2_type", "Unknown");

                if (0 == mttstate->Rmt2_temp)
                    strcpy(tmp, multilang::getLabel("Unknown"));
                else
                    sprintf(tmp, "%d", mttstate->Rmt2_temp);
                addUserVariable("mtu2_temp", tmp);

                
                for (int j = 1; j <= 2; j++)
                {
                    int state = mttstate->Rmt1_ethstate;
                    if (j == 2)
                        state = mttstate->Rmt2_ethstate;
                        
                    for (int i = 0; i < 4; i++)
                    {
                        //int state = (mttstate->Rmt2_ethstate >> (i * 2)) & 0x03;
                        state = (state >> (i * 2)) & 0x03;
                        
                        sprintf(tmp, "rmt%d_Eth%dState", j, i+1);
                        char tmp_img[24];
                        sprintf(tmp_img, "eth%d_img%d", j, i+1);
                        switch(state)
                        {
                            case 0:
                            default:
                                addUserVariable(tmp_img, "<img  title='DOWN'  src='/images/red.gif' >");
                                addUserVariable(tmp,  multilang::getLabel("Down"));
                            break;
                            case 1:
                                addUserVariable(tmp_img, "<img  title='10M' src='/images/green.gif' >");
                                addUserVariable(tmp,  multilang::getLabel("10M"));
                            break;
                            case 2:
                                addUserVariable(tmp_img, "<img  title='100M' src='/images/green.gif' >");
                                addUserVariable(tmp,  multilang::getLabel("100M"));
                            break;
                            case 3:
                                addUserVariable(tmp_img, "<img  title='1G' src='/images/green.gif' >");
                                addUserVariable(tmp,  multilang::getLabel("1G"));
                            break;
                        }
                    }
                }

                // 为mtt添加新的html文件
				m_desthtml = "mttfiber_table.html";
                    
			}
			else if (flag == "getmtuport")
			{
				msg_header->msg_code = C_MTT_GETSTATE;
				msg_header->card_no  = (unsigned char)std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff)!=0)
					return  ERROR_COMM_LOCAL;
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0)
					return  (msgrsp_header->rsp_code);
				if(msgrsp_header->length!=sizeof(MTT_STATE))
					return  ERROR_COMM_PARSE;

				mttstate = (MTT_STATE *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));
				
				// 解析前端的状态，然后输出
				
				addUserVariable("mtuno", ntuno);
				int state, rpf_state;
				char tmp[24];
				int card_type ;
				if (ntuno == "1")
				{
				    card_type = mttstate->Rmt1_card_type;
                    sprintf(tmp, "%d", mttstate->Rmt1_temp);
                    addUserVariable("mtu_temp", tmp);
                    
                    state = mttstate->Rmt1_ethstate;
                    rpf_state = mttstate->FD1State;
				}
				else if (ntuno == "2")
				{
				    card_type = mttstate->Rmt1_card_type;
                    sprintf(tmp, "%d", mttstate->Rmt2_temp);
                    addUserVariable("mtu_temp", tmp);
                    state = mttstate->Rmt2_ethstate;
                    rpf_state = mttstate->FD2State;
                }

                if (card_type == 0x1C)
                    strcpy(tmp,  "HVFE402M");
                else if (card_type == 0x1D)
                    strcpy(tmp,  "HVFE102T");
                else
                    strcpy(tmp,  "Unknown");

                addUserVariable("mtu_type", tmp);
                
                char tmp_img[24];
                strcpy(tmp_img, "rpf_img");
                strcpy(tmp, "rpf_state");
                if ((rpf_state & 0x04) == 4)
                {
                    addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("OFF"));
                }
                else
                {
                    addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("ON"));
                }
				

                for (int i = 0; i < 4; i++)
                {
                    state = (state >> (i * 2)) & 0x03;
                    sprintf(tmp, "rmt_Eth%dState", i + 1);
                    char tmp_img[24];
                    sprintf(tmp_img, "eth_img%d", i+1);
                    switch(state)
                    {
                        case 0:
                            addUserVariable(tmp_img, "<img  title='DOWN'  src='/images/red.gif' >");
                            addUserVariable(tmp,  multilang::getLabel("Down"));
                        break;
                        case 1:
                            addUserVariable(tmp_img, "<img  title='10M' src='/images/green.gif' >");
                            addUserVariable(tmp,  multilang::getLabel("10M"));
                        break;
                        case 2:
                            addUserVariable(tmp_img, "<img  title='100M' src='/images/green.gif' >");
                            addUserVariable(tmp,  multilang::getLabel("100M"));
                        break;
                        case 3:
                            addUserVariable(tmp_img, "<img  title='1G' src='/images/green.gif' >");
                            addUserVariable(tmp,  multilang::getLabel("1G"));
                        break;
                    }
                }
                


                // 为mtt前端添加新的html文件
				m_desthtml = "mtu_port.html";
			}
			else
			{
				m_desthtml = "cardmtt.html";
			}
			// 目前没有config
        break;
#else
		case CARD_TYPE_NTT:
        case CARD_TYPE_NTT_S: // 20150115 add 
			if ( flag == "getstate" )
			{
				msg_header->msg_code = C_NTT_GETSTATE;
				msg_header->card_no  = (unsigned char)std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff,recv_buff)!=0)
					return  ERROR_COMM_LOCAL;
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0)
					return  (msgrsp_header->rsp_code);
				if(msgrsp_header->length!=sizeof(NTT_STATE))
					return  ERROR_COMM_PARSE;

				nttstate = (NTT_STATE *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));
				char flink;
				flink=nttstate->link;
				if(flink&0x01)
					addUserVariable("fiber1_state",multilang::getLabel("fiber_on"));
				else
					addUserVariable("fiber1_state",multilang::getLabel("fiber_off"));
				if(flink&0x02)
					addUserVariable("fiber2_state",multilang::getLabel("fiber_on"));
				else
					addUserVariable("fiber2_state",multilang::getLabel("fiber_off"));

				char buff[256],buff_mode[256];	

				msg_header->msg_code = C_NTT_INFO;
				msg_header->length = sizeof(FE_PORT_STATE) ;
				fe8portstatesend = (FE_PORT_STATE*)(send_buff + sizeof(MSG_HEAD_CMD));
				for(int i=1;i<=8;i++){
					fe8portstatesend->portIndex = i;
					if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
			
					msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
					if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);

					if(msgrsp_header->length != sizeof(FE_PORT_STATE))
						return(ERROR_COMM_PARSE);
					nttinfo = (FE_PORT_STATE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
								
					sprintf(buff,"ntt_port%d",i);
					sprintf(buff_mode,"ntt_port_state%d",i);
					switch(nttinfo->port_state)
					{
						
						case 1:
							addUserVariable(buff, "<img  title='ON'  src='/images/green.gif' >");
							addUserVariable(buff_mode, "ON");
							break;
						case 0:
						case 2:
							addUserVariable(buff,"<img  title='DOWN'  src='/images/red.gif'>");
							addUserVariable(buff_mode,"DOWN");
							break;
						
					}
				}
				
				m_desthtml = "nttfiber_table.html";
			
			}
			else if( flag == "config" )
			{
				msg_header->msg_code = C_NTT_INFO;
				msg_header->length = sizeof(FE_PORT_STATE);
				fe8portstatesend = (FE_PORT_STATE*)(send_buff + sizeof(MSG_HEAD_CMD));
				
				for(int i=1;i<=8;i++)
				{
				
					fe8portstatesend->portIndex = i;
					
					if(comm::send_cmd(send_buff, recv_buff) != 0) 
						return ERROR_COMM_LOCAL;
			
					msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
					if(msgrsp_header->rsp_code != 0) 
						return(msgrsp_header->rsp_code);

					if(msgrsp_header->length != sizeof(FE_PORT_STATE))
						return(ERROR_COMM_PARSE);

					char buff[256],nbuff[256];
					nttinfo = (FE_PORT_STATE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
									
					sprintf(buff,"ntt_admin_state%d",i);					
					addAdminStateVariable(buff, nttinfo->admin_state);

					sprintf(nbuff,"ntt_bw_alloc_send%d",i);
					sprintf(buff, "%d", nttinfo->bw_alloc_send*32 );
					addUserVariable(nbuff , buff);

					sprintf(nbuff,"ntt_bw_alloc_recv%d",i);
					sprintf(buff, "%d",nttinfo->bw_alloc_recv*32 );
					addUserVariable(nbuff, buff );
							
				}
				
				msg_header->msg_code = C_FE8_BROADCASTSTORM_GET;
				msg_header->card_no = (unsigned char) std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff) != 0) 
					return ERROR_COMM_LOCAL;
						
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0) 
					return(msgrsp_header->rsp_code);
				if(msgrsp_header->length != sizeof(BROADCASTSTORM_CTRL_FE))
					return(ERROR_COMM_PARSE);
			
				broadcastStormInfo = (BROADCASTSTORM_CTRL_FE*)(recv_buff + sizeof(MSG_HEAD_CMD_RSP));
				addUserVariable("storm_ctrl", multilang::getstormstate(broadcastStormInfo->broadcaststorm_ctrl));
				sprintf(buff, "%d",  broadcastStormInfo->storm_threshold);
				addUserVariable("storm_threshold",buff);
				m_desthtml = "ntt_config.html";
			}
			else if( flag == "getntuport"){
				msg_header->msg_code = C_NTT_GETSTATE;
				msg_header->card_no  = (unsigned char)std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff,recv_buff)!=0)
					return  ERROR_COMM_LOCAL;
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0)
					return  (msgrsp_header->rsp_code);
				if(msgrsp_header->length!=sizeof(NTT_STATE))
					return  ERROR_COMM_PARSE;

				nttstate = (NTT_STATE *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));

				int mask=0x01;
				char ntustate;
				char buff[256],buff_mode[256];
				ntustate=nttstate->ntu_state;
				addUserVariable("ntuno",ntuno);
				if(ntuno == "2")
					mask=mask<<4;
				for(int i=1;i<=4;i++)
				{
    				sprintf(buff,"ntu_port%d",i);
    				sprintf(buff_mode,"ntu_port_state%d",i);
    				if(ntustate&mask){
    					addUserVariable(buff, "<img  title='ON'  src='/images/green.gif' >");
    					addUserVariable(buff_mode,"ON");
    				}
    				else{
    					addUserVariable(buff, "<img  title='DOWN'  src='/images/red.gif' >");
    					addUserVariable(buff_mode,"DOWN");
    			}
				mask=mask<<1;
				}
				
				m_desthtml = "ntu_port.html";

			}
			else {
								
				m_desthtml = "cardntt.html";
			}
			break;
#endif			

#if 1 // 201708
        case CARD_TYPE_MTT_411:
			if ( flag == "getstate" )
			{
			    char tmp[24];
				msg_header->msg_code = C_MTT_GETSTATE;
				msg_header->card_no  = (unsigned char)std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff)!=0)
					return  ERROR_COMM_LOCAL;
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0)
					return  (msgrsp_header->rsp_code);
				if(msgrsp_header->length!=sizeof(MTT_411_STATE))
					return  ERROR_COMM_PARSE;

				mtt411state = (MTT_411_STATE *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));
				
				// 解析接收端状态，然后输出
                int state = mtt411state->FD1State;
                char tmp_img[24];
                if (i == 2) 
                    state = mtt411state->FD2State;
                sprintf(tmp_img, "spf%d_img", i);
                
                sprintf(tmp, "spf%d_state", i);
                if ((state & 0x01) == 1)
                {
                    addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("ON"));
                }
                else
                {
                    addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("OFF"));
                }

                sprintf(tmp_img, "fiber%d_img", i);
                sprintf(tmp, "fiber%d_state", i);
                if ((state & 0x02) == 2)
                {
                    addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("ON"));
                }
                else
                {
                    addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("OFF"));
                }
                
                sprintf(tmp_img, "rpf%d_img", i);
                sprintf(tmp, "rpf%d_state", i);
                if ((state & 0x04) == 4)
                {
                    addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("OFF"));
                }
                else
                {
                    addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("ON"));
                }

                m_desthtml = "mtt411fiber_table.html";                
            }
			else if (flag == "getmtuport")
			{
				msg_header->msg_code = C_MTT_GETSTATE;
				msg_header->card_no  = (unsigned char)std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff)!=0)
					return  ERROR_COMM_LOCAL;
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0)
					return  (msgrsp_header->rsp_code);
				if(msgrsp_header->length!=sizeof(MTT_411_STATE))
					return  ERROR_COMM_PARSE;

				mtt411state = (MTT_411_STATE *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));
				
				// 解析前端的状态，然后输出
				
				addUserVariable("mtuno", ntuno);
				int state, rpf_state;
				char tmp[24];
				int card_type ;
				if (ntuno == "1")
				{
				    card_type = mttstate->Rmt1_card_type;
                    sprintf(tmp, "%d", mttstate->Rmt1_temp);
                    addUserVariable("mtu_temp", tmp);
                    
                    state = mttstate->Rmt1_ethstate;
                    rpf_state = mttstate->FD1State;
				}
				else if (ntuno == "2")
				{
				    card_type = mttstate->Rmt1_card_type;
                    sprintf(tmp, "%d", mttstate->Rmt2_temp);
                    addUserVariable("mtu_temp", tmp);
                    state = mttstate->Rmt2_ethstate;
                    rpf_state = mttstate->FD2State;
                }

                if (card_type == 0x1C)
                    strcpy(tmp,  "HVFE402M");
                else if (card_type == 0x1D)
                    strcpy(tmp,  "HVFE102T");
                else
                    strcpy(tmp,  "Unknown");

                addUserVariable("mtu_type", tmp);
                
                char tmp_img[24];
                strcpy(tmp_img, "rpf_img");
                strcpy(tmp, "rpf_state");
                if ((rpf_state & 0x04) == 4)
                {
                    addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("OFF"));
                }
                else
                {
                    addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("ON"));
                }

                
                // 为mtt前端添加新的html文件
                m_desthtml = "mtu411_port.html";
            }
            else
            {
                m_desthtml = "cardmtt.html";
            }
				
                            
        break;
        
        case CARD_TYPE_MTT_441:
			if ( flag == "getstate" )
			{
			    char tmp[24];
				msg_header->msg_code = C_MTT_GETSTATE;
				msg_header->card_no  = (unsigned char)std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff)!=0)
					return  ERROR_COMM_LOCAL;
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0)
					return  (msgrsp_header->rsp_code);
				if(msgrsp_header->length!=sizeof(MTT_441_STATE))
					return  ERROR_COMM_PARSE;

				mtt441state = (MTT_441_STATE *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));
				
				// 解析接收端状态，然后输出
                int state = mtt441state->;
                char tmp_img[24];
                if (i == 2) 
                    state = mtt441state->FD2State;
                sprintf(tmp_img, "spf%d_img", i);
                
                sprintf(tmp, "spf%d_state", i);
                if ((state & 0x01) == 1)
                {
                    addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("ON"));
                }
                else
                {
                    addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("OFF"));
                }

                sprintf(tmp_img, "fiber%d_img", i);
                sprintf(tmp, "fiber%d_state", i);
                if ((state & 0x02) == 2)
                {
                    addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("ON"));
                }
                else
                {
                    addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("OFF"));
                }
                
                sprintf(tmp_img, "rpf%d_img", i);
                sprintf(tmp, "rpf%d_state", i);
                if ((state & 0x04) == 4)
                {
                    addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("OFF"));
                }
                else
                {
                    addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("ON"));
                }

                m_desthtml = "mtt441fiber_table.html";                
            }
			else if (flag == "getmtuport")
			{
				msg_header->msg_code = C_MTT_GETSTATE;
				msg_header->card_no  = (unsigned char)std::atoi(card_no.c_str());
				if(comm::send_cmd(send_buff, recv_buff)!=0)
					return  ERROR_COMM_LOCAL;
				msgrsp_header = (MSG_HEAD_CMD_RSP *)recv_buff;
				if(msgrsp_header->rsp_code != 0)
					return  (msgrsp_header->rsp_code);
				if(msgrsp_header->length!=sizeof(MTT_441_STATE))
					return  ERROR_COMM_PARSE;

				mtt441state = (MTT_441_STATE *)(recv_buff+sizeof(MSG_HEAD_CMD_RSP));
				
				// 解析前端的状态，然后输出
				
				addUserVariable("mtuno", ntuno);
				int state, rpf_state;
				char tmp[24];
				int card_type = CARD_TYPE_MTT_441;
				if (ntuno == "1")
				{
				    
                    sprintf(tmp, "%.2f", mtt441state->temp_int + (float)mtt441state->temp_fract /16.0);
                    addUserVariable("mtu_temp", tmp);
                    
                    state = mtt441state->Rmt1_ethstate;
                    rpf_state = mtt441state->FD1State;
				}
				else if (ntuno == "2")
				{
				    card_type = mtt441state->Rmt1_card_type;
                    sprintf(tmp, "%d", mttstate->Rmt2_temp);
                    addUserVariable("mtu_temp", tmp);
                    state = mtt441state->Rmt2_ethstate;
                    rpf_state = mtt441state->FD2State;
                }


                addUserVariable("mtu_type", "MTT_GD441");
                
                char tmp_img[24];
                strcpy(tmp_img, "rpf_img");
                strcpy(tmp, "rpf_state");
                if ((rpf_state & 0x04) == 4)
                {
                    addUserVariable(tmp_img, "<img  title='OFF' src='/images/red.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("OFF"));
                }
                else
                {
                    addUserVariable(tmp_img, "<img  title='ON' src='/images/green.gif' >");
                    addUserVariable(tmp,  multilang::getLabel("ON"));
                }

                
                // 为mtt前端添加新的html文件
                m_desthtml = "mtu441_port.html";
            }
            else
            {
                m_desthtml = "cardmtt.html";
            }
		break;
#endif
		default:
			break;

	}

	return 0;

}

void CGetCard::output()
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

	int ret = getCardData();
	
	if(ret == 0) {
		if(outputHTMLFile(m_desthtml, true) != 0)
		{
			outputError("Cann't open card.html\n");
		}	
	} 
	else 
		outputError(multilang::getError(ret));

	free(cs);

}


